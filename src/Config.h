#ifndef CONFIG_H
# define CONFIG_H

# include "http_types.h"
# include <sys/types.h>
# include <map>
# include <string>
# include <list>
# include <vector>

# ifndef DEFAULT_MAX_CONNECTIONS
#  define DEFAULT_MAX_CONNECTIONS 512
# endif

# ifndef DEFAULT_IP
#  define DEFAULT_IP 0x7F000001 // 127.0.0.1
# endif

# ifndef DEFAULT_PORT
#  define DEFAULT_PORT 8080
# endif

# ifndef VERSION
#  define VERSION "webserv/2026"
# endif

# ifndef DEFAULT_ERROR_400
#  define DEFAULT_ERROR_400 "<html>"\
"<head><title>400 Bad Request</title></head>"\
"<body>"\
"<center><h1>400 Bad Request</h1></center>"\
"<hr><center>" VERSION "</center>"\
"</body>"\
"</html>"
# endif

# ifndef DEFAULT_ERROR_401
#  define DEFAULT_ERROR_401 "<html>"\
"<head><title>401 Unauthorized</title></head>"\
"<body>"\
"<center><h1>401 Unauthorized</h1></center>"\
"<hr><center>" VERSION "</center>"\
"</body>"\
"</html>"
# endif

# ifndef DEFAULT_ERROR_402
#  define DEFAULT_ERROR_402 "<html>"\
"<head><title>402 Payment Required</title></head>"\
"<body>"\
"<center><h1>402 Payment Required</h1></center>"\
"<hr><center>" VERSION "</center>"\
"</body>"\
"</html>"
# endif

# ifndef DEFAULT_ERROR_403
#  define DEFAULT_ERROR_403 "<html>"\
"<head><title>403 Forbidden</title></head>"\
"<body>"\
"<center><h1>403 Forbidden</h1></center>"\
"<hr><center>" VERSION "</center>"\
"</body>"\
"</html>"
# endif

# ifndef DEFAULT_ERROR_404
#  define DEFAULT_ERROR_404 "<html>"\
"<head><title>404 Not Found</title></head>"\
"<body>"\
"<center><h1>404 Not Found</h1></center>"\
"<hr><center>" VERSION "</center>"\
"</body>"\
"</html>"
# endif

# ifndef DEFAULT_ERROR_500
#  define DEFAULT_ERROR_500 "<html>"\
"<head><title>500 Internal Server Error</title></head>"\
"<body>"\
"<center><h1>500 Internal Server Error</h1></center>"\
"<hr><center>" VERSION "</center>"\
"</body>"\
"</html>"
# endif

# ifndef DEFAULT_ERROR_501
#  define DEFAULT_ERROR_501 "<html>"\
"<head><title>501 Not Implemented</title></head>"\
"<body>"\
"<center><h1>501 Not Implemented</h1></center>"\
"<hr><center>" VERSION "</center>"\
"</body>"\
"</html>"
# endif

# ifndef DEFAULT_ERROR_502
#  define DEFAULT_ERROR_502 "<html>"\
"<head><title>502 Bad Gateway</title></head>"\
"<body>"\
"<center><h1>502 Bad Gateway</h1></center>"\
"<hr><center>" VERSION "</center>"\
"</body>"\
"</html>"
# endif


typedef std::pair<status_t, std::string> redirection_t;
typedef std::map<int, std::string> error_page_t;
typedef std::pair<std::list<std::string>, std::string> storage_t; // url, real_path

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
	std::vector<listen_t> listen;
	std::vector<location_t> locations;
	unsigned int max_body_size;
	redirection_t redirection;
	std::string root;
	storage_t storage; // url, real_path
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
