#include "Response.hpp"
#include <cstring>

Response::Response(void)
: m_sockfd(-1), m_buffer("TEST BUFFER")
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

size_t	Response::get_buf_size(void) const
{
	return (m_buffer.size());
}

void	Response::clear(void)
{
	m_sockfd = -1;
	m_request = request_t(); // a dire a Jules
	m_buffer.clear();
}
