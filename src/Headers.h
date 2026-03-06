#ifndef HEADERS_H
# define HEADERS_H

# include "http_types.h"
# include <map>
# include <string>

typedef std::map<std::string, std::string> headers_map_t;

class Headers {
public:
	Headers();
	Headers(headers_map_t headers);
	Headers(const Headers& src);
	Headers& operator=(const Headers& src);

	static std::string headers_map_to_string(const headers_map_t& headers);

	headers_t get_headers();
	headers_map_t get_raw_headers();
	void set_raw_headers(const headers_map_t& raw_headers);

	std::string extract_key(const std::string& buffer, size_t& pos);
	std::string extract_values(const std::string& buffer, size_t& pos);

	long parse_content_length(const headers_map_t& headers);
	long parse_content_type(const headers_map_t& headers);
	bool parse_connection(const headers_map_t& headers);
private:
	std::map<std::string, std::string> m_raw_headers;
	headers_t m_headers;
};

#endif
