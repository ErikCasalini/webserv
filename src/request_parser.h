#ifndef REQUEST_PARSER_H
# define REQUEST_PARSER_H

# include "http_types.h"
# include "request_parser_states.h"
# include <stdexcept>
# include <string>
# include <sys/types.h>

# ifndef REQUEST_RECV_SIZE
#  define REQUEST_RECV_SIZE 51200 // 50 KO
# endif

// TODO: add an init function that creates the method and protocol maps
class Request {
public:
	Request(const config_t& config);
	Request(const config_t& config, std::string buffer); // For testing purposes
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

	socket_t* m_socket;

	// Give the state machine access to the private members of Request.
	// TODO: rename Init to a better name
	friend class RequestStates::Init;
	friend class RequestStates::ReadingBuffer;
	friend class RequestStates::ParsingHead;
	friend class RequestStates::ExtractingBody;
	friend class RequestStates::Done;
	friend class RequestStates::Invalid;
private:
	Request();
	const config_t& m_config;
	request_t m_infos;
	std::string m_buffer;
	size_t m_pos;
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

	ssize_t read_socket(int sockfd, string& buffer);

	void consume_sp(const string& buffer, size_t& pos);
	void consume_crlf(const string& buffer, size_t& pos);
	void consume_ows_cr(const string& buffer, size_t& pos);

	method_t parse_method(const string& buffer, size_t& pos);
	string parse_target(const string& buffer, size_t& pos);
	protocol_t parse_protocol(const string& buffer, size_t& pos);

	std::string extract_headers(const string& m_buffer, size_t& pos);
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
