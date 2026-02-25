#ifndef CGIEPOLLMANAGER_HPP
# define CGIEPOLLMANAGER_HPP

// # include <vector>
// # include <sys/epoll.h>

class	CgiEpollManager
{

public:

	CgiEpollManager(void);
	~CgiEpollManager(void);

protected:

	static int	cgi_epoll_instance;
};

#endif
