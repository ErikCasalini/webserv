#ifndef OPENSOCKETS_HPP
# define OPENSOCKETS_HPP

# include <vector>
# include <utility>
# include <sys/socket.h>
# include <iostream>
# include <string>
# include <netinet/in.h>

typedef struct sockaddr_in sockaddr_in;

class Sockets
{
	public:

						Sockets(int epoll_instance, int socket_limit);
						~Sockets(void);
		void			add(int sockfd, sockaddr_in &peer_data);
		int				close(int sockfd);
		int				epollInst(void) const;
		int				limit(void) const;
		int				size(void) const;
		sockaddr_in		peerData(int fd) const;
		std::string		info(int fd) const;

	private:

		const int									m_epoll_instance;
		const int									m_socket_limit;
		int											m_size;
		std::vector<std::pair<int, sockaddr_in> >	m_sockets;

	// DEBUG
		int		debugFd(int i) const;
		bool	exist(int fd) const;
};

#endif


