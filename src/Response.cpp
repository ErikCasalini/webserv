#include "Response.hpp"
#include <cstring>
#include "http_types.h"
#include <list>

const char	Response::authorized_chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~" // Unreserved
							":/?[]@#" // Reserved gen-delims
							"!$&'()*+,;=" // Reserved sub-delims
							"%"; // Url encoding

// Helpers for Response class
namespace _Response
{
	void	extract_uri_elem(std::string&uri, std::string &path, std::string &querry)
	{
		size_t	q_pos = uri.find('?');
		size_t	end = uri.find('#');

		if (q_pos != std::string::npos && q_pos < end) { // if querry present
			path.assign(uri, 0, q_pos);
			q_pos++; // after '?'
			if (end != std::string::npos)
				querry.assign(uri, q_pos, end - q_pos); // querry is everything between ? and #
			else
				querry.assign(uri, q_pos); // querry is everything after '?' (or nothing if q_pos = m_request.target.size)
		}
		else
			path.assign(uri, 0, end);
	}

	std::list<std::string>	split_path(const std::string &path) // assumes path starts with '/'
	{
		std::list<std::string>	segments;
		bool						is_dir = false;
		size_t						pos;
		std::string::const_iterator		seg_end;
		std::string::const_iterator		seg_beg;

		if (*(path.end() - 1) == '/')
			is_dir = true;

		pos = path.find('/', 1);
		seg_beg = path.begin();
		while (pos != std::string::npos) {
			seg_end = path.begin() + pos;
			std::string new_seg(seg_beg + 1, seg_end); // we remove both '/'
			segments.push_back(new_seg);
			seg_beg = path.begin() + pos;
			pos = path.find('/', pos + 1);
		}
		if (!is_dir) {
			std::string new_string(seg_beg + 1, path.end());
			segments.push_back(new_string);
		}
		else {
			std::string new_string("");
			segments.push_back(new_string);
		}
		return (segments);
	}
}

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

void	Response::parse_uri(void)
{
	if (!m_request.target.size()
		|| m_request.target[0] != '/'
		|| m_request.target.find_first_not_of(authorized_chars) != std::string::npos) {
		m_request.status = bad_request;
		return ;
	}
	_Response::extract_uri_elem(m_request.target, m_path, m_querry);


}
