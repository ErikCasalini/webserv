#include "EpollManager.hpp"
#include "../include/c_network_exception.h"
#include "http_types.h"
#include "Sockets.hpp"
#include "unistd.h"

EpollManager::EpollManager(int event_limit)
: m_epoll_inst(-1)
{
	epoll_event	data;

	if (event_limit <= 0)
		throw std::logic_error("Event limit is not positive");

	m_epoll_inst = epoll_create_ex(1);
	std::memset(&data, 0, sizeof(data));
	m_events.resize(event_limit, data);
}

EpollManager::~EpollManager(void)
{
	if (m_epoll_inst != -1) {
		close(m_epoll_inst);
		m_epoll_inst = -1;
	}
}

int		EpollManager::epoll_inst(void) const
{
	return (m_epoll_inst);
}

epoll_event	*EpollManager::events_addr(void)
{
	return (&m_events[0]);
}

epoll_event	&EpollManager::events_at(int i)
{
	return (m_events.at(i));
}

size_t	EpollManager::events_size(void) const
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

epoll_event	EpollManager::create(Cgi *cgi, uint32_t events)
{
	epoll_event	ret;

	if (cgi == NULL)
		throw std::logic_error("Attempt to create invalid epoll event");

	std::memset(&ret, 0, sizeof(ret));
	ret.events = events;
	ret.data.ptr = cgi;
	return (ret);
}
