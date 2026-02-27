#ifndef CONFIG_H
# define CONFIG_H

# include "http_types.h"
# include <sys/types.h>
# include <map>
# include <string>
# include <list>
# include <vector>

# define DEFAULT_MAX_CONNECTIONS 512

typedef std::pair<status_t, std::string> redirection_t;
typedef std::map<int, std::string> error_page_t;

struct listen_t {
	listen_t();
	u_int32_t ip;
	u_int16_t port;
};

struct location_t {
	location_t();
	bool autoindex;
	bool cgi;
	error_page_t error_page;
	bool exact_match;
	std::string index;
	std::vector<method_t> limit_except;
	std::list<std::string> path;
	redirection_t redirection;
	std::string root;
};

struct server_t {
	server_t();
	bool autoindex;
	error_page_t error_page;
	// unsigned int keepalive_timeout; // put it here too?
	std::vector<listen_t> listen;
	std::vector<location_t> locations;
	unsigned int max_body_size;
	redirection_t redirection;
	std::string root;
	std::pair<std::list<std::string>, std::string> upload; // check if not in cgi
};

struct http_t {
	http_t();
	bool autoindex;
	error_page_t error_page;
	unsigned int keepalive_timeout;
	unsigned int max_body_size;
	std::string root;
	std::vector<server_t> server;
	std::string default_type;
	std::map<std::string, std::string> types;
};

struct events_t {
	events_t();
	unsigned int max_connections;
};

struct config_t {
	events_t events;
	http_t http;
};

#endif
