#ifndef CGI_HPP
# define CGI_HPP

# include "http_types.h"
# include <vector>

struct config_t;

class	Cgi : public epoll_item_t
{

public:

					Cgi(socket_t *response_socket);
					~Cgi(void);
	void			clear(void);
	void			reset_state(int epoll_inst);
	void			clear_cgi_pipes(int epoll_inst);
	pid_t			get_child_pid(void) const;
	status_t		get_status(void);
	void			terminate_child(void);
	pipes_t			get_pipes_data(void) const;
	void			exec(const char* script_name, const char* script_dir, const char* script_path, char** envp, int epoll_inst);

	socket_t		*get_socket(void);

	int				write_body_to_child(int epoll_inst, config_t &config);
	int				read_child_response(int epoll_inst, config_t &config);

	void			set_body(std::string &body);
	void			set_socket(socket_t *socket);

	static void		delete_envp(char*** envp);
	static char**	allocate_envp(const std::vector<std::string>& env);

private:

	enum	cgi_status_t
	{
		init,
		write_to_child,
		read_from_child,
		done
	};

	void	close_pipes(int *p1, int *p2);


	cgi_status_t	m_status;
	std::string		m_body;
	socket_t		*m_response_socket;
	pid_t			m_child_pid;
	pipes_t			m_pipes;
	std::string		m_cgi_response;
};

#endif
