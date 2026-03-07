#include "http_types.h"
#include <ctime>
#include "Config.h"
#include "general_utils.h"
#include "Cgi.hpp"

// headers_t struct
headers_t::headers_t()
	: content_length(0)
	, keep_alive(false)
	, server("Webserver_2026")
{}

headers_t::headers_t(const headers_t& src)
	: content_length(src.content_length)
	, content_type(src.content_type)
	, cookies(src.cookies)
	, keep_alive(src.keep_alive)
	, date(src.date)
	, location(src.location)
	, server(src.server)
	, if_modified_since(src.if_modified_since)
	, allow(src.allow)
{}

headers_t& headers_t::operator=(const headers_t& src)
{
	if (this != &src) {
		content_length = src.content_length;
		content_type = src.content_type;
		cookies = src.cookies;
		keep_alive = src.keep_alive;
		date = src.date;
		location = src.location;
		server = src.server;
		if_modified_since = src.if_modified_since;
		allow = src.allow;
	}
	return (*this);
}

void headers_t::clear()
{
	content_length = 0;
	content_type.clear();
	cookies.clear();
	keep_alive = false;
	date.clear();
	location.clear();
	server = "Webserver_2026";
	if_modified_since.clear();
	allow.clear();
}

std::ostream& operator<<(std::ostream& os, const headers_t& h)
{
	os
		<< "headers: [\n"
		<< "\tcontent_length: " << h.content_length << '\n'
		<< "\tcontent_type: " << h.content_type << '\n'
		<< "\tcookies: " << h.cookies << '\n'
		<< "\tkeep_alive: " << (h.keep_alive ? "true\n" : "false\n")
		<< "\tdate: " << h.date << '\n'
		<< "\tlocation: " << h.location << '\n'
		<< "\tserver: " << h.server << '\n'
		<< "\tif_modified_since: " << h.if_modified_since << "\n\t]\n";
	return (os);
}

// request_t struct
request_t::request_t()
	: status(parsing)
	, method(get)
	, protocol(zero_nine)
{}

request_t::request_t(const request_t& src)
	: status(src.status)
	, method(src.method)
	, target(src.target)
	, protocol(src.protocol)
	, headers(src.headers)
	, body(src.body)
{}

request_t& request_t::operator=(const request_t& src)
{
	if (this != &src) {
		status = src.status;
		method = src.method;
		target = src.target;
		protocol = src.protocol;
		headers = src.headers;
		body = src.body;
	}
	return (*this);
}

void request_t::clear()
{
	status = parsing;
	method = get;
	target.clear();
	protocol = zero_nine;
	headers.clear();
	body.clear();
}

std::ostream& operator<<(std::ostream& os, const method_t& m)
{
	switch (m) {
	case del:
		os << "DELETE";
		break ;
	case get:
		os << "GET";
		break ;
	case post:
		os << "POST";
		break ;
	}
	return (os);
}

std::ostream& operator<<(std::ostream& os, const protocol_t& p)
{
	switch (p) {
	case zero_nine:
		os << "0.9";
		break ;
	case one:
		os << "1.0";
		break ;
	case one_one:
		os << "1.1";
		break ;
	case two:
		os << "2";
		break ;
	case three:
		os << "3";
		break ;
	}
	return (os);
}

std::ostream& operator<<(std::ostream& os, const status_t& s)
{
	switch (s) {
	case parsing:
		os << "parsing";
		break ;
	case ok:
		os << "ok";
		break ;
	case bad_request:
		os << "bad_request";
		break ;
	case not_implemented:
		os << "not_implemented";
		break ;
	case sending_resp:
		os << "sending_resp";
		break ;
	case created:
		os << "created";
		break ;
	case no_content:
		os << "no_content";
		break ;
	case moved_perm:
		os << "moved_perm";
		break ;
	case moved_temp:
		os << "moved_temp";
		break ;
	case moved_perm_body:
		os << "moved_perm_body";
		break ;
	case forbidden:
		os << "forbidden";
		break ;
	case not_found:
		os << "not_found";
		break ;
	case method_not_allowed:
		os << "method_not_allowed";
		break ;
	case internal_err:
		os << "internal_err";
		break ;
	case bad_gateway:
		os << "bad_gateway";
		break ;
	}
	return (os);
}

std::ostream& operator<<(std::ostream &os, sock_type s)
{
	switch (s) {
		case passive:
			os << "passive";
			break ;
		case active:
			os << "active";
		default:
			break ;
	}
	return (os);
}

std::ostream& operator<<(std::ostream& os, const request_t& r)
{
	os
		<< "request:\n"
		<< "\tstatus: " << r.status << '\n'
		<< "\tmethod: " <<  r.method << '\n'
		<< "\ttarget: " << r.target << '\n'
		<< "\tprotocol: " << r.protocol << '\n'
		<< '\t' << r.headers << '\n'
		<< "\tbody: " << r.body << '\n';
	return (os);
}

epoll_item_t::epoll_item_t(fd_type type)
: type(type)
{}

socket_t::socket_t()
: epoll_item_t(sockt),
  fd(-1),
  socktype(passive),
  server_id(-1),
  local_data_len(sizeof(local_data)),
  peer_data_len(sizeof(peer_data)),
  last_activity(0)
{
	std::memset(&(this->peer_data), 0, sizeof(this->peer_data));
	std::memset(&(this->local_data), 0, sizeof(this->local_data));
}

bool socket_t::operator==(socket_t &rhs) const
{
	if (this->fd == rhs.fd)
		return (true);
	return (false);
}

void socket_t::clear()
{
	this->fd = -1;
	this->server_id = -1;
	this->type = sockt;
	this->socktype = passive;
	this->local_data_len = sizeof(local_data);
	this->peer_data_len = sizeof(peer_data);
	last_activity = 0;
	std::memset(&(this->peer_data), 0, this->peer_data_len);
	std::memset(&(this->local_data), 0, this->local_data_len);
}

bool	socket_t::timeout(config_t &config)
{
	if (last_activity && std::time(NULL) - last_activity > config.http.keepalive_timeout)
		return (true);
	return (false);
}

std::string	socket_t::str_peer_addr(void) const
{
	std::ostringstream	ret;
	uint32_t			ip;

	ip = ntohl(this->peer_data.sin_addr.s_addr);
	ret << ((ip >> 24) & 0XFF)
		<< '.'
		<< ((ip >> 16) & 0XFF)
		<< '.'
		<< ((ip >> 8) & 0XFF)
		<< '.'
		<< (ip & 0XFF);

	return (ret.str());
}

std::string	socket_t::str_peer_port(void) const
{
	std::ostringstream	ret;

	ret << ntohs(this->peer_data.sin_port);
	return (ret.str());
}

std::string	socket_t::str_peer_interface(void) const
{
	std::ostringstream	ret;
	uint32_t			ip;
	uint16_t			port;

	ip = ntohl(this->peer_data.sin_addr.s_addr); // convert ip to system endian (little)
	port = ntohs(this->peer_data.sin_port); // convert port to system endian (little)
		ret << ((ip >> 24) & 0XFF)
		<< '.'
		<< ((ip >> 16) & 0XFF)
		<< '.'
		<< ((ip >> 8) & 0XFF)
		<< '.'
		<< (ip & 0XFF)
		<< ":"
		<< port;
	return (ret.str());
}

std::string	socket_t::str_local_addr(void) const
{
	std::ostringstream	ret;
	uint32_t			ip;

	ip = ntohl(this->local_data.sin_addr.s_addr);
	ret << ((ip >> 24) & 0XFF)
		<< '.'
		<< ((ip >> 16) & 0XFF)
		<< '.'
		<< ((ip >> 8) & 0XFF)
		<< '.'
		<< (ip & 0XFF);

	return (ret.str());
}

std::string	socket_t::str_local_port(void) const
{
	std::ostringstream	ret;

	ret << ntohs(this->local_data.sin_port);
	return (ret.str());
}

std::string	socket_t::str_local_interface(void) const
{
	std::ostringstream	ret;
	uint32_t			ip;
	uint16_t			port;

	ip = ntohl(this->local_data.sin_addr.s_addr); // convert ip to system endian (little)
	port = ntohs(this->local_data.sin_port); // convert port to system endian (little)
		ret << ((ip >> 24) & 0XFF)
		<< '.'
		<< ((ip >> 16) & 0XFF)
		<< '.'
		<< ((ip >> 8) & 0XFF)
		<< '.'
		<< (ip & 0XFF)
		<< ":"
		<< port;
	return (ret.str());
}

std::ostream& operator<<(std::ostream& os, const socket_t& s)
{
	os << "Socket FD: " << s.fd
	   << " | Type: " << s.socktype
	   << " | Server ID: " << s.server_id
	   << " | Peer interface: " << s.str_peer_interface()
	   << " | Local interface: " << s.str_local_interface();
	return (os);
}

pipes_t::pipes_t()
: fd_in(-1),
  fd_out(-1)
{}

pipes_t::~pipes_t(void)
{
	clear();
}

void pipes_t::clear()
{
	if (fd_in != -1) {
		close(fd_in);
		fd_in = -1;
	}
	if (fd_out != -1) {
		close(fd_out);
		fd_out = -1;
	}
}

cgi_uri_infos_t::cgi_uri_infos_t(const location_t &location, std::list<std::string> path) // assumes location_paths ends with '/'
{
	std::list<std::string>::const_iterator	it_loc = location.path.begin();

	if (is_exact_match(location.path, path)) {
		if (location.index == "")
			throw (Cgi::cgi_error("Default script name is empty"));
		else {
			script_name = location.index;
			path_info = "";
		}
	}
	else {
		while (*it_loc == *path.begin())
			path.pop_front();
		script_name = *path.begin();

		path.pop_front();
		while (path.size()) {
			path_info += *path.begin();
			path.pop_front();
		}
	}

	script_dir += location.root;
	it_loc = location.path.begin();
	while (it_loc != location.path.end()) {
		script_dir += *it_loc;
		it_loc++;
	}

	script_abs_path = script_dir + script_name;
}
