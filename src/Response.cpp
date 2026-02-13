#include "Response.hpp"
#include "Config.h"
#include <cstring>
#include <cstdlib>
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
		std::list<std::string>		segments;
		bool						is_dir = false;
		size_t						pos;
		std::string::const_iterator	seg_end;
		std::string::const_iterator	seg_beg;

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

	unsigned char	url_decode(const std::string &url_code)
	{
		if (!isxdigit(url_code[0])
			|| !isxdigit(url_code[1]))
			throw std::invalid_argument("Wrong url encoding format");

		int tmp = std::strtol(url_code.c_str(), NULL, 16);
		if (tmp == 0x0 || tmp == 0x2F || tmp == 0x5C)
			throw std::invalid_argument("Url expands to forbidden symbol");
		return (static_cast<unsigned char>(tmp));
	}

	void	decode_segments(std::list<std::string> &segments)
	{
		std::list<std::string>::iterator	it_list = segments.begin();
		size_t								pos;

		for (size_t i = 0; i < segments.size(); i++, it_list++) {
			std::string				decoded; // new decoded string
			std::string::iterator	it_seg = it_list->begin(); // original string iterator

			decoded.reserve(it_list->size());
			pos = 0;
			while ((pos = it_list->find('%', pos)) != std::string::npos) {
				if (it_list->begin() + pos >= it_list->end() - 2) // if '%' is one of the 2 last chars --> bad URL encoding (%A or %)
					throw std::invalid_argument("Wrong url encoding format");
				decoded.append(*it_list, it_seg - it_list->begin(), pos - (it_seg - it_list->begin()));
				decoded.append(1, url_decode(it_list->substr(pos + 1, 2)));
				pos += 3;
				it_seg = it_list->begin() + pos;
			}
			decoded.append(*it_list, it_seg - it_list->begin());
			*it_list = decoded;
		}
	}

	std::string	create_path(std::list<std::string> &segments)
	{
		std::string				ret;
		bool					is_dir = false;
		std::list<std::string>	new_path;

		if (segments.back() == ""
			|| segments.back() == "."
			|| segments.back() == "..")
			is_dir = true;

		while (segments.size()) {
			if (segments.front() == "" || segments.front() == ".")
				segments.pop_front();
			else if (segments.front() == "..") {
				if (new_path.size() > 0)
					new_path.pop_back();
				segments.pop_front();
			}
			else {
				new_path.push_back(segments.front());
				segments.pop_front();
			}
		}

		for (std::list<std::string>::iterator it = new_path.begin(); it != new_path.end(); it++) {
			ret.append("/");
			ret.append(*it);
		}

		if ((is_dir && new_path.size() > 0)
			|| ret.size() == 0)
			ret.append("/");

		return (ret);
	}
}

Response::Response(void)
: m_socket(NULL),
  m_status(ok)
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

status_t	Response::get_status(void)
{
	return (m_status);
}

void	Response::clear(void)
{
	m_socket = NULL;
	m_request.clear();
	m_buffer.clear();
	m_path.clear();
	m_querry.clear();
	m_status = ok;
}

void	Response::parse_uri(void)
{
	if (!m_request.target.size()
		|| m_request.target[0] != '/'
		|| m_request.target.find_first_not_of(authorized_chars) != std::string::npos) {
		m_status = bad_request;
		return ;
	}

	_Response::extract_uri_elem(m_request.target, m_path, m_querry);
	std::list<std::string>	segments(_Response::split_path(m_path));
	try {
		_Response::decode_segments(segments);
	}
	catch (std::invalid_argument &e) {
		m_status = bad_request;
		return ;
	}
	m_path = _Response::create_path(segments);
}

// void	Response::generate(config_t &config)
// {
	//chercher m_path dans config
		//si pas present on remplit avec fichier 404
		//si present
// }
