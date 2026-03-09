#include <map>
#include <string>
#include <iostream>
#include <stdexcept>
#include "main_loop.hpp"
#include "Config.h"
#include "ConfigLexer.h"
#include "ConfigParser.h"
#include "parse_uri_utils.h"
#include "signals_handling.h"

int	main(int argc, char *argv[])
{
	ConfigLexer lexer(argv[1]);
	std::list<std::string> tokens = lexer.lex();
	ConfigParser parser(tokens, argv[1]);
	config_t config = parser.parse();

	try {
		set_signal_handlers();
	}
	catch (std::runtime_error &e) {
		std::cerr << e.what() << '\n';
		return (1);
	}
	std::cout << "PID: " << getpid() << "\n";

	try {
		main_server_loop(config);
	}
	catch (std::exception &e) {
		std::cerr << e.what() << '\n';
		return (1);
	}
	return (0);
}
