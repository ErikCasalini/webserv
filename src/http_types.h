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
	ok,
	bad_request, // 400
	not_implemented // 501
};

typedef std::map<std::string, std::vector<std::string> > headers_t;

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
