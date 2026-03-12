#include "Response.hpp"
#include "Config.h"
#include <cstring>
#include <cstdlib>
#include "http_types.h"
#include <list>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <climits>
#include <ctime>
#include "EpollManager.hpp"
#include "Sockets.hpp"
#include "parse_uri_utils.h"
#include "response_utils.h"
#include "general_utils.h"
#include "../include/c_network_exception.h"
#include "Cookies.hpp"

using std::vector;

const char	Response::authorized_chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~" // Unreserved
							":/?[]@#" // Reserved gen-delims
							"!$&'()*+,;=" // Reserved sub-delims
							"%"; // Url encoding

Response::Response(const config_t &config, Cookies &cookie_jar)
: m_socket(NULL),
  m_config(config),
  m_status(ok),
  m_version("HTTP/1.0"),
  m_cgi(m_socket, config, this),
  m_storage(config),
  m_location(NULL),
  m_cookies(cookie_jar)
{}

Response::~Response(void)
{}

Response	&Response::operator=(const Response &rhs)
{
	if (this != &rhs) {
		m_request = rhs.m_request;
		m_buffer = rhs.m_buffer;
		m_querry = rhs.m_querry;
		m_path = rhs.m_path;
		m_path_segments = rhs.m_path_segments;
		m_target = rhs.m_target;
		m_headers = rhs.m_headers;
		m_status = rhs.m_status;
		m_version = rhs.m_version;
		m_body = rhs.m_body;
		m_cgi = rhs.m_cgi;
		m_storage = rhs.m_storage;
		m_location = rhs.m_location;
		m_cookies = rhs.m_cookies;
	}
	return (*this);
}

request_t	&Response::get_request(void)
{
	return (m_request);
}

void	Response::set_request(const request_t &request)
{
	m_request = request;
}

void	Response::set_storage_infos(const storage_t *storage)
{
	m_storage.set_storage_infos(storage);
}

Cookies	&Response::get_cookies(void)
{
	return (m_cookies);
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

status_t	Response::get_status(void) const
{
	return (m_status);
}

headers_t	&Response::get_headers(void)
{
	return (m_headers);
}

cgi_status_t	Response::get_cgi_status(void) const
{
	return (m_cgi.get_status());
}

const location_t	*Response::get_location(void) const
{
	return (m_location);
}

void	Response::set_status(status_t status)
{
	m_status = status;
}

void	Response::reset_cgi(int epoll_inst)
{
	m_cgi.reset_state(epoll_inst);
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
	m_cgi.clear();
	m_storage.clear();
	m_location = NULL;
}

void	Response::parse_uri(void)
{
	if (!m_request.target.size()
		|| m_request.target[0] != '/'
		|| m_request.target.find_first_not_of(authorized_chars) != std::string::npos) {
		m_status = bad_request;
		return ;
	}

	extract_uri_elem(m_request.target, m_path, m_querry);
	m_path_segments = split_path(m_path);
	try {
		decode_segments(m_path_segments);
	}
	catch (std::invalid_argument &e) {
		m_status = bad_request;
		return ;
	}
	m_path = create_path(m_path_segments);
	m_status = ok;
}

bool	Response::cgi_timeout(void)
{
	return (m_cgi.timeout());
}

void	Response::generate_response(void)
{
	std::stringstream	buf;

	buf << m_version << ' ' << static_cast<int>(m_status) << ' ' << Response::get_status_codes().at(m_status) << CRLF;

	switch (m_status) {
		case method_not_allowed:
			buf << "Allow: " << m_headers.allow << CRLF
				<< "Content-Length: " << m_headers.content_length << CRLF;
			break ;
		case no_content:
			break ;
		case created:
		case temp_redir:
		case perm_redir:
			buf << "Location: " << m_headers.location << CRLF
				<< "Content-Type: " << m_headers.content_type << CRLF
				<< "Content-Length: " << m_headers.content_length << CRLF;
			break ;
		case moved_perm:
		case moved_temp:
			buf << "Location: " << m_headers.location << CRLF
				<< "Content-Length: " << m_headers.content_length << CRLF;
			break ;
		default:
			buf << "Content-Type: " << m_headers.content_type << CRLF
				<< "Content-Length: " << m_headers.content_length << CRLF;
			break ;
		}
		buf << "Connection: " << (m_headers.keep_alive ? "Keep-Alive" : "Close") << CRLF
		<< "Server: " << m_headers.server << CRLF
		<< "Date: " << get_date(std::time(NULL)) << CRLF;
		if (m_headers.set_cookie.size())
			buf << "Set-Cookie: " << m_headers.set_cookie << CRLF;
		buf << CRLF;

	if (m_body.size())
		buf << m_body;

	m_buffer = buf.str();
}

void	Response::set_error(status_t status, const std::string &error_body)
{
	m_status = status;
	m_body = error_body;
	m_headers.content_length = m_body.size();
	m_headers.content_type = "text/html";
	if (status == bad_request)
		m_headers.keep_alive = false;
}

void	Response::set_redirection(status_t status, const std::string &redir_addr)
{
	if (status == perm_redir || status == temp_redir) {
		m_body = m_request.body;
		m_headers.content_length = m_request.headers.content_length;
		m_headers.content_type = m_request.headers.content_type;
	}
	m_headers.location = redir_addr;
	m_status = status;
}

void	Response::handle_static_request()
{
	file_stat	type = get_file_type(m_target);

	switch (type) {
		case nonexistent:
			set_error(not_found, m_location->error_page.at(not_found));
			return ;
		case bad_perms:
			set_error(forbidden, m_location->error_page.at(forbidden));
			return ;
		case error:
			set_error(internal_err, m_location->error_page.at(internal_err));
			return ;
		case dir:
			if (m_target.at(m_target.size() - 1) != '/') {
				if (m_request.method == post)
					set_redirection(perm_redir, m_path + '/');
				else
					set_redirection(moved_perm, m_path + '/');
				return ;
			}
			break ;
		default: // file
			break ;
	}

	if (m_request.method == post) { // on est surs ?
		m_headers.allow = "GET, DELETE";
		set_error(method_not_allowed, m_location->error_page.at(method_not_allowed));
		return;
	}

	if (m_request.method == del) {
		if (type == dir)
			set_error(forbidden, m_location->error_page.at(forbidden));
		else {
			errno = 0;
			std::remove(m_target.c_str());
			switch (errno) {
				case 0:
					m_status = no_content;
					break ;
				case EPERM:
				case EACCES:
					set_error(forbidden, m_location->error_page.at(forbidden));
					break ;
				default:
					set_error(internal_err, m_location->error_page.at(internal_err));
			}
		}
		return ;
	}

	if (type == dir) {
		std::string	index_testing(m_target + m_location->index);

		switch (get_file_type(index_testing)) {
			case file:
				m_target = index_testing;
				break ;
			case dir:
				if (index_testing.at(index_testing.size() - 1) != '/') {
					if (m_request.method == post)
						set_redirection(perm_redir, m_path + m_location->index + '/');
					else
						set_redirection(moved_perm, m_path + m_location->index + '/');
					return ;
				}
				__attribute__((fallthrough));
			case nonexistent:
				if (m_location->autoindex) {
					m_status = generate_indexing(m_target, m_body, m_path); //+ m_target IF DIR PATH DO NOT EXIST --> NOT FOUND, IF BAD PERM --> FORBIDDEN, ELSE --> INTERNAL ERR
					if (m_status != ok)
						set_error(m_status, m_location->error_page.at(m_status));
					else {
						m_headers.content_length = m_body.size();
						m_headers.content_type = "text/html";
					}
					return ;
				}
				else
					__attribute__((fallthrough));
			case bad_perms:
				set_error(forbidden, m_location->error_page.at(forbidden));
				return ;
			case error:
				set_error(internal_err, m_location->error_page.at(internal_err));
				return ;
		}
	}
	m_status = read_file_to_body(m_target, m_body);
	if (m_status != ok)
		set_error(m_status, m_location->error_page.at(m_status));
	set_body_headers(m_headers, m_body, m_target, m_config);
}

void	Response::init_cgi(void)
{
	m_cgi.set_body(&m_request.body);
	m_cgi.set_response_buf(&m_buffer);
	m_cgi.set_socket(m_socket);
	m_cgi.set_location(m_location);
}

const vector<std::string> Response::generate_cgi_env(const cgi_uri_infos_t &uri_infos) const
{
	vector<std::string>	env;

	if (m_request.headers.content_length)
		env.push_back("CONTENT_LENGTH=" + to_string(m_request.headers.content_length));
	else
		env.push_back("CONTENT_LENGTH=");
	env.push_back("CONTENT_TYPE=" + m_request.headers.content_type);
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	// env.push_back("HTTP_COOKIE=") = ;
	env.push_back("PATH_INFO=" + uri_infos.path_info);
	env.push_back("QUERY_STRING=" + m_querry);
	env.push_back("REMOTE_ADDR=" + m_socket->str_peer_addr());
	env.push_back("REQUEST_METHOD=" + to_string(m_request.method));
	env.push_back("SCRIPT_NAME=" + uri_infos.script_name);
	env.push_back("SERVER_NAME=" + m_socket->str_local_addr());
	env.push_back("SERVER_PORT=" + m_socket->str_local_port());
	env.push_back("SERVER_PROTOCOL=HTTP/1.0");
	env.push_back("SERVER_SOFTWARE=" VERSION);
	return (env);
}

void	Response::handle_cgi_error(Sockets &sockets)
{
	epoll_event	new_event = EpollManager::create(m_socket, EPOLLOUT);

	m_cgi.reset_state(sockets.epoll_inst());
	std::cout << "\033[1;31m[CGI INTERNAL ERROR]\033[0m " << *m_socket << '\n';
	epoll_ctl_ex(sockets.epoll_inst(), EPOLL_CTL_ADD, m_socket->fd, &new_event);
	set_status(internal_err);
	process(sockets);
}

void	Response::handle_cgi(Sockets &sockets)
{
	cgi_uri_infos_t	cgi_uri_infos;

	// IF SCRIPT NAME/INDEX IS EMPTY --> ATTEMPT TO INDEX CGI FOLDER
	if (cgi_uri_infos.init(*m_location, m_path_segments) == -1) {
		if (m_location->autoindex) {
			m_status = generate_indexing(cgi_uri_infos.script_abs_path, m_body, m_path); //+ m_target IF DIR PATH DO NOT EXIST --> NOT FOUND, IF BAD PERM --> FORBIDDEN, ELSE --> INTERNAL ERR
			if (m_status != ok)
				set_error(m_status, m_location->error_page.at(m_status));
			else {
				m_headers.content_length = m_body.size();
				m_headers.content_type = "text/html";
			}
		}
		else
			set_error(forbidden, m_location->error_page.at(forbidden));
		throw Cgi::cgi_error("cgi: script name is empty, attempt to index dir");
	}

	switch(get_file_type(cgi_uri_infos.script_abs_path)) {
		case file:
			if (access(cgi_uri_infos.script_abs_path.c_str(), X_OK) < 0) {
				set_error(forbidden, m_location->error_page.at(forbidden));
				throw Cgi::cgi_error("cgi: can't open script");
			}
			break ;
		case dir:
		case nonexistent:
			set_error(not_found, m_location->error_page.at(not_found));
			throw Cgi::cgi_error("cgi: can't open script");
		case bad_perms:
			set_error(forbidden, m_location->error_page.at(forbidden));
			throw Cgi::cgi_error("cgi: can't open script");
		case error:
			set_error(internal_err, m_location->error_page.at(internal_err));
			throw Cgi::cgi_error("cgi: can't open script");
		default:
			throw CriticalException("cgi: get_cgi_file() returned unexpected file type");
	}

	const vector<std::string>	env(generate_cgi_env(cgi_uri_infos));
	char						**envp = m_cgi.allocate_envp(env);

	try {
		m_cgi.exec(cgi_uri_infos.script_name.c_str(),
				cgi_uri_infos.script_dir.c_str(),
				envp,
				sockets);
	}
	catch (internal_error &e) {
		m_cgi.delete_envp(&envp);
		set_error(internal_err, m_location->error_page.at(internal_err));
		throw Cgi::cgi_error("cgi: execution failed");
	}
	catch (CriticalException &e) {
		m_cgi.delete_envp(&envp);
		throw CriticalException("cgi: critical failure of epoll_ctl()");
	}
	m_cgi.delete_envp(&envp);
	init_cgi(); // --> on set NPH ou normal CGI
	epoll_ctl_ex(sockets.epoll_inst(), EPOLL_CTL_DEL, m_socket->fd, NULL); // stop report socket fd until CGI is not resolved
}

void	Response::generate_target()
{
	if (m_location->root == "")
		throw bad_config("generate_target: no root is set");
	m_target = m_location->root; // root doit commencer par '/'
	if (m_target.at(m_target.size() - 1) == '/')
		m_target.resize(m_target.size() - 1);
	m_target.append(m_path);
}

void	Response::process(Sockets &sockets)
{
	switch (m_status) {
		case bad_request:
			if (m_location)
				set_error(bad_request, m_location->error_page.at(bad_request));
			else
				set_error(bad_request, m_config.http.server.at(m_socket->server_id).error_page.at(bad_request));
			generate_response();
			return ;
		case internal_err:
			if (m_location)
				set_error(internal_err, m_location->error_page.at(internal_err));
			else
				set_error(internal_err, m_config.http.server.at(m_socket->server_id).error_page.at(internal_err));
			generate_response();
			return ;
		default:
			break ;
	}

	// COOKIE HANDLING
	if (m_cookies.find(m_request.headers.cookies)) {
		std::time_t	curr_time = std::time(NULL);

		m_cookies.at(m_request.headers.cookies).last_visit = curr_time;
		m_cookies.at(m_request.headers.cookies).last_visit_str = get_date(curr_time);
	}
	else
		m_headers.set_cookie = m_cookies.create();

	// STORAGE
	if (m_storage.init(m_path_segments) == 0) {
		m_status = m_storage.exec(m_request, m_body, m_headers);
		if (m_status != ok && m_status != no_content && m_status != created)
			set_error(m_status, m_config.http.server.at(m_socket->server_id).error_page.at(m_status));
	}
	// NORMAL LOCATION PROCESSING
	else
	{
		try {
			m_location = find_location(m_path_segments, m_config.http.server.at(m_socket->server_id).locations);
		}
		catch (bad_location &e) {
			set_error(not_found, m_config.http.server.at(m_socket->server_id).error_page.at(not_found));
			generate_response();
			return ;
		}

		if (is_bad_method(m_request.method, m_location->limit_except)) {
			set_error(forbidden, m_location->error_page.at(forbidden));
			generate_response();
			return ;
		}
		else if (m_location->redirection.first) {
			set_redirection(m_location->redirection.first, m_location->redirection.second);
			generate_response();
			return ;
		}
		else if (!m_location->cgi) {
			try {
				generate_target();
			}
			catch (bad_config &e) {
				set_error(not_found, m_config.http.server.at(m_socket->server_id).error_page.at(not_found));
				generate_response();
				return ;
			}
		}

		if (m_location->cgi) {
			try {
				handle_cgi(sockets);
				return ;
			}
			catch (Cgi::cgi_error &e) {
				generate_response();
				return ;
			}
		}
		else
			handle_static_request();
	}
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
		m_status = sending_resp;
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
	status_codes.insert(std::make_pair(307, std::string("Temporary redirect")));
	status_codes.insert(std::make_pair(308, std::string("Permanent redirect")));
	status_codes.insert(std::make_pair(400, std::string("Bad request")));
	status_codes.insert(std::make_pair(403, std::string("Forbidden")));
	status_codes.insert(std::make_pair(404, std::string("Not found")));
	status_codes.insert(std::make_pair(405, std::string("Method not allowed")));
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
