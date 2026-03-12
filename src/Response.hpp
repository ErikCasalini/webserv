#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "request_parser.h"
# include "Config.h"
# include "http_types.h"
# include <string>
# include <list>
# include "EpollManager.hpp"
# include "Cgi.hpp"
# include "Storage.hpp"
# include "Cookies.hpp"

class Response
{

public:

											Response(const config_t &config, Cookies &cookie_jar);
											~Response(void);
	Response								&operator=(const Response &rhs);
	void									clear(void);

	request_t								&get_request(void);
	const char								*get_buf(void) const;
	const std::string						&get_path(void) const;
	const std::string						&get_querry(void) const;
	size_t									get_buf_size(void) const;
	status_t								get_status(void) const;
	cgi_status_t							get_cgi_status(void) const;
	headers_t								&get_headers(void);
	Cookies									&get_cookies(void);
	void									set_request(const request_t &request);
	void									set_status(status_t status);
	void									set_storage_infos(const storage_t *storage);

	void									parse_uri(void);
	void									process(Sockets &sockets);
	void									init_cgi(void);
	void									reset_cgi(int epoll_inst);
	void									handle_cgi_error(Sockets &sockets);
	int										send_response(void);
	bool									cgi_timeout(void);


	socket_t								*m_socket;
	static const char						authorized_chars[];

	friend class Cgi;
	friend class CgiParser;

private:

	void									generate_target(void);
	void									generate_response(void);
	const std::vector<std::string>			generate_cgi_env(const cgi_uri_infos_t &uri_infos) const;
	void									set_error(status_t status, const std::string &error_body);
	void									handle_static_request(void);
	void									handle_cgi(Sockets &sockets);
	static std::map<int, std::string>		init_status_codes(void);
	static const std::map<int, std::string>	&get_status_codes(void);
	void									set_redirection(status_t status, const std::string &redir_addr);

	const config_t							&m_config;
	request_t								m_request;
	std::string								m_buffer;
	std::string								m_querry;
	std::string								m_path; //
	std::list<std::string>					m_path_segments; // besoin des 3 champs path?
	std::string								m_target; //
	headers_t								m_headers;
	status_t								m_status;
	std::string								m_version;
	std::string								m_body;
	Cgi										m_cgi;
	Storage									m_storage;
	const location_t						*m_location;
	Cookies									&m_cookies;

};

#endif
