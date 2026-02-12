#include "http_types.h"
#include "request_parser.h"
#include "../include/cctype_cast.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>
#include <errno.h>
#include <sys/socket.h>

//HEADERS
// allow
// "authorization",
// // can be ignored safely
// "content-encoding",
// // IMPLEMENT POST
// "content-length",
// // can be ignored safely server side
// "content-type",
// // can be ignored
// "date"
// // can be ignored without cache
// "expires"
// // can be ignored
// "from"
// // can be ignored
// "if-modified-since"
// // can be ignored
// "last-modified",
// // can be ignored
// "location"
// // can be ignored
// "pragma"
// // can be ignored
// "referer"
// //can be ignored
// "user-agent"

// RESPONSE
// should be ignored
// server
// WWW-Authenticate
using std::string;
using std::map;

Request::Request()
	: m_sockfd(-1)
	, m_recv_buf_size(50000)
	, m_pos(0)
	, m_state(RequestStates::Init::get_instance())
{}

// For testing purposes
Request::Request(string buffer)
	: m_sockfd(-1)
	, m_buffer(buffer)
	, m_recv_buf_size(50000)
	, m_pos(0)
	, m_state(RequestStates::Init::get_instance())
{}
 
Request::Request(const Request& src)
	: m_sockfd(src.m_sockfd)
	, m_infos(src.m_infos)
	, m_buffer(src.m_buffer)
	, m_recv_buf_size(src.m_recv_buf_size)
	, m_pos(src.m_pos)
	, m_state(src.m_state)
{
}

Request& Request::operator=(const Request& src)
{
	if (&src != this) {
		m_sockfd = src.m_sockfd;
		m_infos = src.m_infos;
		m_buffer = src.m_buffer;
		m_recv_buf_size = src.m_recv_buf_size;
		m_pos = src.m_pos;
		m_state = src.m_state;
	}
	return (*this);
}

void Request::clear()
{
	m_state->clear(this);
}

void Request::_clear()
{
	m_sockfd = -1;
	m_infos.clear();
	m_buffer.clear();
	m_pos = 0;
	m_state = RequestStates::Init::get_instance();
}

void Request::clear_infos()
{
	m_state->clear_infos(this);
}

void Request::_clear_infos()
{
	m_infos.clear();
}

const request_t& Request::get_infos() const
{
	return (m_infos);
}

/**
 * @brief Parse the raw request
 *
 * Fill the m_request struct with parsed data.
 * 
 * @throws exception on error
 * @throws ConnectionClosed exception on closed connection (recv() == 0)
 */
void Request::parse()
{
	m_state->parse(this);
}

void Request::set_state(RequestState* state)
{
	m_state = state;
}

void Request::erase_parsed()
{
	m_buffer.erase(0, m_pos);
	m_pos = 0;
}

void Request::_append_buffer(const char* str)
{
	m_buffer.append(str);
}

Request::BadRequest::BadRequest(const char* msg) : std::runtime_error(msg) {};

Request::NotImplemented::NotImplemented(const char* msg) : std::runtime_error(msg) {};

Request::ConnectionClosed::ConnectionClosed(const char* msg) : std::runtime_error(msg) {};

namespace _Request {
	ssize_t read_socket(int sockfd, string& buffer, size_t read_size)
	{
		char buf[read_size];
		ssize_t ret = 0;
		ret = recv(sockfd, &buf, read_size, MSG_DONTWAIT);
		if (ret > 0)
			buffer.append(buf, static_cast<string::size_type>(ret));
		return (ret);
	}

	// TODO: call this function from an init one called from the main
	static const map<string, method_t> build_method_map()
	{
		map<string, method_t> methods;
		// Following rfc9112 all the methods must be uppercase
		methods["DELETE"] = del;
		methods["GET"] = get;
		methods["POST"] = post;
		return (methods);
	}

	// TODO: call this function from an init one called from the main
	static const map<string, protocol_t> build_protocol_map()
	{
		map<string, protocol_t> protocols;
		protocols["1.0"] = one;
		protocols["1.1"] = one_one;
		return (protocols);
	}

	void consume_sp(const string& buffer, size_t& pos)
	{
		try {
			if (buffer.at(pos) == ' ')
				++pos;
			else
				throw Request::BadRequest("missing SP");
			if (buffer.at(pos) == ' ')
				throw Request::BadRequest("extraneous SP");
		} catch (const std::out_of_range& e) {
			throw Request::BadRequest("unexpected EOL in place of sp");
		}


	}

	void consume_crlf(const string& buffer, size_t& pos)
	{
		try {
			if (buffer.substr(pos, 2) == CRLF)
				pos += 2;
			else
				throw Request::BadRequest("missing CRLF");
		} catch (const std::out_of_range& e) {
			throw Request::BadRequest("unexpected EOL in place of crlf");
		}
	}

	void consume_ows_cr(const string& buffer, size_t& pos)
	{
		try {
			while (buffer.at(pos) == ' '
					|| buffer.at(pos) == '\t'
					|| (buffer.at(pos) == '\r' && buffer.substr(pos, 2) != CRLF))
				++pos;
		} catch (const std::out_of_range& e) {
			throw Request::BadRequest("unexpected EOL in place of ows/cr");
		}
	}

	// The method (and the space following it) will be consumed from pos.
	// Throws on failure.
	method_t parse_method(const string& buffer, size_t& pos)
	{
		static const map<string, method_t> methods = build_method_map();
		map<string, method_t>::const_iterator method = methods.begin();
		map<string, method_t>::const_iterator end = methods.end();
		try {
			for (; method != end; ++method) {
				if (buffer.substr(pos, method->first.length()) == method->first) {
					pos += method->first.length();
					return (method->second);
				}
			}
		} catch (const std::out_of_range& e) {
			throw Request::BadRequest("unexpected EOL in method");
		}
		// TODO: replace this exception by NotImplemented when the method is uppercase but not found in the map
		throw Request::BadRequest("illformed method");
	}

	// TODO: do we need to handle absolute form (e.g. http://example.com/index.html?q=now)
	// Only handles target as origin form (e.g /index.html)
	string parse_target(const string& buffer, size_t& pos)
	{
		string target;
		size_t start = pos;
		try {
			while (!is_space(buffer.at(pos))) {
				++pos;
			}
			if (pos == start)
				throw Request::BadRequest("missing target");
			target = buffer.substr(start, pos - start);
		} catch (const std::out_of_range& e) {
			throw Request::BadRequest("non terminated target");
		}
		return (target);
	};

	// Case sensitive protocol parsing (expect uppercase)
	protocol_t parse_protocol(const string& buffer, size_t& pos)
	{
		// Following rfc9112 the protocol name is case sensitive
		const string http_name = "HTTP/";
		try {
			if (buffer.substr(pos, http_name.length()) != http_name)
				throw Request::BadRequest("invalid protocol name");
			else
				pos += http_name.length();

			static const map<string, protocol_t> protocols = build_protocol_map();
			map<string, protocol_t>::const_iterator protocol = protocols.begin();
			map<string, protocol_t>::const_iterator end = protocols.end();
			for (; protocol != end; ++protocol) {
				if (buffer.substr(pos, protocol->first.length()) == protocol->first) {
					pos += protocol->first.length();
					return (protocol->second);
				}
			}
		} catch (const std::out_of_range& e) {
			throw Request::BadRequest("unexpected EOL in protocol");
		}
		throw Request::BadRequest("unknown protocol version");
	}

	string extract_key(const string& buffer, size_t& pos)
	{
		string key;
		size_t start = pos;
		// Catch out-of-range exceptions if at end of string
		try {
			while (is_graph(buffer.at(pos))) {
				if (buffer.at(pos) == ':') {
					if (pos == start)
						throw Request::BadRequest("missing header name");
					string key = buffer.substr(start, pos - start);
					std::transform(key.begin(), key.end(), key.begin(), to_lower);
					++pos;
					return (key);
				}
				else if (is_space(buffer.at(pos)))
					throw Request::BadRequest("space between name and colon");
				++pos;
			}
		} catch (const std::out_of_range& e) {
			throw Request::BadRequest("unexpected EOL in header key");
		}
		throw Request::BadRequest("invalid header name");
	}

	// TODO: handle quoted strings: "asdsda" or quoted pairs: \n
	string extract_values(const string& buffer, size_t& pos)
	{
		string val;
		try {
			size_t start = pos;
			while (buffer.substr(pos, 2) != CRLF)
				++pos;
			// Push the full string without any checks,
			// actual parsing is done after only if the key is recognized.
			val = buffer.substr(start, pos - start);
			pos += 2;
			return (val);
		} catch (const std::out_of_range& e) {
			throw Request::BadRequest("missing CRLF after header line");
		}
	}

	raw_headers_t extract_headers(const string& buffer, size_t& pos)
	{
		raw_headers_t headers;

		try {
			while (buffer.substr(pos, 2) != CRLF && pos < buffer.length()) {
				string key = extract_key(buffer, pos);
				headers[key] = extract_values(buffer, pos);
			}
		} catch (const std::out_of_range& e) {
			throw Request::BadRequest("unexpected EOL in headers");
		}
		return (headers);
	}

	unsigned long parse_content_length(const raw_headers_t& raw_headers)
	{
		try {
			string val = raw_headers.at("content-length");
			if (val.length() > 0) {
				strtrim(val);
				if (val.at(0) == '-')
					throw Request::BadRequest("content-length is negative");
				char* end = NULL;
				errno = 0;
				unsigned long len = std::strtoul(val.c_str(), &end, 10);
				// TODO: check for max specified length (in config file or default value)
				// is it a specific status code?
				if (*end || errno) {
					errno = 0;
					throw Request::BadRequest("invalid content-length");
				}
				return (len);
			}
		} catch (const std::out_of_range& e) {}
		throw Request::BadRequest("missing content-length value");
	}

	bool parse_connection(const raw_headers_t& raw_headers)
	{
		try {
			string val = raw_headers.at("connection");
			strtrim(val);
			std::transform(val.begin(), val.end(), val.begin(), to_lower);
			if (val.length() > 0 && val == "keep-alive")
				return (true);
		} catch (const std::out_of_range& e) {}
		return (false);
	}

	headers_t parse_headers(
				const string& buffer,
				size_t& pos,
				const request_t& request)
	{
		raw_headers_t raw_headers = extract_headers(buffer, pos);
		headers_t headers;
		headers.keep_alive = parse_connection(raw_headers);
		// headers.cookies = parse_cookies(raw_headers);
		// if (request.method == get)
		// 	headers.if_modified_since = parse_if_modified_since(raw_headers);
		if (request.method == post)
			headers.content_length = parse_content_length(raw_headers);
		return (headers);
	}

	string extract_body(const string& buffer, size_t& pos, request_t& request)
	{
		static size_t len;
		if (request.body.length() == 0)
			len = request.headers.content_length;
		string body;
		try {
			if (request.headers.content_length > 0) {
				// Check if the full body is in the recv buffer
				// if it isn't the status must stay in "parsing".
				if (buffer.length() - pos >= len) {
					request.status = ok;
					body = buffer.substr(pos, len);
					pos += len;
				} else {
					body = buffer.substr(pos);
					len -= buffer.length() - pos;
					pos += buffer.length() - pos;
				}
			}
		} catch (const std::out_of_range& e) {
			request.status = bad_request;
		}
		return (body);
	}
}
