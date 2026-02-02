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
