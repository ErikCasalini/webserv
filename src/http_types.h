#ifndef HTTP_TYPES_H
# define HTTP_TYPES_H

# include <map>
# include <string>
# include <vector>

# define CRLF "\r\n"

enum method_t
{
	del,
	get,
	post
};

enum protocol_t
{
	zero_nine,
	one,
	one_one,
	two,
	three
};

enum status_t
{
	parsing,
	ok,
	bad_request, // 400
	not_implemented // 501
};

struct headers_t
{
	unsigned long content_length;
	std::string cookies;
	bool keep_alive;
	// TODO: store it as a date type? What format to handle
	std::string if_modified_since;
};

struct request_t
{
	status_t status;
	method_t method;
	std::string target;
	protocol_t protocol;
	headers_t headers;
	std::string body;
};

#endif
