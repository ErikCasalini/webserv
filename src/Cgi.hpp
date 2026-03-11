#ifndef CGI_HPP
# define CGI_HPP

# include "http_types.h"
# include "Sockets.hpp"
# include <stdexcept>
# include <vector>
# include <ctime>

# ifndef CGI_TIMEOUT
#  define CGI_TIMEOUT 3
# endif

struct config_t;

class	Cgi : public epoll_item_t
{

public:

	class	ChildCriticalError : public std::runtime_error
	{
		public:
			ChildCriticalError(const std::string &str)
			: std::runtime_error(str)
			{};
	};
	class	cgi_error : public std::runtime_error
	{
		public:
			cgi_error(const std::string &str)
			: std::runtime_error(str)
			{};
	};

					Cgi(socket_t *response_socket);
					~Cgi(void);
	void			clear(void);
	void			reset_state(int epoll_inst);
	void			terminate_child(void);

	socket_t		*get_socket(void) const;
	cgi_status_t	get_status(void) const;
	pid_t			get_child_pid(void) const;
	void			set_body(std::string *body);
	void			set_response_buf(std::string *response_buf);
	void			set_socket(socket_t *socket);
	void			reset_child_pid(void);

	void			exec(const char* script_name, const char* script_dir, char** envp, Sockets &sockets);
	int				write_body_to_child(int epoll_inst);
	int				read_child_response(int epoll_inst);
	bool			timeout(void);


	static void		delete_envp(char*** envp);
	static char**	allocate_envp(const std::vector<std::string>& env);

private:

	void			close_pipes(int *p1, int *p2);

	cgi_status_t	m_status;
	socket_t		*m_response_socket;
	pid_t			m_child_pid;
	pipes_t			m_pipes;
	std::string		*m_request_body;
	std::string		*m_response_buf;
	std::time_t		m_last_activity;
	bool			m_is_child;
};

#endif
