#include "http_types.h"
#include "request_parser.h"
#include <cstring>
#include <string>

Request::Request()
{
	// PERF: is this call necessary as an error is returned on failure?
	// If an user try to read body while there isn't in this request:
	// in any case it must be avoided and the unique solution
	// is to use the struct with care.
	// In this struct the objects will be zero initialized
	// and only the structs will be filled with garbage
	// moreover if any of this structs can't be parsed an exception must be thrown.
	std::memset((void*)&m_request, 0, sizeof(m_request));
};

/**
 * @brief Parse the raw request
 *
 * Fill the m_request struct with parsed data.
 *
 * @throws exception on error
 */
void Request::parse()
{
	size_t pos = 0;
	// The rfc9112 specify that we SHOULD ignore at least one crlf prior to the request.
	while (m_buffer.substr(pos, 2) == CRLF)
		pos += 2;
	m_request.method = _Request::parse_method(m_buffer, pos);
	_Request::consume_single_whitespace(m_buffer, pos);
	m_request.target = _Request::parse_target(m_buffer, pos);
	_Request::consume_single_whitespace(m_buffer, pos);
	m_request.protocol = _Request::parse_protocol(m_buffer, pos);
	_Request::consume_single_crlf(m_buffer, pos);
}

// TODO: check with a file if the crlf is translated to a single \n
// TODO: real implementation
void Request::read_socket()
{
	m_buffer =  "GET /exemple.com HTTP/1.1\r\n";
}

const request_t& Request::get_request() const
{
	return (m_request);
}

// Helper functions
namespace _Request {

	// TODO: call this function from an init one called from the main
	static const std::map<std::string, method_t> build_method_map()
	{
		std::map<std::string, method_t> methods;
		// Following rfc9112 all the methods must be uppercase
		methods["DELETE"] = del;
		methods["GET"] = get;
		methods["POST"] = post;
		return (methods);
	}

	// TODO: call this function from an init one called from the main
	static const std::map<std::string, protocol_t> build_protocol_map()
	{
		std::map<std::string, protocol_t> protocols;
		protocols["0.9"] = zero_nine;
		protocols["1.0"] = one;
		protocols["1.1"] = one_one;
		protocols["2"] = two;
		protocols["3"] = three;
		return (protocols);
	}
	
	void consume_single_whitespace(const std::string& buffer, size_t& pos)
	{
		if (buffer.at(pos) == ' ')
			++pos;
		else
			throw std::exception();
		// TODO!: determine if we need to skip more than the ' ' char (tab...)
		if (buffer.at(pos) == ' ')
			throw std::exception();
	}

	void consume_single_crlf(const std::string& buffer, size_t& pos)
	{
		if (buffer.substr(pos, 2) == CRLF)
			pos += 2;
		else
			throw std::exception();
	}

	// The method (and the space following it) will be consumed from pos.
	// Throws on failure.
	method_t parse_method(const std::string& buffer, size_t& pos)
	{
		static const std::map<std::string, method_t> methods = build_method_map();
		std::map<std::string, method_t>::const_iterator method = methods.begin();
		std::map<std::string, method_t>::const_iterator end = methods.end();
		for (; method != end; ++method) {
			if (buffer.substr(pos, method->first.length()) == method->first) {
				pos += method->first.length();
				return (method->second);
			}
		}
		throw std::exception();
	}

	// TODO: do we need to handle absolute form (e.g. http://example.com/index.html?q=now)
	// Only handles target as origin form (e.g /index.html)
	// TODO: handle query strings (e.g /main?l=en/index.html?q=now/)
	std::string parse_target(const std::string& buffer, size_t& pos)
	{
		std::string target;
		size_t start = pos;
		// TODO: check for path format validity (but what is valid?)
		while (!std::isspace(buffer.at(pos))) {
			++pos;
		}
		target = buffer.substr(start, pos - start);
		return (target);
	};

	// TODO: do we need to handle protocol case insensitive?
	// Case sensitive protocol parsing (expect uppercase)
	protocol_t parse_protocol(const std::string& buffer, size_t& pos)
	{
		const std::string http_name = "HTTP/";
		if (buffer.substr(pos, http_name.length()) != http_name)
			throw std::exception();
		else
			pos += http_name.length();

		static const std::map<std::string, protocol_t> protocols = build_protocol_map();
		std::map<std::string, protocol_t>::const_iterator protocol = protocols.begin();
		std::map<std::string, protocol_t>::const_iterator end = protocols.end();
		for (; protocol != end; ++protocol) {
			if (buffer.substr(pos, protocol->first.length()) == protocol->first) {
				pos += protocol->first.length();
				return (protocol->second);
			}
		}
		throw std::exception();
	}

}
//
// #include <iostream>
// int main(void)
// {
// 	try {
// 		Request test;
// 		test.read_socket();
// 		test.parse();
// 		std::cout << test.get_request().method << '\n';
// 	}
// 	catch (const std::exception& e) {
// 		// TODO: create a function to automatically print exception messages
// 		std::cerr << e.what() << '\n';
// 	}
//
// 	return (0);
// }
