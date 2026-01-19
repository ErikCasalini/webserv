#ifndef REQUEST_PARSER_H
# define REQUEST_PARSER_H

# include "http_types.h"
# include <string>

class Request
{
public:
	Request();
	void read_socket();
	void parse();
	const request_t& get_request() const;
private:
	request_t m_request;
	std::string m_buffer;
};

// Helper functions namespace
namespace _Request {
	void consume_single_whitespace(const std::string& buffer, size_t& pos);
	void consume_single_crlf(const std::string& buffer, size_t& pos);
	method_t parse_method(const std::string& buffer, size_t& pos);
	std::string parse_target(const std::string& buffer, size_t& pos);
	protocol_t parse_protocol(const std::string& buffer, size_t& pos);
}

#endif
