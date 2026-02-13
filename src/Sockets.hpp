#ifndef OPENSOCKETS_HPP
# define OPENSOCKETS_HPP

# include <vector>
# include <utility>
# include <sys/socket.h>
# include <iostream>
# include <string>
# include <netinet/in.h>
# include "http_types.h"

class Sockets
{
	public:

						Sockets(int epoll_instance, int socket_limit);
						~Sockets(void);
		int				add(socket_t &socket);
		void			close(socket_t &socket);
		int				epollInst(void) const;
		int				limit(void) const;
		int				size(void) const;
		socket_t		&at(int i);

	private:

		const int				m_epoll_instance;
		const int				m_socket_limit;
		int						m_size;
		std::vector<socket_t>	m_sockets;

	// DEBUG
		socket_t	debugFd(int i) const;
		bool		exist(int fd) const;
};

#endif
