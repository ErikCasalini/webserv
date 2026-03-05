#ifndef EPOLLMANAGER_HPP
# define EPOLLMANAGER_HPP

# include <vector>
# include <sys/epoll.h>

class Response;
class Cgi;
struct socket_t;


class	EpollManager
{

public:

					EpollManager(int event_limit);
					~EpollManager(void);
void				close_inst(void);
int					epoll_inst(void) const;
epoll_event			*events_addr(void);
epoll_event			&events_at(int i);
size_t				events_size(void) const;

static epoll_event	create(socket_t *socket, uint32_t events);
static epoll_event	create(Cgi *cgi, uint32_t events);

private:

	std::vector<epoll_event>	m_events;
	int							m_epoll_inst;
};

#endif
