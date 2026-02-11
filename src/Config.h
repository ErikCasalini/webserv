#ifndef CONFIG_H
# define CONFIG_H

# include <map>
# include <string>
# include <vector>

# define DEFAULT_WORKER_CONNECTIONS 512

using std::string;
using std::map;
using std::vector;

struct main_t {
	events_t events;
	http_t http;
};

struct events_t {
	unsigned int worker_connections = DEFAULT_WORKER_CONNECTIONS;
};

struct http_t {
	// map<string, string> types;
	// string default_type;
	vector<server_t> server;
	autoindex_t autoindex;
	error_page_t error_page;
	unsigned int max_body;
	unsigned int keepalive_timeout;
};

struct server_t {
	vector<listen_t> listen;
	location_t location;
	error_page_t error_page;
	autoindex_t autoindex;
	redirection_t redirection;
};

struct location_params_t {
	limit_except_t limit_except_t;
	autoindex_t autoindex;
	error_page_t error_page;
	string index;
	redirection_t redirection;
};

struct listen_t {
	uint32_t ip;
	uint16_t port;
};

struct autoindex_t {
	bool on;
};

struct limit_except_t {
	vector<string> methods;
};

typedef std::multimap<int, string> redirection_t;
typedef std::multimap<string, location_params_t> location_t;
typedef std::multimap<int, string> error_page_t;

#endif
