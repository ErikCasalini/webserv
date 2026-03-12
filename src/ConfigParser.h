#ifndef CONFIG_PARSER_H
# define CONFIG_PARSER_H

# include "Config.h"
# include <string>
# include <list>

# ifndef CONNECTIONS_MAX
#  define CONNECTIONS_MAX 2048
# endif

# ifndef CONNECTIONS_MIN
#  define CONNECTIONS_MIN 2
# endif

# ifndef KEEP_ALIVE_TIMEOUT_MAX
#  define KEEP_ALIVE_TIMEOUT_MAX 86400 // => 24h
# endif

# ifndef ERROR_PAGE_LEN_MAX
#  define ERROR_PAGE_LEN_MAX 1048576 // => 1MO
# endif

// Limit max number of error pages as we load them all in ram.
# ifndef ERROR_PAGE_MAX
#  define ERROR_PAGE_MAX 100
# endif

class ConfigParser {
public:
	ConfigParser(std::string conf_path, std::string executable_path);

	config_t parse();

	class UnexpectedToken : public std::runtime_error {
	public:
		UnexpectedToken(std::string msg);
	};
	class UnexpectedEof : public std::runtime_error {
	public:
		UnexpectedEof();
	};
	class InvalidValue : public std::runtime_error {
	public:
		InvalidValue(std::string msg);
	};
private:
	ConfigParser();
	std::list<std::string> m_tokens;
	std::list<std::string>::iterator m_tok_it;
	int m_depth;
	config_t m_config;
	const std::string m_conf_path;
	const std::string m_exec_path;

	void advance();
	void advance(std::string symbol);
	void consume();
	void consume(std::string symbol);

	unsigned long value_to_ul(
					const std::string& value,
					unsigned long max,
					unsigned long min,
					std::string error_scope);
	bool extract_boolean();
	void skip_block();
	void parse_events();
	void parse_http();
	server_t parse_server();
	server_t inherit_from_http();
	void parse_locations(server_t& server);
	void parse_location(location_t& location);
	location_t inherit_loc_from_server(const server_t& server);
	void parse_exact_match(location_t& location);
	redirection_t parse_redirection();
	bool parse_cgi();
	bool parse_cgi_nph();
	std::string parse_index();
	std::vector<method_t> parse_limit_except();
	std::string parse_default_type();
	void parse_type(std::map<std::string, std::string>& types);
	std::map<std::string, std::string> parse_types();
	unsigned int parse_max_connections();
	bool parse_autoindex();
	std::list<std::string>::iterator get_error_filename_pos();
	void parse_error_page(error_page_t& error_page);
	unsigned int parse_keepalive_timeout();
	listen_t parse_listen();
	unsigned int parse_max_body_size();
	std::string prepare_path(std::string path);
	std::string parse_root();
	u_int32_t ip_to_ui32(std::string ip);
	u_int16_t port_to_ui16(std::string port);
	storage_t parse_storage();
};

#endif
