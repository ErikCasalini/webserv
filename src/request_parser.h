#ifndef REQUEST_PARSER_H
# define REQUEST_PARSER_H

# include "http_types.h"
# include "request_parser_states.h"
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

	void _append_buffer(const char* str); // For testing
	void clear();
	void clear_infos();
	void parse();
	const request_t& get_infos() const;

	class BadRequest : public std::runtime_error {
	public:
		BadRequest(const char* msg);
	};
	class NotImplemented : public std::runtime_error {
	public:
		NotImplemented(const char* msg);
	};
	class ConnectionClosed : public std::runtime_error {
	public:
		ConnectionClosed(const char* msg);
	};

	int m_sockfd;

	// Give the state machine access to the private members of Request.
	// TODO: rename Init to a better name
	friend class RequestStates::Init;
	friend class RequestStates::ReadingBuffer;
	friend class RequestStates::ParsingHead;
	friend class RequestStates::ExtractingBody;
	friend class RequestStates::Done;
	friend class RequestStates::Invalid;
private:
	request_t m_infos;
	std::string m_buffer;
	size_t m_recv_buf_size;
	size_t m_pos;
	// bool m_extracting_body;
	RequestState* m_state;
	void set_state(RequestState* state);
	void erase_parsed();
	void _clear();
	void _clear_infos();
};

// Helper functions namespace
namespace _Request {
	using std::string;

	typedef std::map<string, string> raw_headers_t;

	ssize_t read_socket(int sockfd, string& buffer, size_t read_size);

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
	string extract_body(
			const string& buffer,
			size_t& pos,
			request_t& request);
}

#endif
