#ifndef REQUEST_PARSER_H
# define REQUEST_PARSER_H

# include "http_types.h"
# include <stdexcept>
# include <string>
# include <sys/types.h>

// TODO: add an init function that creates the method and protocol maps
class Request {
public:
	Request();
	Request(std::string buffer); // For testing purposes
	Request(const Request& src);
	Request& operator=(const Request& src);

	ssize_t read_socket();
	void clear();
	void clear_request();
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

	int m_sockfd;
private:
	request_t m_request;
	std::string m_buffer;
	size_t m_recv_buf_size;
};

// Helper functions namespace
namespace _Request {
	using std::string;

	typedef std::map<string, string> raw_headers_t;

	void consume_sp(const string& buffer, size_t& pos);
	void consume_crlf(const string& buffer, size_t& pos);
	void consume_ows_cr(const string& buffer, size_t& pos);

	method_t parse_method(const string& buffer, size_t& pos);
	string parse_target(const string& buffer, size_t& pos);
	protocol_t parse_protocol(const string& buffer, size_t& pos);

	string extract_key(const string& buffer, size_t& pos);
	string extract_values(const string& buffer, size_t& pos);
	raw_headers_t extract_headers(const string& m_buffer, size_t& pos);
	unsigned long parse_content_length(const raw_headers_t& raw_headers);
	bool parse_connection(const raw_headers_t& raw_headers);
	headers_t parse_headers(
			const string& buffer,
			size_t& pos,
			const request_t& request);
	string extract_body(const string& buffer, size_t& pos, request_t& request);
}

#endif
