#include <map>
#include <string>
#include <iostream>
#include "main_loop.hpp"
#include "EpollEvents.hpp"
#include "Config.h"
#include "Response.hpp"

int	main(void)
{
	// Hardcode to simulate config file for interfaces
	config_t			config;
	server_t			server1, server2;
	listen_t			serv1_listen1, serv1_listen2, serv2_listen1, serv2_listen2;
	location_t			serv1_loc1, serv1_loc2, serv2_loc1, serv2_loc2;

	serv1_listen1.ip = 0x7F000001; // 127.0.0.1 big endian endian ???
	serv1_listen1.port = 4242;
	serv1_listen2.ip = 0x7F000001;
	serv1_listen2.port = 4343;

	serv2_listen1.ip = 0x7F000001;
	serv2_listen1.port = 5252;
	serv2_listen2.ip = 0x7F000001;
	serv2_listen2.port = 5353;

	server1.listen.push_back(serv1_listen1); // 4242
	server1.listen.push_back(serv1_listen2); // 4343
	server2.listen.push_back(serv2_listen1); // 5252
	server2.listen.push_back(serv2_listen2); // 5353

	serv1_loc1.autoindex = false;
	serv1_loc1.cgi = false;
	serv1_loc1.exact_match = false;
	serv1_loc1.index = "index.html";
	serv1_loc1.limit_except.push_back(get);
	serv1_loc1.path = _Response::split_path("/src/"); // path doit toujours commencer et finir par '/'
	// serv1_loc1.root = "/home/ecasalin/Documents/serv_files/4242/";
	serv1_loc1.root = "/home/erik/WEBSERV_FILES/";
	serv1_loc1.error_page.insert(std::make_pair(400, std::string("<html>\n<head><title>400 Bad Request</title></head>\n<body>\n<center><h1>400 Bad Request</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	serv1_loc1.error_page.insert(std::make_pair(403, std::string("<html>\n<head><title>403 Forbidden</title></head>\n<body>\n<center><h1>403 Forbidden</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	serv1_loc1.error_page.insert(std::make_pair(404, std::string("<html>\n<head><title>404 Not Found</title></head>\n<body>\n<center><h1>404 Not Found</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	serv1_loc1.error_page.insert(std::make_pair(405, std::string("<html>\n<head><title>405 Method Not Allowed</title></head>\n<body>\n<center><h1>405 Method Not Allowed</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	serv1_loc1.error_page.insert(std::make_pair(500, std::string("<html>\n<head><title>500 Internal Error</title></head>\n<body>\n<center><h1>500 Internal Error</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	serv1_loc1.error_page.insert(std::make_pair(501, std::string("<html>\n<head><title>501 Not Implemented</title></head>\n<body>\n<center><h1>501 Not Implemented</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	serv1_loc1.error_page.insert(std::make_pair(502, std::string("<html>\n<head><title>502 Bad Gateway</title></head>\n<body>\n<center><h1>502 Bad Gateway</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));

	serv1_loc2.autoindex = false;
	serv1_loc2.cgi = false;
	serv1_loc2.exact_match = false;
	serv1_loc2.index = "index.html";
	serv1_loc2.limit_except.push_back(get);
	serv1_loc2.path = _Response::split_path("/");
	// serv1_loc2.root = "/home/ecasalin/Documents/serv_files/4242/";
	serv1_loc2.root = "/home/erik/WEBSERV_FILES/";
	serv1_loc2.error_page.insert(std::make_pair(400, std::string("<html>\n<head><title>400 Bad Request</title></head>\n<body>\n<center><h1>400 Bad Request</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	serv1_loc2.error_page.insert(std::make_pair(403, std::string("<html>\n<head><title>403 Forbidden</title></head>\n<body>\n<center><h1>403 Forbidden</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	serv1_loc2.error_page.insert(std::make_pair(404, std::string("<html>\n<head><title>404 Not Found</title></head>\n<body>\n<center><h1>404 Not Found</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	serv1_loc2.error_page.insert(std::make_pair(405, std::string("<html>\n<head><title>405 Method Not Allowed</title></head>\n<body>\n<center><h1>405 Method Not Allowed</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	serv1_loc2.error_page.insert(std::make_pair(500, std::string("<html>\n<head><title>500 Internal Error</title></head>\n<body>\n<center><h1>500 Internal Error</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	serv1_loc2.error_page.insert(std::make_pair(501, std::string("<html>\n<head><title>501 Not Implemented</title></head>\n<body>\n<center><h1>501 Not Implemented</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	serv1_loc2.error_page.insert(std::make_pair(502, std::string("<html>\n<head><title>502 Bad Gateway</title></head>\n<body>\n<center><h1>502 Bad Gateway</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));

	server1.locations.push_back(serv1_loc1);
	server1.locations.push_back(serv1_loc2);
	server1.error_page.insert(std::make_pair(400, std::string("<html>\n<head><title>400 Bad Request</title></head>\n<body>\n<center><h1>400 Bad Request</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	server1.error_page.insert(std::make_pair(403, std::string("<html>\n<head><title>403 Forbidden</title></head>\n<body>\n<center><h1>403 Forbidden</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	server1.error_page.insert(std::make_pair(404, std::string("<html>\n<head><title>404 Not Found</title></head>\n<body>\n<center><h1>404 Not Found</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	server1.error_page.insert(std::make_pair(405, std::string("<html>\n<head><title>405 Method Not Allowed</title></head>\n<body>\n<center><h1>405 Method Not Allowed</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	server1.error_page.insert(std::make_pair(500, std::string("<html>\n<head><title>500 Internal Error</title></head>\n<body>\n<center><h1>500 Internal Error</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	server1.error_page.insert(std::make_pair(501, std::string("<html>\n<head><title>501 Not Implemented</title></head>\n<body>\n<center><h1>501 Not Implemented</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));
	server1.error_page.insert(std::make_pair(502, std::string("<html>\n<head><title>502 Bad Gateway</title></head>\n<body>\n<center><h1>502 Bad Gateway</h1></center>\n<hr><center>webserv/0.0.0</center>\n</body>\n</html>")));

	config.http.server.push_back(server1);
	config.http.server.push_back(server2);
	config.events.max_connections = 512;

	try {
		main_server_loop(config);
	}
	catch (std::exception &e) {
		std::cerr << e.what();
		return (1);
	}
	return (0);
}
