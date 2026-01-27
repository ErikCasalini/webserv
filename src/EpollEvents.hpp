#ifndef EPOLLEVENTS_HPP
# define EPOLLEVENTS_HPP

# include <sys/epoll.h>
# include "main_loop.hpp"

class EpollEvents
{

public:

						EpollEvents(int socket_limit);
						~EpollEvents(void) {};
	epoll_event			*addr(void);
	epoll_event			&at(int i);
	size_t				size(void) const;

	static int			getFd(const epoll_event &event);
	static sock_type	getSockType(const epoll_event &event);
	static void			setFd(int fd, epoll_event &event);
	static void			setSockType(sock_type type, epoll_event &event);
	static epoll_event	create(uint32_t events, int fd, sock_type type);

private:

	vec_events			m_events;

};

#endif
