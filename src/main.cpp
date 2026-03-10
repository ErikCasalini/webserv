#include <string>
#include <iostream>
#include <exception>
#include "main_loop.hpp"
#include "Config.h"
#include "ConfigParser.h"
#include "signals_handling.h"

int	main(int argc, char *argv[])
{
	if (argc != 2) {
		std::cerr << "Usage: webserv CONFIG_FILE\n";
		return (2);
	}

	try {
		config_t config;
		{
			ConfigParser parser(argv[1], argv[0]);
			config = parser.parse();
		}
		set_signal_handlers();
		std::cout << "PID: " << getpid() << "\n";
		main_server_loop(config);
	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return (1);
	}
	return (0);
}
