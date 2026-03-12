#ifndef CGIPARSER_H
# define CGIPARSER_H

# include "Headers.h"
# include "Cgi.hpp"
# include "Response.hpp"
# include <string>

class CgiParser {
public:
	class BadCgiResponse : public std::runtime_error {
	public:
		BadCgiResponse(const std::string& msg);
	};

	CgiParser(Response* response, const std::string& nl = CRLF);
	~CgiParser();

	void init();
	void parse();

private:
	CgiParser(const CgiParser& src);
	CgiParser();
	CgiParser operator=(const CgiParser& src);

	std::string::size_type extract_headers();
	void extract_body(std::string::size_type body_start);
	void dispatch_response();
	void parse_redirdoc();
	void parse_document();
	void parse_client_redir();
	void parse_local_redir();
	unsigned long parse_content_length_header();
	void parse_error();
	void check_redir();
	void craft_redir();
	bool is_error_status();
	status_t status_to_enum(const std::string& status);
	std::string body_size_to_str();
	unsigned long string_to_ul(const std::string& str);

	Response *m_response;
	Headers m_headers;
	std::string m_body;
	const std::string m_nl;
};

#endif
