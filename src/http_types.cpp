#include "http_types.h"

// headers_t struct
headers_t::headers_t()
	: content_length(0)
	, keep_alive(false)
{}

headers_t::headers_t(const headers_t& src)
	: content_length(src.content_length)
	, cookies(src.cookies)
	, keep_alive(src.keep_alive)
	, if_modified_since(src.if_modified_since)
{}

headers_t& headers_t::operator=(const headers_t& src)
{
	if (this != &src) {
		content_length = src.content_length;
		cookies = src.cookies;
		keep_alive = src.keep_alive;
		if_modified_since = src.if_modified_since;
	}
	return (*this);
}

void headers_t::clear()
{
	content_length = 0;
	cookies.clear();
	keep_alive = false;
	if_modified_since.clear();
}

std::ostream& operator<<(std::ostream& os, const headers_t& h)
{
	os
		<< "headers: [\n"
		<< "\tcontent_length: " << h.content_length << '\n'
		<< "\tcookies: " << h.cookies << '\n'
		<< "\tkeep_alive: " << (h.keep_alive ? "true\n" : "false\n")
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
