#include "Response.hpp"
#include <cstring>
#include "http_types.h"

const char	Response::authorized_chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~" // Unreserved
							":/?[]@#" // Reserved gen-delims
							"!$&'()*+,;=" // Reserved sub-delims
							"%"; // Url encoding

Response::Response(void)
: m_sockfd(-1)
{}

Response::~Response(void)
{}

const request_t	&Response::get_request(void) const
{
	return (m_request);
}

void	Response::set_request(const request_t &request)
{
	m_request = request;
}

const char	*Response::get_buf(void) const
{
	return (m_buffer.c_str());
}

const std::string	&Response::get_path(void) const
{
	return (m_path);
}

const std::string &Response::get_querry(void) const
{
	return(m_querry);
}

size_t	Response::get_buf_size(void) const
{
	return (m_buffer.size());
}

void	Response::clear(void)
{
	m_sockfd = -1;
	m_request.clear();
	m_buffer.clear();
	m_path.clear();
	m_querry.clear();
}

void	extract_uri_elem(std::string&uri, std::string &path, std::string &querry)
{
	size_t		pos = uri.find('?');
	size_t		end = std::string::npos;

	path.assign(uri, 0, pos);
	if (pos != std::string::npos) { // if querry present
		pos++; // after '?'
		end = uri.find(pos, '#');
		if (end != std::string::npos)
			querry.assign(uri, pos, end - pos); // querry is everything between ? and #
		else
			querry.assign(uri, pos); // querry is everything after '?' (or nothing if pos = m_request.target.size)
	}
}

void	Response::parse_uri(void)
{
	if (!m_request.target.size()
		|| m_request.target[0] != '/'
		|| m_request.target.find_first_not_of(authorized_chars) != std::string::npos) {
		m_request.status = bad_request;
		return ;
	}
	extract_uri_elem(m_request.target, m_path, m_querry);
	

}
