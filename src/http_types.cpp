#include "http_types.h"

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
	os << "method: ";
	switch (m) {
	case del:
		os << "del";
		break ;
	case get:
		os << "get";
		break ;
	case post:
		os << "post";
		break ;
	}
	return (os);
}

std::ostream& operator<<(std::ostream& os, const protocol_t& p)
{
	os << "protocol: ";
	switch (p) {
	case zero_nine:
		os << "zero_nine";
		break ;
	case one:
		os << "one";
		break ;
	case one_one:
		os << "one_one";
		break ;
	case two:
		os << "two";
		break ;
	case three:
		os << "three";
		break ;
	}
	return (os);
}

std::ostream& operator<<(std::ostream& os, const status_t& s)
{
	os << "status: ";
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
	case waiting_cgi:
		os << "waiting_cgi";
		break ;
	case writing:
		os << "writing";
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

std::ostream& operator<<(std::ostream& os, const request_t& r)
{
	os
		<< "request:\n"
		<< '\t' << r.status << '\n'
		<< '\t' << r.method << '\n'
		<< '\t' << r.target << '\n'
		<< '\t' << r.protocol << '\n'
		<< '\t' << r.headers << '\n'
		<< '\t' << r.body << '\n';
	return (os);
}

socket_t::socket_t()
: fd(-1),
  type(passive),
  server_id(-1),
  data_len(sizeof(data)),
  peer_data_len(sizeof(peer_data))
{
	std::memset(&(this->peer_data), 0, sizeof(this->peer_data));
	std::memset(&(this->data), 0, sizeof(this->data));
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
	this->type = passive;
	this->data_len = sizeof(data);
	this->peer_data_len = sizeof(peer_data);
	std::memset(&(this->peer_data), 0, sizeof(this->peer_data));
	std::memset(&(this->data), 0, sizeof(this->data));
}

std::string	socket_t::str_peer_data(void) const
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

std::string	socket_t::str_data(void) const
{
	std::ostringstream	ret;
	uint32_t			ip;
	uint16_t			port;

	ip = ntohl(this->data.sin_addr.s_addr); // convert ip to system endian (little)
	port = ntohs(this->data.sin_port); // convert port to system endian (little)
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
	   << " | Type: " << s.type
	   << " | Server ID: " << s.server_id
	   << " | Peer data: " << s.str_peer_data()
	   << " | Local data: " << s.str_data() << '\n';
	return (os);
}
