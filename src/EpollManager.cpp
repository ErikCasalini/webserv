#include "EpollManager.hpp"
#include "../include/c_network_exception.h"
#include "unistd.h"

int	EpollManager::sockets_epoll_instance = -1;
int	EpollManager::cgi_epoll_instance = -1;

EpollManager::EpollManager(int event_limit)
{
	epoll_event	data;

	if (sockets_epoll_instance == -1)
		sockets_epoll_instance = epoll_create_ex(1);
	if (cgi_epoll_instance == -1) {
		try {
			cgi_epoll_instance = epoll_create_ex(1);
		} catch (CriticalException &e) {
			close (sockets_epoll_instance);
			throw CriticalException("EpollManager init failure");
		}
	}

	if (m_events.size() < event_limit) {
		if (event_limit <= 0)
			throw std::logic_error("Event limit is not positive");
		std::memset(&data, 0, sizeof(data));
		m_events.resize(event_limit, data);
	}
}

EpollManager::~EpollManager(void)
{
	if (sockets_epoll_instance != -1) {
		close(sockets_epoll_instance);
		sockets_epoll_instance = -1;
	}
	if (cgi_epoll_instance != -1) {
		close(cgi_epoll_instance);
		cgi_epoll_instance = -1;
	}
}

int		EpollManager::epoll_inst(void) const
{
	return (sockets_epoll_instance);
}

int		EpollManager::cgi_epoll_inst(void) const
{
	return (cgi_epoll_instance);
}

epoll_event	*EpollManager::addr(void)
{
	return (&m_events[0]);
}

epoll_event	&EpollManager::at(int i)
{
	return (m_events.at(i));
}

size_t	EpollManager::size(void) const
{
	return (m_events.size());
}

epoll_event	EpollManager::create(socket_t *socket, uint32_t events)
{
	epoll_event	ret;

	if (socket == NULL)
		throw std::logic_error("Attempt to create invalid epoll event");

	std::memset(&ret, 0, sizeof(ret));
	ret.events = events;
	ret.data.ptr = socket;
	return (ret);
}
