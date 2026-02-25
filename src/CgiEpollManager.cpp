#include "CgiEpollManager.hpp"
#include "../include/c_network_exception.h"
#include "unistd.h"

int	CgiEpollManager::cgi_epoll_instance = -1;

CgiEpollManager::CgiEpollManager(void)
{
	if (cgi_epoll_instance == -1)
		cgi_epoll_instance = epoll_create_ex(1);
}

CgiEpollManager::~CgiEpollManager(void)
{
	if (cgi_epoll_instance != -1) {
		close(cgi_epoll_instance);
		cgi_epoll_instance = -1;
	}
}
