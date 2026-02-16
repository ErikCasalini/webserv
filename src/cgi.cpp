#include "cgi.h"
#include "http_types.h"
#include <cstring>
#include <sys/epoll.h>
#include <unistd.h>

using std::string;
using std::vector;

void delete_envp(char*** envp)
{
	for (size_t i = 0; (*envp)[i] != NULL; ++i) {
		delete [] (*envp)[i];
	}
	delete [] *envp;
	*envp = NULL;
}

char** build_envp(const vector<string>& env)
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

const vector<string> prepare_env(const Response& response)
{
	vector<string> env;
	const request_t& request = response.get_request();

	string content_length = "CONTENT_LENGTH=";
	if (request.headers.content_length > 0) {
		content_length = content_length 
			+ to_string(request.headers.content_length);
	}
	env.push_back(content_length);

	// TODO: implement content_type header in request_t and parser
	// env["CONTENT_TYPE"] = request.headers.content_type;
	
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	// TODO: how do we handle the path info?
	// env.push_back("PATH_INFO=") = ;
	// env.push_back("PATH_TRANSLATED=") = ;
	env.push_back("QUERY_STRING=" + response.get_querry());
	// TODO: how to get the ip of the client? Look in the code
	// env.push_back("REMOTE_ADDR=") = client ip;
	// env.push_back("REMOTE_HOST=") = client hostname;
	env.push_back("REQUEST_METHOD=" + to_string(request.method));
	// env.push_back("SCRIPT_NAME=") = ;
	// env.push_back("SERVER_NAME=") = ;
	// env.push_back("SERVER_PORT=") = ;
	env.push_back("SERVER_PROTOCOL=HTTP/1.0");
	env.push_back("SERVER_SOFTWARE=webserv/2026");
	// env.push_back("HTTP_COOKIE=") = ;

	return (env);
}

int close_pipes(int p1[2], int p2[2])
{
	if (close(p1[0]))
		return (-1);
	if (close(p1[1]))
		return (-1);
	if (close(p2[0]))
		return (-1);
	if (close(p2[1]))
		return (-1);
	return (0);
}

// Catch exceptions to free envp or switch to an object
string exec_cgi(
		const char* script_name,
		const char* script_dir,
		const char* script_path,
		string request_body,
		char** envp)
{
	string response;

	// Open pipes.
	int cgi_pipe_in[2];
	int cgi_pipe_out[2];
	if (pipe(cgi_pipe_in))
		throw std::runtime_error("cgi: pipe() failed");
	if (pipe(cgi_pipe_out))
		throw std::runtime_error("cgi: pipe() failed");

	// Create the process to exec the cgi script.
	pid_t pid;
	switch (pid = fork()) {
		case -1:
			throw std::runtime_error("cgi: fork() failed");
			break;

		// In child process.
		// TODO: throwing in the child process will just abort it,
		// how do we signal an error to the parent process?
		case 0:
			// map the pipes to STD{IN,OUT}
			int r_fd;
			if ((r_fd = dup2(cgi_pipe_in[0], STDIN_FILENO)) == -1) {
				close_pipes(cgi_pipe_in, cgi_pipe_out);
				throw std::runtime_error("cgi: dup2() failed");
			}
			int w_fd;
			if ((w_fd = dup2(cgi_pipe_out[1], STDOUT_FILENO)) == -1) {
				close_pipes(cgi_pipe_in, cgi_pipe_out);
				throw std::runtime_error("cgi: dup2() failed");
			}
			if (close_pipes(cgi_pipe_in, cgi_pipe_out))
				throw std::runtime_error("cgi: close() failed");
			// Set directory to SCRIPT_FILENAME.
			// TODO: how to handle different errors (non existant dir, no access...) see man
			if (chdir(script_dir) == -1)
				throw std::runtime_error("cgi: chdir() failed");
			// Execute the script.
			const char* argv[2];
			argv[0] = script_name;
			argv[1] = NULL;
			// TODO: do we need to close STD{IN,OUT}_FILENO?
			if (execve(script_path, const_cast<char**>(argv), envp) == -1) {
				delete_envp(&envp);
				throw std::runtime_error("cgi: execve() failed");
			}
			break;

		// In parent process
		default:
			// TODO: check for a maximum size and send it in multiple parts if it exceeds it.

			// Send the body to the cgi.
			size_t nbyte = request_body.size() + 1;
			ssize_t w = 0;
			w = write(cgi_pipe_in[1], (request_body + "\n").c_str(), nbyte);
			if (w < 0 || static_cast<size_t>(w) != nbyte)
				throw std::runtime_error("cgi: write() failed");

			// Read the response of the cgi
			int epoll_fd = epoll_create1(0);
			if (epoll_fd == -1)
				throw std::runtime_error("cgi: epoll_create() failed");
			struct epoll_event event;
			event.events = EPOLLIN;
			event.data.fd = cgi_pipe_out[0];
			if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cgi_pipe_out[0], &event))
				throw std::runtime_error("cgi: epoll_ctl() failed");
			struct epoll_event events[1];
			switch (epoll_wait(epoll_fd, events, 1, 1000)) {
				case -1:
					throw std::runtime_error("cgi: epoll_wait() failed");
					break;
				default:
					char buf[8192];
					ssize_t n = 1;
					while (n > 0) {
						n = read(cgi_pipe_out[0], buf, 8192);
						if (n < 0)
							throw std::runtime_error("cgi: read() failed");
					}
					response.append(buf, static_cast<size_t>(n));
			}

			// Close the fds.
			if (close_pipes(cgi_pipe_in, cgi_pipe_out))
				throw std::runtime_error("cgi: close() failed");
			if (close(epoll_fd) < 0)
				throw std::runtime_error("cgi: close() failed");
			break;
	}
	return (response);
}
