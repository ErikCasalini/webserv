#include <map>
#include <string>
#include <iostream>
#include "main_loop.hpp"
#include "EpollEvents.hpp"

int	main(void)
{
	temp_config	config;

	try {
		// Hardcode to simulate config file for interfaces
		config.interfaces.push_back(std::pair<const std::string, const short>("127.0.0.1", 4242));
		config.interfaces.push_back(std::pair<const std::string, const short>("127.0.0.1", 5252));
		config.socket_limit = 10;
		main_server_loop(config);
	}
	catch (std::exception &e) {
		std::cerr << e.what();
		return (1);
	}

	/* TEST FD ENCODING */
	// epoll_event	test = EpollEvents::create(EPOLLIN | EPOLLOUT, 5, active);
	// std::cout << "FD=" << EpollEvents::getFd(test) << " Type=" << EpollEvents::getSockType(test) << '\n';
	// EpollEvents::setFd(12, test);
	// std::cout << "FD=" << EpollEvents::getFd(test) << " Type=" << EpollEvents::getSockType(test) << '\n';
	// EpollEvents::setSockType(passive, test);
	// EpollEvents::setFd(100, test);
	// std::cout << "FD=" << EpollEvents::getFd(test) << " Type=" << EpollEvents::getSockType(test) << '\n';
}
