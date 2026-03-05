#ifndef OPENSOCKETS_HPP
# define OPENSOCKETS_HPP

# include <vector>
# include <utility>
# include <sys/socket.h>
# include <iostream>
# include <string>
# include <netinet/in.h>
# include "http_types.h"
# include "EpollManager.hpp"

class Sockets
{
	public:

						Sockets(int socket_limit);
						~Sockets(void);
		void			close(socket_t &socket);
		void			close_all(void);
		void			close_epoll(void);
		int				add(socket_t &socket);
		int				limit(void) const;
		int				size(void) const;
		socket_t		&at(int i);
		int				epoll_inst(void) const;
		epoll_event		*events_addr(void);
		epoll_event		&events_at(int i);
		size_t			events_size(void) const;

	private:

		EpollManager			m_epoll;
		const int				m_socket_limit;
		int						m_size;
		std::vector<socket_t>	m_sockets;

	// DEBUG
		socket_t	debugFd(int i) const;
		bool		exist(int fd) const;
};

#endif
