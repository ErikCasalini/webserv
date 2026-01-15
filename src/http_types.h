#ifndef HTTP_TYPES_H
# define HTTP_TYPES_H

# include <map>
# include <string>

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

struct request_t
{
	method_t method;
	std::string target;
	protocol_t protocol;
	std::map<std::string, std::string> header;
	std::string body;
};

#endif
