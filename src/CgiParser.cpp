#include "CgiParser.h"
#include <cerrno>
#include <cstdlib>

using std::string;

CgiParser::BadCgiResponse::BadCgiResponse(const std::string& msg)
	: std::runtime_error(("cgi response: " + msg).c_str())
{};

CgiParser::CgiParser(Response* response, const std::string& nl)
	: m_response(response)
	, m_nl(nl)
{
}

void CgiParser::init()
{
	const string::size_type body_start = extract_headers();
	extract_body(body_start);
}

CgiParser::~CgiParser() {}

void CgiParser::parse()
{
	try {
		init();
		dispatch_response();
	} catch (const BadCgiResponse& e) {
		m_response->set_error(internal_err, m_response->m_location->error_page.at(internal_err));
	}
	m_response->generate_response();
}

std::string::size_type CgiParser::extract_headers()
{
	const string response = m_response->m_buffer;
	const string::size_type i = response.find(m_nl + m_nl);
	if (i == string::npos)
		throw BadCgiResponse("unexpected EOF");
	const string::size_type len = i + 2;
	try {
		m_headers.set_map(Headers::string_to_map(response.substr(0, len), m_nl));
	} catch (const Headers::BadHeader& e) {
		throw BadCgiResponse(e.what());
	}
	return (len + 2);
}

void CgiParser::extract_body(std::string::size_type body_start)
{
	m_body = m_response->m_buffer.substr(body_start);
}

void CgiParser::dispatch_response()
{
	if (is_error_status()) {
		parse_error();
	} else if (m_body.size() > 0) {
		if (m_headers.count("location") == 1)
			parse_redirdoc();
		else
			parse_document();
	} else if (m_headers.count("location") == 1) {
		if (m_headers.at("location").find("http://") == 0)
			parse_client_redir();
		else
			parse_local_redir();
	} else {
		throw BadCgiResponse("invalid response");
	}
}

void CgiParser::parse_redirdoc()
{
	if (m_headers.count("status") == 0
			|| status_to_enum(m_headers.at("status")) != 302
			|| m_headers.at("location").size() == 0)
		throw BadCgiResponse("invalid response");

	m_response->m_status = perm_redir;

	m_response->m_headers.location = m_headers.at("location");

	if (m_headers.count("content-type") == 1
			&& m_headers.at("content-type").size() != 0) {
		m_response->m_headers.content_type = m_headers.at("content-type");
	} else {
		m_response->m_headers.content_type = m_response->m_config.http.default_type;
	}

	m_response->m_headers.content_length = parse_content_length_header();

	m_response->m_body = m_body;
}

void CgiParser::parse_document()
{
	if (m_headers.count("status") == 1
			&& status_to_enum(m_headers.at("status")) != ok)
		throw BadCgiResponse("invalid response");
	else
		m_response->m_status = ok;

	if (m_headers.count("content-type") != 1
			|| m_headers.at("content-type").size() == 0)
		throw BadCgiResponse("invalid response");
	else
		m_response->m_headers.content_type = m_headers.at("content-type");

	m_response->m_headers.content_length = parse_content_length_header();

	m_response->m_body = m_body;
}

void CgiParser::parse_client_redir()
{
	check_redir();
	craft_redir();
}

void CgiParser::parse_local_redir()
{
	check_redir();
	craft_redir();
}

bool CgiParser::is_error_status()
{
	if (m_headers.count("status") == 1) {
		const status_t status = status_to_enum(m_headers.at("status"));
		if ((status >= 400 && status <= 405)
				|| (status >= 500 && status <= 502))
			return (true);
	}
	return (false);
}

unsigned long CgiParser::parse_content_length_header()
{
	if (m_headers.count("content-length") == 1
			&& m_headers.at("content-lenght").size() != 0) {
		const unsigned long len = string_to_ul(m_headers.at("content-length"));
		if (len != m_body.size())
			throw BadCgiResponse("invalid response");
		else
			return (len);
	} else {
		return (m_body.size());
	}
}

void CgiParser::parse_error()
{
	const status_t status = status_to_enum(m_headers.at("status"));
	if (!((status >= 400 && status <= 405)
			|| (status >= 500 && status <= 502))) {
		m_response->m_status = internal_err;
		if (status == 400)
			m_response->m_headers.keep_alive = false;
	} else {
		m_response->m_status = status_to_enum(m_headers.at("status"));
	}
	m_response->set_error(m_response->m_status,
							m_response->m_location->error_page.at(m_response->m_status));
}

void CgiParser::check_redir()
{
	if (m_headers.get_map().size() != 1
		|| m_body.size() != 0)
		throw BadCgiResponse("invalid response");
}

void CgiParser::craft_redir()
{
	m_response->m_status = moved_perm;
	m_response->m_headers.location = m_headers.at("location");
}

status_t CgiParser::status_to_enum(const std::string& status)
{
	char* end = NULL;
	errno = 0;
	const long n = std::strtol(status.c_str(), &end, 10);
	if (*end != ' ' || errno || n < 0 || n > 511) {
		errno = 0;
		throw BadCgiResponse("invalid response");
	}
	return (static_cast<status_t>(n));
}

unsigned long CgiParser::string_to_ul(const std::string& str)
{
	char* end;
	errno = 0;
	const unsigned long n = std::strtoul(str.c_str(), &end, 10);
	if (*end || errno) {
		errno = 0;
		throw BadCgiResponse("invalid response");
	}
	return (n);
}

std::string CgiParser::body_size_to_str()
{
	std::stringstream ss;
	ss << m_body.size();
	return (ss.str());
}
