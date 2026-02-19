#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "request_parser.h"
# include "Config.h"
# include "http_types.h"
# include <string>
# include <list>

class Response
{

public:

											Response(void);
											~Response(void);
	// 										Response(const Response &src);
	// Response								&operator=(const Response &rhs);
	const request_t							&get_request(void) const;
	void									set_request(const request_t &request);
	void									clear(void);
	const char								*get_buf(void) const;
	const std::string						&get_path(void) const;
	const std::string						&get_querry(void) const;
	size_t									get_buf_size(void) const;
	void									parse_uri(void);
	status_t								get_status(void);
	void									process(const config_t &config);
	int										send_response(void);

	socket_t								*m_socket;
	static const char						authorized_chars[];

private:

	void										generate_target(location_t &location);
	void										generate_response(void);
	void										fill_body(location_t &location);
	void										set_body_headers(void);
	void										set_error(status_t status, std::string &error_body);
	void										handle_static_request(location_t &location);
	static std::map<int, std::string>			init_status_codes(void);
	static const std::map<int, std::string>		&get_status_codes(void);

	request_t									m_request;
	std::string									m_buffer;
	std::string									m_querry;
	std::string									m_path; //
	std::list<std::string>						m_path_segments; // besoin des 3 champs path?
	std::string									m_target; //
	headers_t									m_headers;
	status_t									m_status;
	std::string									m_version;
	std::string									m_body; // probablement remplacé par get_file_content()
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

	void						extract_uri_elem(std::string&uri, std::string &path, std::string &querry);
	std::list<std::string>		split_path(const std::string &path);
	unsigned char				url_decode(const std::string &url_code);
	void						decode_segments(std::list<std::string> &segments);
	std::string					create_path(std::list<std::string> &segments);
	bool						is_exact_match(const std::list<std::string> &path, const std::list<std::string> &location);
	int							evaluate_path_matching(const std::list<std::string> &path, const std::list<std::string> &location);
	const location_t			&find_location(const std::list<std::string> &path, const std::vector<location_t> &locations);
	bool						is_bad_method(method_t method, std::vector<method_t> &limit_except);
}

#endif
