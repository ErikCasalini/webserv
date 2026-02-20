#include "Response.hpp"
#include "Config.h"
#include <cstring>
#include <cstdlib>
#include "http_types.h"
#include <list>
#include <iostream>
#include <fstream>
#include <sstream>

using std::vector;

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
		std::stringstream		stream_path(path);
		std::string				temp;
		std::list<std::string>	segments;
		bool					is_dir = false;

		if (stream_path.str().at(stream_path.str().size() - 1) == '/')
			is_dir = true;

		std::getline(stream_path, temp, '/');
		if (stream_path.bad())
			throw std::runtime_error("Internal reading path error");

		while (std::getline(stream_path, temp, '/')) {
			segments.push_back("/");
			if (temp.size())
				segments.push_back(temp);
		}
		if (stream_path.bad())
			throw std::runtime_error("Internal reading path error");

		if (is_dir)
			segments.push_back("/");

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

	std::string	create_path(std::list<std::string> &segments) // assumes segments starts with "/"
	{
		std::string				ret;
		std::list<std::string>	new_path;

		while (segments.size()) {
			if (segments.front() == "/") {
				new_path.push_back(segments.front());
				segments.pop_front();
				while (segments.size() && segments.front() == "/")
					segments.pop_front();
			}
			else if (segments.front() == ".") {
				segments.pop_front();
				if (segments.size())
					segments.pop_front();
			}
			else if (segments.front() == "..") {
				if (new_path.size() > 1) {
					new_path.pop_back();
					new_path.pop_back();
				}
				segments.pop_front();
				if (segments.size())
					segments.pop_front();
			}
			else {
				new_path.push_back(segments.front());
				segments.pop_front();
			}
		}

		for (std::list<std::string>::iterator it = new_path.begin(); it != new_path.end(); it++) {
			ret.append(*it);
		}

		segments = new_path; // update m_path_segments
		return (ret); // return string format path
	}

	bool	is_exact_match(const std::list<std::string> &path, const std::list<std::string> &location)
	{
		std::list<std::string>::const_iterator	it_path = path.begin();
		std::list<std::string>::const_iterator	it_loc = location.begin();

		if (path.size() == 0 || location.size() == 0)
			throw (bad_location("Empty path or location segments"));

		if (path.size() != location.size())
			return (false);

		for (size_t i = 0; i < path.size(); i++, it_path++, it_loc++) {
			if (*it_path != *it_loc)
				return (false);
		}
		return (true);
	}

	int	evaluate_path_matching(const std::list<std::string> &path, const std::list<std::string> &location)
	{
		std::list<std::string>::const_iterator	it_path = path.begin();
		std::list<std::string>::const_iterator	it_loc = location.begin();
		std::list<std::string>::const_iterator	loc_end = location.end();
		int										match_rate = 0;

		if (path.size() == 0 || location.size() == 0)
			throw (bad_location("Empty path or location segments"));
		if (path.size() < location.size())
			return (0);

		for (; it_loc != loc_end; it_path++, it_loc++) {
			if (*it_path != *it_loc)
				return (0);
			match_rate++;
		}
		return (match_rate);
	}

	const location_t	&find_location(const std::list<std::string> &path, const vector<location_t> &locations) // assume location path starts with '/'
	{
		vector<location_t>::const_iterator	it_loc = locations.begin();
		vector<location_t>::const_iterator	ret = locations.end();

		for (int best_match = 0, match_rate; it_loc < locations.end(); it_loc++) {
			if (it_loc->exact_match) {
				if (is_exact_match(path, it_loc->path))
					return (*it_loc);
			}
			else {
				match_rate = evaluate_path_matching(path, it_loc->path);
				if (match_rate > best_match) {
					best_match = match_rate;
					ret = it_loc;
				}
			}
		}
		if (ret == locations.end())
			throw (bad_location("Uri matchs no locations"));

		return (*ret);
	}

	bool	is_bad_method(method_t method, std::vector<method_t> &limit_except)
	{
		for (std::vector<method_t>::const_iterator it = limit_except.begin(); it != limit_except.end(); it++) {
			if (method == *it)
				return (false);
		}
		return (true);
	}

}

Response::Response(void)
: m_socket(NULL),
  m_status(ok),
  m_version("HTTP/1.0")
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
	m_path_segments.clear();
	m_target.clear();
	m_headers.clear();
	m_version = "HTTP/1.0";
	m_body.clear();
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
	m_path_segments = _Response::split_path(m_path);
	try {
		_Response::decode_segments(m_path_segments);
	}
	catch (std::invalid_argument &e) {
		m_status = bad_request;
		return ;
	}
	m_path = _Response::create_path(m_path_segments);
}

void	Response::generate_response(void)
{
	std::stringstream	buf;

	buf << m_version << ' ' << static_cast<int>(m_status) << ' ' << Response::get_status_codes().at(m_status) << "\r\n";

	switch (m_status) {
		case no_content:
			break ;
		case created:
		case moved_perm:
		case moved_temp:
			buf << "Location: " << m_headers.location << "\r\n";
			break ;
		default:
			buf << "Content-Type: " << m_headers.content_type << "\r\n";
			buf << "Content-Length: " << m_headers.content_length << "\r\n";
			break ;
	}
	buf << "Connection: " << (m_headers.keep_alive ? "Keep-Alive" : "Close") << "\r\n"
		<< "Server: " << m_headers.server << "\r\n"
		<< "Date:\r\n" // << _Response::get_date() << "\r\n"
		<< "\r\n";

	if (m_body.size())
		buf << m_body;

	m_buffer = buf.str();
}

void	Response::set_error(status_t status, std::string &error_body)
{
	m_status = status;
	m_body = error_body;
	m_headers.content_length = m_body.size();
	m_headers.content_type = "text/html";
}

void	Response::fill_body(location_t &location)
{
	errno = 0;
	std::ifstream	file(m_target.c_str());
	std::string		temp;

	switch (errno) {
		case 0:
			break ;
		case ENOENT:
			errno = 0;
			set_error(not_found, location.error_page.at(not_found));
			return ;
		case EACCES:
			errno = 0;
			set_error(forbidden, location.error_page.at(forbidden));
			return ;
		default:
			errno = 0;
			set_error(internal_err, location.error_page.at(internal_err));
			return ;
	}
	while (std::getline(file, temp))
		m_body.append(temp);

	if (file.bad())
		set_error(internal_err, location.error_page.at(internal_err));
	else
		m_status = ok;
}

void	Response::set_body_headers(void)
{
	m_headers.content_length = m_body.size();
	m_headers.content_type = "text/html"; // a changer
}

void	Response::handle_static_request(location_t &location)
{
	// index is already appended if present
	if (m_request.method == post) {
		set_error(method_not_allowed, location.error_page.at(method_not_allowed));
		return ;
	}

	// if (m_request.method == del) {
	// 	if (m_target.back() == '/')
	// 		set_error(forbidden, location.error_page.at(forbidden));
	// 	else {
	// 		m_status = remove_file();
	// 		switch (m_status) {
	// 			case forbidden:
	// 			case not_found:
	// 			case internal_err:
	// 				set_error(m_status, location.error_page.at(m_status));
	// 				break ;
	// 			default:
	// 				break ;
	// 		}
	// 	}
	// 	return ;
	// }

	// if (m_target.back() == '/') { // si '/' alors aucun fichier index n'est spécifié
	// 	if (location.autoindex) {
	// 		try {
	// 			m_body = generate_indexing(m_target);
	// 		}
	// 		catch (_Response::internal_error &e) {
	// 			set_error(internal_err, location.error_page.at(internal_err));
	// 			return ;
	// 		}
	// 		m_status = ok;
	// 		set_body_headers();
	// 	}
	// 	else
	// 		set_error(forbidden, location.error_page.at(forbidden)); // pas d'index, pas de dir indexing
	// 	return ;
	// }
	fill_body(location);
	set_body_headers();
}

void	Response::generate_target(location_t &location)
{
	// if (location.cgi) {
	// 	// ??
	// 	return ;
	// }

	m_target = location.root; // root doit commencer par '/'
	if (m_target.at(m_target.size() - 1) == '/')
		m_target.resize(m_target.size() - 1);
	m_target.append(m_path);
	if (m_target.at(m_target.size() - 1) != '/')
		return ;
	m_target.append(location.index); // index ne doit pas avoir de '/' ni '\'
}

void	Response::process(const config_t &config)
{
	location_t	location;

	if (m_status == bad_request) {
		set_error(bad_request, location.error_page.at(bad_request));
		generate_response();
		return ;
	}

	// UPLOAD
	// if (m_path_segments == config.http.server.at(m_socket->server_id).upload.first)
	// 	handle_storage_request();

	// NORMAL LOCATION PROCESSING
	// else {
		// try {
			location = _Response::find_location(m_path_segments, config.http.server.at(m_socket->server_id).locations);
		// }
		// catch (_Response::bad_location &e) {
		// 	(void)e;
		// 	// find_err_page(config) recursive search
		// 	set_error(not_found, ????);
		// 	generate_response();
		// 	return ; // m_buffer sera rempli et pret a send()
		// }

		if (_Response::is_bad_method(m_request.method, location.limit_except))
			set_error(forbidden, location.error_page.at(forbidden));
		else if (location.redirection.first) {
			m_headers.location = location.redirection.second;
			m_status = location.redirection.first;
		}
		else
			generate_target(location);

		// if (location.cgi)
		// 	handle_cgi(); // check method
		// else
			handle_static_request(location);
	// }
	generate_response();
}

int	Response::send_response(void)
{
	int	ret;

	if (m_socket == NULL)
		throw std::logic_error("Attempt to send invalid socket");
	ret = send(m_socket->fd, m_buffer.c_str(), (m_buffer.size() > 16000 ? 16000 : m_buffer.size()), 0);
	if (ret == 16000) {
		m_buffer.erase(m_buffer.begin(), m_buffer.begin() + 16000);
		m_status = writing;
	}
	else
		m_status = ok;
	return (ret);
}

std::map<int, std::string>	Response::init_status_codes(void)
{
	std::map<int, std::string> status_codes;

	status_codes.insert(std::make_pair(200, std::string("Ok")));
	status_codes.insert(std::make_pair(201, std::string("Created")));
	status_codes.insert(std::make_pair(204, std::string("No content")));
	status_codes.insert(std::make_pair(301, std::string("Moved permanently")));
	status_codes.insert(std::make_pair(302, std::string("Moved temporarily")));
	status_codes.insert(std::make_pair(400, std::string("Bad request")));
	status_codes.insert(std::make_pair(403, std::string("Forbidden")));
	status_codes.insert(std::make_pair(404, std::string("Not found")));
	status_codes.insert(std::make_pair(500, std::string("Internal server error")));
	status_codes.insert(std::make_pair(501, std::string("Not implemented")));
	status_codes.insert(std::make_pair(502, std::string("Bad gateway")));
	return (status_codes);
}

const std::map<int, std::string>	&Response::get_status_codes(void)
{
	static std::map<int, std::string> status_codes = init_status_codes();
	return (status_codes);
}
