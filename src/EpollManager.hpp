#ifndef EPOLLMANAGER_HPP
# define EPOLLMANAGER_HPP

# include <vector>
# include <sys/epoll.h>

class Response;
struct socket_t;
struct pipes_t;

class	EpollManager
{

public:

					EpollManager(int event_limit);
					~EpollManager(void);
int					epoll_inst(void) const;
epoll_event			*events_addr(void);
epoll_event			&events_at(int i);
size_t				events_size(void) const;

static epoll_event	create(socket_t *socket, uint32_t events);
static epoll_event	create(pipes_t *pipes, uint32_t events);

private:

	std::vector<epoll_event>	m_events;
	int							m_epoll_inst;
};

#endif
