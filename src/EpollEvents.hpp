#ifndef EPOLLEVENTS_HPP
# define EPOLLEVENTS_HPP

# include <sys/epoll.h>
# include "main_loop.hpp"
# include "http_types.h"


class EpollEvents
{

public:

						EpollEvents(int socket_limit);
						~EpollEvents(void) {};
	epoll_event			*addr(void);
	epoll_event			&at(int i);
	size_t				size(void) const;

	static epoll_event	create(socket_t *socket, uint32_t events);

private:

	std::vector<epoll_event>	m_events;

};

#endif
