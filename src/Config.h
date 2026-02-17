#ifndef CONFIG_H
# define CONFIG_H

# include <sys/types.h>
# include <map>
# include <string>
# include <list>
# include <vector>

# define DEFAULT_MAX_CONNECTIONS 512

typedef std::multimap<int, std::string> redirection_t;

typedef std::map<int, std::string> error_page_t;

struct listen_t {
	u_int32_t ip;
	u_int16_t port;
};

struct limit_except_t {
	std::vector<std::string> methods;
};

struct location_t {

	location_t(void)
	: exact_match(false),
	  autoindex(false) {};

	std::list<std::string> path;
	bool exact_match;
	std::string root;
	limit_except_t limit_except;
	bool autoindex;
	error_page_t error_page;
	std::string index;
	redirection_t redirection;
};

struct server_t {
	std::vector<listen_t> listen;
	std::vector<location_t> locations;
	std::string root;
	error_page_t error_page;
	bool autoindex;
	redirection_t redirection;
};

struct http_t {
	// map<std::string, std::string> types;
	// std::string default_type;
	std::vector<server_t> server;
	std::string root;
	bool autoindex;
	error_page_t error_page;
	unsigned int max_body;
	unsigned int keepalive_timeout;
};

struct events_t {
	unsigned int max_connections;
};

struct config_t {
	events_t events;
	http_t http;
};

#endif
