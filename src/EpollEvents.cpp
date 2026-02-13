#include "EpollEvents.hpp"
#include <cstring>
#include <stdexcept>

/* CONSTRUCTORS/DESTRUCTORS */

EpollEvents::EpollEvents(int socket_limit)
{
	epoll_event	data;

	if (socket_limit <= 0)
		throw std::logic_error("Socket limit is not positive");
	std::memset(&data, 0, sizeof(data));
	m_events.resize(socket_limit, data);
}

/* MEMBER FUNCTIONS */

epoll_event	*EpollEvents::addr(void)
{
	return (&m_events[0]);
}

epoll_event	&EpollEvents::at(int i)
{
	return (m_events.at(i));
}

size_t	EpollEvents::size(void) const
{
	return (m_events.size());
}

epoll_event	EpollEvents::create(socket_t *socket, uint32_t events)
{
	epoll_event ret;

	if (socket == NULL)
		throw std::logic_error("Attempt to create invalid epoll event");

	std::memset(&ret, 0, sizeof(ret));
	ret.events = events;
	ret.data.ptr = socket;
	return (ret);
}

