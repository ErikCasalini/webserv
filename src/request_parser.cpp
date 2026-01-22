#include "http_types.h"
#include "request_parser.h"
#include "../include/cctype_cast.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>
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
	try {
		// The rfc9112 specify that we SHOULD ignore at least one crlf prior to the request.
		while (m_buffer.substr(pos, 2) == CRLF)
			pos += 2;

		// request line
		// TODO: handle simple requests in the form `METHOD TARGET`
		m_request.method = _Request::parse_method(m_buffer, pos);
		_Request::consume_sp(m_buffer, pos);
		m_request.target = _Request::parse_target(m_buffer, pos);
		// Start line for 0.9 don't have the PROTOCOL field
		if (m_buffer.substr(pos, 2) == CRLF) {
			m_request.protocol = zero_nine;
			throw Request::NotImplemented();
		}
		_Request::consume_sp(m_buffer, pos);
		m_request.protocol = _Request::parse_protocol(m_buffer, pos);
		_Request::consume_crlf(m_buffer, pos);

		// headers
		m_request.headers = _Request::parse_headers(m_buffer, pos);
	} catch (const Request::BadRequest& e) {
		m_request.status = bad_request;
	}
	m_request.status = ok;
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

const char* Request::BadRequest::what() const throw()
{
	return ("bad request");
}

const char* Request::NotImplemented::what() const throw()
{
	return ("not implemented");
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
		protocols["1.0"] = one;
		protocols["1.1"] = one_one;
		return (protocols);
	}
	
	void consume_sp(const std::string& buffer, size_t& pos)
	{
		if (buffer.at(pos) == ' ')
			++pos;
		else
			throw Request::BadRequest();
		if (buffer.at(pos) == ' ')
			throw Request::BadRequest();
	}

	void consume_crlf(const std::string& buffer, size_t& pos)
	{
		if (buffer.substr(pos, 2) == CRLF)
			pos += 2;
		else
			throw Request::BadRequest();
	}

	void consume_ows_cr(const std::string& buffer, size_t& pos)
	{
		while (buffer.at(pos) == ' '
				|| buffer.at(pos) == '\t'
				|| (buffer.at(pos) == '\r' && buffer.substr(pos, 2) != CRLF))
			++pos;
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
		// TODO: replace this exception by NotImplemented when the method is uppercase but not found in the map
		throw Request::BadRequest();
	}

	// TODO: do we need to handle absolute form (e.g. http://example.com/index.html?q=now)
	// Only handles target as origin form (e.g /index.html)
	// TODO: handle query strings (e.g /main?l=en/index.html?q=now/)
	std::string parse_target(const std::string& buffer, size_t& pos)
	{
		std::string target;
		size_t start = pos;
		// TODO: check for path format validity (but what is valid?)
		try {
			while (!is_space(buffer.at(pos))) {
				++pos;
			}
		} catch (const std::out_of_range& e) {
			throw Request::BadRequest();
		}
		if (pos == start)
			throw Request::BadRequest();
		target = buffer.substr(start, pos - start);
		return (target);
	};

	// Case sensitive protocol parsing (expect uppercase)
	protocol_t parse_protocol(const std::string& buffer, size_t& pos)
	{
		// Following rfc9112 the protocol name is case sensitive
		const std::string http_name = "HTTP/";
		if (buffer.substr(pos, http_name.length()) != http_name)
			throw Request::BadRequest();
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
		throw Request::BadRequest();
	}

	std::string extract_key(const std::string& buffer, size_t& pos)
	{
		std::string key;
		size_t start = pos;
		// Catch out-of-range exceptions if at end of string
		try {
			while (is_graph(buffer.at(pos))) {
				if (buffer.at(pos) == ':') {
					if (pos == start)
						throw Request::BadRequest();
					std::string key = buffer.substr(start, pos);
					std::transform(key.begin(), key.end(), key.begin(), to_lowercase);
					++pos;
					return (key);
				}
				else if (is_space(buffer.at(pos)))
					throw Request::BadRequest();
				++pos;
			}
		} catch (const std::out_of_range& e) {}
		throw Request::BadRequest();
	}

	// TODO: handle quoted strings: "asdsda" or quoted pairs: \n
	std::vector<std::string> extract_values(const std::string& buffer, size_t& pos)
	{
		std::vector<std::string> values;
		try {
			size_t start = pos;
			while (buffer.substr(pos, 2) != CRLF)
				++pos;
			// Push the full string without any checks,
			// actual parsing is done after only if the key is recognized.
			values.push_back(buffer.substr(start, pos));
			pos += 2;
			return (values);
		} catch (const std::out_of_range& e) {
			throw Request::BadRequest();
		}
	}

	headers_t parse_headers(const std::string& buffer, size_t& pos)
	{
		headers_t headers;

		while (buffer.substr(pos, 2) != CRLF) {
			std::string key = extract_key(buffer, pos);
			headers[key] = extract_values(buffer, pos);
		}
		// TODO: actually parse the headers (content-length...)
		return (headers);
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
