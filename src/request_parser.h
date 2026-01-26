#ifndef REQUEST_PARSER_H
# define REQUEST_PARSER_H

# include "http_types.h"
# include <stdexcept>
# include <string>

// TODO: add an init function that creates the method and protocol maps
class Request
{
public:
	Request();

	void read_socket();
	void parse();
	const request_t& get_request() const;

	class BadRequest : public std::runtime_error {
	public:
		BadRequest(const char* msg);
	};
	class NotImplemented : public std::runtime_error {
	public:
		NotImplemented(const char* msg);
	};
private:
	request_t m_request;
	std::string m_buffer;
};

// Helper functions namespace
namespace _Request {
	void consume_sp(const std::string& buffer, size_t& pos);
	void consume_crlf(const std::string& buffer, size_t& pos);
	void consume_ows_cr(const std::string& buffer, size_t& pos);

	method_t parse_method(const std::string& buffer, size_t& pos);
	std::string parse_target(const std::string& buffer, size_t& pos);
	protocol_t parse_protocol(const std::string& buffer, size_t& pos);

	std::string extract_key(const std::string& buffer, size_t& pos);
	std::string extract_values(const std::string& buffer, size_t& pos);
}

#endif
