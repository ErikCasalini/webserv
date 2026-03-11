#include "Headers.h"
#include "../include/cctype_cast.h"
#include <algorithm>
#include <stdlib.h>
#include <cerrno>
#include <string>

Headers::BadHeader::BadHeader(const std::string& msg)
	: std::runtime_error(("Headers: " + msg).c_str()) {};

Headers::Headers() {}

Headers::Headers(const headers_map_t& headers_map)
	: m_map(headers_map)
{}

Headers::Headers(const std::string& headers_str, const std::string& nl)
	: m_map(string_to_map(headers_str, nl))
{}

Headers::Headers(const Headers& src)
	: m_map(src.m_map)
	, m_struct(src.m_struct)
{}

Headers& Headers::operator=(const Headers& src)
{
	if (this != &src) {
		m_map = src.m_map;
		m_struct = src.m_struct;
	}
	return (*this);
}

headers_map_t::size_type Headers::count(const std::string& key)
{
	return (m_map.count(key));
}

const std::string& Headers::at(const std::string& key)
{
	return (m_map.at(key));
}

headers_map_t Headers::string_to_map(const std::string& headers_str,
										const std::string& nl)
{
	size_t pos = 0;
	headers_map_t map;
	try {
		while (headers_str.substr(pos, nl.size()) != nl && pos < headers_str.length()) {
			std::string key = extract_key(headers_str, pos);
			map[key] = extract_value(headers_str, pos, nl);
		}
	} catch (const std::out_of_range& e) {
		throw Headers::BadHeader("unexpected EOL in headers str");
	}
	return (map);
}

std::string Headers::key_to_string(const headers_map_t::const_iterator& key,
									const std::string& nl)
{
	std::string str;
	str.append(key->first);
	str.append(": ");
	str.append(key->second);
	str.append(nl);
	return (str);
}

std::string Headers::map_to_string(const headers_map_t& headers_map,
									const std::string& nl)
{
	std::string str;
	headers_map_t::const_iterator end = headers_map.end();
	headers_map_t::const_iterator it = headers_map.begin();
	for ( ; it != end; ++it) {
		key_to_string(it, nl);
	}
	return (str);
}

std::string Headers::extract_key(const std::string& buffer, size_t& pos)
{
	std::string key;
	size_t start = pos;
	try {
		while (is_graph(buffer.at(pos))) {
			if (buffer.at(pos) == ':') {
				if (pos == start)
					throw Headers::BadHeader("missing header name");
				std::string key = buffer.substr(start, pos - start);
				std::transform(key.begin(), key.end(), key.begin(), to_lower);
				++pos;
				return (key);
			}
			else if (is_space(buffer.at(pos)))
				throw Headers::BadHeader("space between name and colon");
			++pos;
		}
	} catch (const std::out_of_range& e) {
		throw Headers::BadHeader("unexpected EOL in header key");
	}
	throw Headers::BadHeader("missing semicolon after key");
}

std::string Headers::extract_value(const std::string& buffer,
									size_t& pos,
									const std::string& nl)
{
	std::string val;
	try {
		size_t start = pos;
		while (buffer.substr(pos, nl.size()) != nl)
			++pos;
		// Push the full string without any checks,
		// actual parsing is done after only if the key is recognized.
		while (buffer.at(start) == ' ') {
			++start;
			if (start >= buffer.size())
				throw Headers::BadHeader("empty value");
		}
		val = buffer.substr(start, pos - start);
		pos += 2;
		return (val);
	} catch (const std::out_of_range& e) {
		throw Headers::BadHeader("missing CRLF after header line");
	}
}

long Headers::parse_content_length(unsigned long max)
{
	try {
		std::string val = m_map.at("content-length");
		if (val.length() > 0) {
			strtrim(val);
			if (val.at(0) == '-')
				throw Headers::BadHeader("content-length is negative");
			char* end = NULL;
			errno = 0;
			unsigned long len = std::strtoul(val.c_str(), &end, 10);
			// is it a specific status code?
			if (*end || errno || len > max) {
				errno = 0;
				throw Headers::BadHeader("invalid content-length");
			}
			return (static_cast<long>(len));
		}
	} catch (const std::out_of_range& e) {}
	return (-1);
}

std::string Headers::parse_content_type()
{
	try {
		std::string val = m_map.at("content-type");
		strtrim(val);
		std::transform(val.begin(), val.end(), val.begin(), to_lower);
		return (val);
	} catch (const std::out_of_range& e) {}
	return ("application/octet-stream");
}

bool Headers::parse_connection()
{
	try {
		std::string val = m_map.at("connection");
		strtrim(val);
		std::transform(val.begin(), val.end(), val.begin(), to_lower);
		if (val.length() > 0 && val == "keep-alive")
			return (true);
	} catch (const std::out_of_range& e) {}
	return (false);
}

headers_t Headers::get_struct()
{
	return (m_struct);
}

headers_map_t Headers::get_map()
{
	return (m_map);
}

void Headers::set_map(const headers_map_t& headers_map)
{
	m_map = headers_map;
}
