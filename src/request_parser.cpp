#include "http_types.h"
#include "request_parser.h"
#include <cstring>
#include <string>

Request::Request()
{
	std::memset((void*)&m_request, 0, sizeof(m_request));
};


namespace _Request {

	// TODO: call this function from an init one called from the main
	static const std::map<std::string, method_t> build_method_map()
	{
		std::map<std::string, method_t> methods;
		// All the methods must be uppercase
		methods["DELETE"] = del;
		methods["GET"] = get;
		methods["POST"] = post;
		return (methods);
	}
	
	static void consume_single_whitespace(const std::string& buffer, size_t& pos)
	{
		if (buffer.at(pos) == ' ')
			++pos;
		else
			throw std::exception();
		if (buffer.at(pos) == ' ')
			throw std::exception();
	}

	// On success the method (and the space following it) will be consumed
	// from the iterator. Throws on failure.
	method_t parse_method(const std::string& buffer, size_t& pos)
	{
		static const std::map<std::string, method_t> methods = build_method_map();
		std::map<std::string, method_t>::const_iterator method = methods.begin();
		std::map<std::string, method_t>::const_iterator method_end = methods.end();
		for (; method != method_end; ++method) {
			if (buffer.substr(pos, method->first.length()) == method->first) {
				pos += method->first.length();
				consume_single_whitespace(buffer, pos);
				return (method->second);
			}
		}
		throw std::exception();
	}

}

/**
 * @brief Parse the raw request
 *
 * Fill the m_request struct with parsed data.
 */
void Request::parse()
{
	size_t pos = 0;
	m_request.method = _Request::parse_method(m_buffer, pos);
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
