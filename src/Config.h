#ifndef CONFIG_H
# define CONFIG_H

# include <map>
# include <string>
# include <vector>

# define DEFAULT_WORKER_CONNECTIONS 512

using std::string;
using std::map;
using std::vector;

struct events_t {
	unsigned int worker_connections = DEFAULT_WORKER_CONNECTIONS;
};

struct listen_t {
	uint32_t ip;
	uint16_t port;
};

struct location_t {
	string root;
	string index;
};

struct error_page_t {
	vector<int> status;
	string target;
};

struct autoindex_t {
	bool on;
};

struct limit_except_t {
	vector<string> methods;
};

struct server_t {
	listen_t listen;
	location_t location;
	error_page_t error_page;
	autoindex_t autoindex;
	limit_except_t limit_except;
};

struct http_t {
	map<string, string> types;
	string default_type;
	server_t server;
	autoindex_t autoindex;
	limit_except_t limit_except;
	unsigned int keepalive_timeout;
};

struct config_t {
	events_t events;
	http_t http;
};

// very long access
// config.http.server.listen.ip;
// config.http.server.listen.port;

#endif
