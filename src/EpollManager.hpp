#ifndef EPOLLMANAGER_HPP
# define EPOLLMANAGER_HPP

# include <vector>
# include <sys/epoll.h>
# include "Sockets.hpp"
# include "Response.hpp"

class	EpollManager
{

public:

					EpollManager(int event_limit);
					~EpollManager(void);
int					epoll_inst(void) const;
int					cgi_epoll_inst(void) const;
epoll_event			*addr(void);
epoll_event			&at(int i);
size_t				size(void) const;

static epoll_event	create(socket_t *socket, uint32_t events);
static epoll_event	create(socket_t *socket, uint32_t events);

private:

	static std::vector<epoll_event>	m_events;
	static int						sockets_epoll_instance;
	static int						cgi_epoll_instance;
};

#endif
