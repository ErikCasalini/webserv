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

/* The uint32_t stored inside the epoll_event data represent an encoding
of the FD number and the sock_type of the registerd event in the following
form : bit 32 = sock_type and bits 1-31 = fd.
Following are the functions to set and retrive those informations */

int	EpollEvents::getFd(const epoll_event &event)
{
	return (event.data.u32 & 0x7FFFFFFF);
}

sock_type	EpollEvents::getSockType(const epoll_event &event)
{
	return (static_cast<sock_type>((event.data.u32 & 0X80000000) >> 31));
}

void	EpollEvents::setFd(int fd, epoll_event &event)
{
	uint32_t temp_flag;

	temp_flag = event.data.u32 & 0x80000000;
	event.data.u32 = temp_flag | fd;
}

void	EpollEvents::setSockType(sock_type type, epoll_event &event)
{
	uint32_t temp_fd;

	temp_fd = event.data.u32 & 0x7FFFFFFF;
	event.data.u32 = temp_fd | (static_cast<uint32_t>(type) << 31);
}

epoll_event	EpollEvents::create(uint32_t events, int fd, sock_type type)
{
	epoll_event ret;

	std::memset(&ret, 0, sizeof(ret));
	ret.events = events;
	setFd(fd, ret);
	setSockType(type, ret);
	return (ret);
}

