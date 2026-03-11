#ifndef HEADERS_H
# define HEADERS_H

# include "http_types.h"
# include <limits>
# include <map>
# include <string>

typedef std::map<std::string, std::string> headers_map_t;

class Headers {
public:
	class BadHeader : public std::runtime_error {
	public:
		BadHeader(const std::string& msg);
	};

	Headers();
	Headers(const headers_map_t& headers_map);
	Headers(const std::string& headers_str, const std::string& nl);
	Headers(const Headers& src);
	Headers& operator=(const Headers& src);

	static headers_map_t string_to_map(const std::string& headers_str,
										const std::string& nl);
	static std::string key_to_string(const headers_map_t::const_iterator& key,
										const std::string& nl);
	static std::string map_to_string(const headers_map_t& headers_map,
										const std::string& nl = CRLF);

	static std::string extract_key(const std::string& buffer, size_t& pos);
	static std::string extract_value(const std::string& buffer,
										size_t& pos,
										const std::string& nl);

	long parse_content_length(unsigned long max
			= static_cast<unsigned long>(std::numeric_limits<long>::max()));
	std::string parse_content_type();
	bool parse_connection();
	headers_map_t::size_type count(const std::string& key);
	const std::string& at(const std::string& key);

	headers_t get_struct();
	headers_map_t get_map();
	void set_map(const headers_map_t& raw_headers);
	// void set_str(const std::string& headers_str);

private:
	headers_map_t m_map;
	headers_t m_struct;
};

#endif
