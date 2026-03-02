#include "Cgi.hpp"
#include <sys/types.h>
#include <signal.h>
#include <sys/epoll.h>
#include <climits>
#include "Response.hpp"
#include "../include/c_network_exception.h"

// HELPER FUNCTIONS
void	Cgi::close_pipes(int *p1, int *p2)
{
	if (!p1 || !p2)
		throw std::logic_error("attempt to close NULL pipes");

	if (p1[0] != -1) {
		close(p1[0]);
		p1[0] = -1;
		}
	if (p1[1] != -1) {
		close(p1[1]);
		p1[1] = -1;
		}
	if (p2[0] != -1) {
		close(p2[0]);
		p2[0] = -1;
	}
	if (p2[1] != -1) {
		close(p2[1]);
		p2[1] = -1;
	}
}

char**	Cgi::allocate_envp(const std::vector<std::string>& env)
{
	size_t size = env.size();
	char** envp = new char*[size + 1];
	for (size_t i = 0; i < size; ++i) {
		envp[i] = new char[env[i].length() + 1];
		std::strcpy(envp[i], env[i].c_str());
	}
	envp[size] = NULL;
	return (envp);
}

void	Cgi::delete_envp(char*** envp)
{
	for (size_t i = 0; (*envp)[i] != NULL; ++i) {
		delete [] (*envp)[i];
	}
	delete [] *envp;
	*envp = NULL;
}

// PUBLIC FUNCTIONS
Cgi::Cgi(socket_t *response_socket)
: epoll_item_t(cgi),
  m_status(init),
  m_response_socket(response_socket),
  m_child_pid(-1)
{}

Cgi::~Cgi(void)
{
	m_pipes.clear();
	terminate_child();
}

void	Cgi::reset_state(int epoll_inst)
{
	switch (m_status) {
		case write_to_child:
			// REMOVE FD IN TRACKING AND CLOSE BOTH PIPES
			epoll_ctl_ex(epoll_inst, EPOLL_CTL_DEL, m_pipes.fd_in, NULL);
			close(m_pipes.fd_in);
			m_pipes.fd_in = -1;
			close(m_pipes.fd_out);
			m_pipes.fd_out = -1;
			break ;
		case read_from_child:
			// REMOVE FD OUT TRACKING AND CLOSE OUT PIPE
			epoll_ctl_ex(epoll_inst, EPOLL_CTL_DEL, m_pipes.fd_out, NULL);
			close(m_pipes.fd_out);
			m_pipes.fd_out = -1;
			break ;
		default:
			if (m_pipes.fd_in != -1 || m_pipes.fd_out != -1)
				throw std::logic_error("Pipes fd are not cleared by CGI runtime");
	}
	m_status = init;
	terminate_child();
	m_body.clear();
	m_cgi_response.clear();
	// SET ICI LE EPOLL_CTL_ADD DU FD DU SOCKET EN EPOLLOUT A NOUVEAU ?
}

void	Cgi::clear(void)
{
	if (m_pipes.fd_in != -1 || m_pipes.fd_out != -1)
		throw std::logic_error("Pipes fd are not cleared by CGI runtime");
	m_status = init;
	terminate_child();
	m_body.clear();
	m_cgi_response.clear();
	m_response_socket = NULL;
}

void		Cgi::set_body(std::string &body)
{
	m_body = body;
}

void		Cgi::set_socket(socket_t *socket)
{
	m_response_socket = socket;
}

socket_t		*Cgi::get_socket(void)
{
	return (m_response_socket);
}

void	Cgi::terminate_child(void)
{
	if (m_child_pid != -1)
		kill(m_child_pid, SIGTERM);
	m_child_pid = -1;
}

void	Cgi::exec(const char* script_name, const char* script_dir, const char* script_path, char** envp, int epoll_inst)
{
	int cgi_pipe_in[2];
	int cgi_pipe_out[2];

	if (pipe(cgi_pipe_in))
		throw _Response::internal_error("cgi: pipe() failed");
	if (pipe(cgi_pipe_out)) {
		close(cgi_pipe_in[0]);
		close(cgi_pipe_in[1]);
		throw _Response::internal_error("cgi: pipe() failed");
	}

	switch (m_child_pid = fork()) {
		case -1:
			close_pipes(cgi_pipe_in, cgi_pipe_out);
			throw _Response::internal_error("cgi: fork() failed");
			break;

		// CHILD
		case 0:
			int r_fd;
			if ((r_fd = dup2(cgi_pipe_in[0], STDIN_FILENO)) == -1) {
				try {
					close_pipes(cgi_pipe_in, cgi_pipe_out);
				} catch (std::logic_error &e) {
					delete_envp(&envp);
					std::exit(-1);
				}
			}
			int w_fd;
			if ((w_fd = dup2(cgi_pipe_out[1], STDOUT_FILENO)) == -1) {
				try {
					close_pipes(cgi_pipe_in, cgi_pipe_out);
				} catch (std::logic_error &e) {
					delete_envp(&envp);
					std::exit(-1);
				}
			}
			try {
					close_pipes(cgi_pipe_in, cgi_pipe_out);
				} catch (std::logic_error &e) {
					delete_envp(&envp);
					std::exit(-1);
				}

			if (chdir(script_dir) == -1) {
				delete_envp(&envp);
				std::exit(-1);
			}

			const char*	argv[2];
			argv[0] = script_name;
			argv[1] = NULL;

			if (execve(script_path, const_cast<char**>(argv), envp) == -1) {
				delete_envp(&envp);
				std::exit(-1);
			}
			break ;

		// PARENT
		default:
			close(cgi_pipe_in[0]);
			close(cgi_pipe_out[1]);

			m_status = write_to_child;
			m_pipes.fd_in = cgi_pipe_in[1];
			m_pipes.fd_out = cgi_pipe_out[0];

			epoll_event	event = EpollManager::create(this, EPOLLOUT);

			errno = 0;
			epoll_ctl(epoll_inst, EPOLL_CTL_ADD, m_pipes.fd_in, &event);
			switch (errno) {
				case 0:
					break ;
				case ENOMEM:
				case ENOSPC:
					reset_state(epoll_inst);
					throw _Response::internal_error("cgi: not enought ressources for epoll_ctl()");
				default:
					throw CriticalException("cgi: critical failure of epoll_ctl()");
			}
			epoll_ctl_ex(epoll_inst, EPOLL_CTL_DEL, m_response_socket->fd, NULL); // stop report socket fd until CGI is not resolved
	}
}

int	Cgi::write_body_to_child(int epoll_inst, config_t &config)
{
	ssize_t		ret;

	if (m_body.size() > PIPE_BUF) {
		ret = write(m_pipes.fd_in, m_body.c_str(), PIPE_BUF);
		if (ret == -1)
			// CLEAN, SET FD TRACKING AGAIN, SEND ERR 500
			// handle_cgi_error(epoll_inst, config);
			return -1;
		else
			// CONTINUE WRITING TO CGI PIPE...
			m_body.erase(m_body.begin(), m_body.begin() + ret);
	}
	else {
		ret = write(m_pipes.fd_in, m_body.c_str(), m_body.size());
		if (ret == -1)
			// CLEAN, SET FD TRACKING AGAIN, SEND ERR 500
			// handle_cgi_error(epoll_inst, config);
			return -1;
		else if (static_cast<size_t>(ret) == m_body.size()) {
			// DONE --> CLOSE WRITING PIPE END AND TRACK READING END
			epoll_event	new_event = EpollManager::create(this, EPOLLIN);
			epoll_ctl_ex(epoll_inst, EPOLL_CTL_DEL, m_pipes.fd_in, NULL);
			close(m_pipes.fd_in);
			m_pipes.fd_in = -1;
			epoll_ctl_ex(epoll_inst, EPOLL_CTL_ADD, m_pipes.fd_out, &new_event);
			m_status = read_from_child;
		}
		else
			// CONTINUE WRITING TO CGI PIPE...
			m_body.erase(m_body.begin(), m_body.begin() + ret);
	}
}

int	Cgi::read_child_response(int epoll_inst, config_t &config)
{
	ssize_t	ret;
	char	buf[50000];

	ret = read(m_pipes.fd_out, &buf, 50000);
	if (ret == -1 || m_cgi_response.size() + 50000 >= 1000000)
		// CLEAN, SET FD TRACKING AGAIN, SEND ERR 500
		// handle_cgi_error(epoll_inst, config);
		return -1;
	else if (ret > 0) {
		// CONTINUE READING...
		buf[ret] = '\0';
		m_cgi_response.append(buf);
	}
	else if (ret == 0) {
		// CLIENT CLOSED --> CLEAN
		clear_cgi_pipes(epoll_inst);
		terminate_child();
		// SET FD TRACKING AGAIN AND HANDLE RESPONSE
		epoll_event new_event = EpollManager::create(m_response_socket, EPOLLOUT);
		epoll_ctl(epoll_inst, EPOLL_CTL_ADD, m_response_socket->fd, &new_event);
		m_status = done;
	}
}

// void	Cgi::clear_cgi_pipes(int epoll_inst)
// {
// 	if (m_pipes.fd_in != -1) {
// 		epoll_ctl_ex(epoll_inst, EPOLL_CTL_DEL, m_pipes.fd_in, NULL);
// 		close(m_pipes.fd_in);
// 		m_pipes.fd_in = -1;
// 	}
// 	if (m_pipes.fd_out != -1) {
// 		epoll_ctl_ex(epoll_inst, EPOLL_CTL_DEL, m_pipes.fd_out, NULL);
// 		close(m_pipes.fd_out);
// 		m_pipes.fd_out = -1;
// 	}
// }

// pid_t	Cgi::get_child_pid(void) const
// {

// }

// status_t	Cgi::get_status(void)
// {

// }

// pipes_t	Cgi::get_pipes_data(void) const
// {

// }
