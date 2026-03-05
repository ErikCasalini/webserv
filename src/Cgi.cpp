#include "Cgi.hpp"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/epoll.h>
#include <climits>
#include <cstdlib>
#include <cerrno>
#include <ctime>
#include "Response.hpp"
#include "Sockets.hpp"
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
  m_child_pid(-1),
  m_request_body(NULL),
  m_response_buf(NULL),
  m_last_activity(0),
  m_is_child(false)
{}

Cgi::~Cgi(void)
{
	m_pipes.clear();
	if (!m_is_child) {
		terminate_child();
	}
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
			throw std::logic_error("Attempt to reset Cgi in a forbidden state");
	}
	m_status = init;
	terminate_child();
	m_last_activity = 0;
}

void	Cgi::clear(void)
{
	if (m_pipes.fd_in != -1 || m_pipes.fd_out != -1)
		throw std::logic_error("Pipes fd are not cleared by CGI runtime");
	m_status = init;
	terminate_child();
	m_request_body = NULL;
	m_response_buf = NULL;
	m_response_socket = NULL;
	m_last_activity = 0;
}

void	Cgi::set_body(std::string *body)
{
	m_request_body = body;
}

void	Cgi::set_response_buf(std::string *response_buf)
{
	m_response_buf = response_buf;
}

void	Cgi::set_socket(socket_t *socket)
{
	m_response_socket = socket;
}

socket_t	*Cgi::get_socket(void) const
{
	return (m_response_socket);
}

cgi_status_t	Cgi::get_status(void) const
{
	return (m_status);
}

pid_t	Cgi::get_child_pid(void) const
{
	return (m_child_pid);
}

void	Cgi::reset_child_pid(void)
{
	m_child_pid = -1;
}

void	Cgi::terminate_child(void)
{
	if (m_child_pid != -1) {
		// CHECK IF CHILD IS ALREADY DEAD
		pid_t	w = waitpid(m_child_pid, NULL, WNOHANG);

		if (w == 0) {
			// CHILD STILL RUNNING --> KILL AND REAP
			kill(m_child_pid, SIGKILL);
			waitpid(m_child_pid, NULL, 0);
		}
	}
	m_child_pid = -1;
}

bool	Cgi::timeout(void)
{
	if (m_last_activity && (std::time(NULL) - m_last_activity > CGI_TIMEOUT))
		return (true);
	return (false);
}

void	Cgi::exec(const char* script_name, const char* script_dir, const char* script_path, char** envp, Sockets &sockets)
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

			m_is_child = true;
			sockets.close_all();
			close(sockets.epoll_inst());

			int r_fd;
			if ((r_fd = dup2(cgi_pipe_in[0], STDIN_FILENO)) == -1) {
				close_pipes(cgi_pipe_in, cgi_pipe_out);
				delete_envp(&envp);
				throw ChildCriticalError("\033[1;31m[CHILD CRITICAL ERROR]\033[0m: close_pipes() failed");
			}

			int w_fd;
			if ((w_fd = dup2(cgi_pipe_out[1], STDOUT_FILENO)) == -1) {
				close_pipes(cgi_pipe_in, cgi_pipe_out);
				delete_envp(&envp);
				throw ChildCriticalError("\033[1;31m[CHILD CRITICAL ERROR]\033[0m: close_pipes() failed");
			}

			close_pipes(cgi_pipe_in, cgi_pipe_out);

			if (chdir(script_dir) == -1) {
				delete_envp(&envp);
				throw ChildCriticalError("\033[1;31m[CHILD CRITICAL ERROR]\033[0m: chdir() failed");
			}

			const char*	argv[2];
			argv[0] = script_name;
			argv[1] = NULL;

			if (execve(script_path, const_cast<char**>(argv), envp) == -1) {
				delete_envp(&envp);
				throw ChildCriticalError("\033[1;31m[CHILD CRITICAL ERROR]\033[0m: execve() failed");
			}
			break ;

		// PARENT
		default:
			close(cgi_pipe_in[0]);
			close(cgi_pipe_out[1]);

			m_pipes.fd_in = cgi_pipe_in[1];
			m_pipes.fd_out = cgi_pipe_out[0];

			epoll_event	event = EpollManager::create(this, EPOLLOUT);

			epoll_ctl(sockets.epoll_inst(), EPOLL_CTL_ADD, m_pipes.fd_in, &event);
			switch (errno) {
				case 0:
					m_status = write_to_child;
					m_last_activity = std::time(NULL);
					break ;
				case ENOMEM:
				case ENOSPC:
					m_pipes.clear();
					throw _Response::internal_error("cgi: not enought ressources for epoll_ctl()");
				default:
					throw CriticalException("cgi: critical failure of epoll_ctl()");
			}
	}
}

int	Cgi::write_body_to_child(int epoll_inst)
{
	ssize_t	ret;

	if (m_request_body->size() > PIPE_BUF) {
		ret = write(m_pipes.fd_in, m_request_body->c_str(), PIPE_BUF);
		if (ret == -1)
			// CLEAN, SET FD TRACKING AGAIN, SEND ERR 500
			return (-1);
		else
			// CONTINUE WRITING TO CGI PIPE...
			m_request_body->erase(m_request_body->begin(), m_request_body->begin() + ret);
	}
	else {
		ret = write(m_pipes.fd_in, m_request_body->c_str(), m_request_body->size());
		if (ret == -1)
			// CLEAN, SET FD TRACKING AGAIN, SEND ERR 500
			return -1;
		else if (static_cast<size_t>(ret) == m_request_body->size()) {
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
			m_request_body->erase(m_request_body->begin(), m_request_body->begin() + ret);
	}
	return (0);
}

int	Cgi::read_child_response(int epoll_inst)
{
	ssize_t	ret;
	char	buf[PIPE_BUF + 1];

	ret = read(m_pipes.fd_out, &buf, PIPE_BUF);
	if (ret == -1 || m_response_buf->size() + PIPE_BUF >= 1000000 || timeout())
		// CLEAN, SET FD TRACKING AGAIN, SEND ERR 500
		return (-1);
	else if (ret > 0) {
		// CONTINUE READING...
		buf[ret] = '\0';
		m_response_buf->append(buf);
	}
	else if (ret == 0) {
		// CHILD CLOSED --> CHECK EXIT STATUS
		int		wstatus = 0;
		pid_t	w = waitpid(m_child_pid, &wstatus, WNOHANG);

		if (w < 0 || !WIFEXITED(wstatus) || WEXITSTATUS(wstatus) != 0) {
		// CHILD OR WAITPID FAILED --> CLEAN, SET FD TRACKING AGAIN, SEND ERR 500
			m_child_pid = -1;
			return (-1);
		}
		// CHILD SUCCEEDED OR NOT EXITED YET--> CLEAN, SET FD TRACKING AGAIN, HANDLE RESPONSE
		reset_state(epoll_inst);
		m_status = done;
		// parse_response (if NPH on envoie direct sinon on craft)
		m_last_activity = 0;
	}
	return (0);
}
