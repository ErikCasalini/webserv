#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "request_parser.h"
# include "Config.h"
# include "http_types.h"
# include <string>
# include <list>
# include "EpollManager.hpp"
# include "Cgi.hpp"

class Response
{

public:

											Response(void);
											~Response(void);
	void									clear(void);

	request_t								&get_request(void);
	const char								*get_buf(void) const;
	const std::string						&get_path(void) const;
	const std::string						&get_querry(void) const;
	size_t									get_buf_size(void) const;
	status_t								get_status(void);
	void									set_request(const request_t &request);
	void									set_status(status_t status);

	void									parse_uri(void);
	void									process(const config_t &config, int epoll_inst);
	void									init_cgi(void);
	void									handle_cgi_error(int epoll_inst, config_t &config);
	int										send_response(void);


	socket_t								*m_socket;
	static const char						authorized_chars[];

private:

	void									generate_target(const location_t &location);
	void									generate_response(void);
	void									generate_indexing(void);
	const cgi_uri_infos_t					generate_cgi_uri_info(const location_t &location_path, std::list<std::string> path) const;
	const std::vector<std::string>			generate_cgi_env(const cgi_uri_infos_t &uri_infos) const;
	void									fill_body(const location_t &location);
	void									set_body_headers(void);
	void									set_error(status_t status, const std::string &error_body);
	void									handle_static_request(const location_t &location);
	void									handle_cgi(const location_t &location, int epoll_inst);
	static std::map<int, std::string>		init_status_codes(void);
	static const std::map<int, std::string>	&get_status_codes(void);
	file_stat								get_file_type(const location_t &location);
	file_stat								get_cgi_file_type(const location_t &location, const std::string &target);
	file_stat								get_index_file_type(const location_t &location);
	void									set_redirection(status_t status, const std::string &redir_addr);
	std::string 							get_current_date(void);


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
};

namespace _Response
{
	class	bad_location : public std::runtime_error
	{
		public:
			bad_location(const std::string &str)
			: std::runtime_error(str)
			{};
	};

	class	forbidden : public std::runtime_error
	{
		public:
			forbidden(const std::string &str)
			: std::runtime_error(str)
			{};
	};

	class	internal_error : public std::runtime_error
	{
		public:
			internal_error(const std::string &str)
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

	void					extract_uri_elem(std::string&uri, std::string &path, std::string &querry);
	std::list<std::string>	split_path(const std::string &path);
	unsigned char			url_decode(const std::string &url_code);
	void					decode_segments(std::list<std::string> &segments);
	std::string				create_path(std::list<std::string> &segments);
	bool					is_exact_match(const std::list<std::string> &path, const std::list<std::string> &location);
	int						evaluate_path_matching(const std::list<std::string> &path, const std::list<std::string> &location);
	const location_t		&find_location(const std::list<std::string> &path, const std::vector<location_t> &locations);
	bool					is_bad_method(method_t method, std::vector<method_t> &limit_except);
}

#endif
