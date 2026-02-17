#include <map>
#include <string>
#include <iostream>
#include "main_loop.hpp"
#include "EpollEvents.hpp"
#include "Config.h"

int	main(void)
{
	// Hardcode to simulate config file for interfaces
	config_t			config;
	server_t			server1, server2;
	listen_t			serv1_listen1, serv1_listen2, serv2_listen1, serv2_listen2;
	location_t	serv1_loc1, serv1_loc2, serv2_loc1, serv2_loc2;

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
