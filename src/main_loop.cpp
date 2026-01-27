#include "main_loop.hpp"
#include "Sockets.hpp"
#include "EpollEvents.hpp"
#include "ActiveRequests.hpp"
#include "ActiveResponses.hpp"
#include "../include/c_network_exception.h"
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <iostream>
#include <cstring>
#include <cerrno>

void	init_listen_sockets(std::vector<std::pair<const std::string, const short> > &interfaces, Sockets &sockets)
{

	sockaddr_in	sock_data;
	epoll_event	event;
	int			sockfd;

	std::memset(&sock_data, 0, sizeof(sock_data));

	for (size_t i = 0; i < interfaces.size() && sockets.size() < sockets.limit(); i++) {
		sockfd = socket_ex(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0); // throws
		sockets.add(sockfd, sock_data); // throws || Sock_data useless, make it nullable ?

		sock_data.sin_family = AF_INET;
		sock_data.sin_port = htons(interfaces[i].second);
		sock_data.sin_addr.s_addr = INADDR_ANY; // peut etre limiter a ce qui est indiqué dans config

		// REUSE SOCKET AFTER SHUTDOWN
		int opt = 1;
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		bind_ex(sockfd, reinterpret_cast<struct sockaddr*>(&sock_data), sizeof(sock_data)); // throws
		listen_ex(sockfd, 1024); // throws

		event = EpollEvents::create(EPOLLIN, sockfd, passive);
		epoll_ctl_ex(sockets.epollInst(), EPOLL_CTL_ADD, sockfd, &event); // throws
	}
}

void	set_active_socket(int sockfd, Sockets &sockets, sockaddr_in &peer_data)
{
	epoll_event	event = EpollEvents::create(EPOLLIN | EPOLLOUT, sockfd, active); // On mettra que EPOLLIN

	errno = 0;
	epoll_ctl(sockets.epollInst(), EPOLL_CTL_ADD, sockfd, &event);
	switch (errno) {
		case 0:
			sockets.add(sockfd, peer_data); // Le check de socket.size() est fait avant, on ne peut pas etre ici si sockets est full
			std::cout << "[NEW CONNECTION] " << sockets.info(sockfd) << '\n';
			break;
		case ENOMEM:
		case ENOSPC:
			std::cout << "[SOCKET ERROR] " << strerror(errno) << " | " << sockets.info(sockfd) << " | [CLOSED]\n";
			close(sockfd); // --> silenlty close unhandlable socket and continue draining.
			errno = 0;
			break;
		default:
			throw critical_exception(strerror(errno));
	}

}

void	open_new_sockets(int &ready_fds, EpollEvents &events, Sockets &sockets)
{
	int			sockfd;
	bool		stop_draining = false;
	sockaddr_in	peer_data;
	socklen_t	data_len;

	for (size_t i = 0; i < events.size() && ready_fds; i++) {
//		if ((events.at(i).data.u32 == passive) && (events.at(i).events & EPOLLERR)) -> on log et on exit getsockopt(SOL_SOCKET, SO_ERROR)
//		if ((events.at(i).data.u32 == passive) && (events.at(i).events & EPOLLHUP)) -> on log et on exit getsockopt(SOL_SOCKET, SO_ERROR)

		if ((EpollEvents::getSockType(events.at(i)) == passive) &&
			(events.at(i).events & EPOLLIN)) { // throws
			while (!stop_draining && sockets.size() < sockets.limit()) {
				std::memset(&peer_data, 0, sizeof(peer_data));
				data_len = sizeof(peer_data);
				errno = 0;
				sockfd = accept(EpollEvents::getFd(events.at(i)), reinterpret_cast<sockaddr*>(&peer_data), &data_len);
				switch (errno) {
					case 0:
						set_active_socket(sockfd, sockets, peer_data); // throws
						break;
					case ECONNABORTED:
					case EPERM:
					case EINTR:
					case ENETDOWN:
					case EPROTO:
					case ENOPROTOOPT:
					case EHOSTDOWN:
					case ENONET:
					case EHOSTUNREACH:
					case EOPNOTSUPP:
					case ENETUNREACH:
						break;
					case EAGAIN:
					case EMFILE:
					case ENFILE:
					case ENOMEM:
					case ENOBUFS:
						stop_draining = true;
						break;
					default:
						throw critical_exception(strerror(errno));
				}
			}
			ready_fds--;
		}
	}
}

// void	read_from_sockets(ActiveRequests &requests, )
// {

// }

void	handle_active_sockets(int &ready_fds, EpollEvents &events, Sockets &sockets, temp_config &config)
{
	for (size_t i = 0; i < events.size() && ready_fds; i++) {
		if ((EpollEvents::getSockType(events.at(i)) == active) &&
			(events.at(i).events & EPOLLERR)) {
			std::cout << "[SOCKET INTERNAL ERROR] " << sockets.info(events.getFd(events.at(i))) << " | [CLOSED]\n";
			epoll_ctl(sockets.epollInst(), EPOLL_CTL_DEL, EpollEvents::getFd(events.at(i)), 0);
			sockets.close(EpollEvents::getFd(events.at(i)));
		}
		else if ((EpollEvents::getSockType(events.at(i)) == active) &&
				(events.at(i).events & EPOLLHUP)) {
			std::cout << "[PEER CLOSED] " << sockets.info(events.getFd(events.at(i))) << " | [CLOSED]\n";
			epoll_ctl(sockets.epollInst(), EPOLL_CTL_DEL, EpollEvents::getFd(events.at(i)), 0);
			sockets.close(EpollEvents::getFd(events.at(i)));
		}
		/* Si EPOLLIN -> jules(events.at(i).data.fd, config) -> Si requete complete -> set flag "ready" ou flag "pas bon" si ca va pas | et epoll_ctl(Enleve EPOLLIN et met EPOLLOUT)
		   Si EPOLLOUT -> traite la reponse et on ecrit une fois -> si tout est ecrit -> epoll_ctl(Enleve EPOLLOUT et met EPOLLIN)*/
		else if ((EpollEvents::getSockType(events.at(i)) == active) &&
				(events.at(i).events & EPOLLIN)) {
			char	buf[1000];
			int		ret;
			std::memset(buf, 0, sizeof(buf));
			ret = recv(EpollEvents::getFd(events.at(i)), buf, sizeof(buf), 0);
			if (ret > 0)
				std::cout << "FD: " << EpollEvents::getFd(events.at(i)) << " " << buf;
			else {
				epoll_ctl(sockets.epollInst(), EPOLL_CTL_DEL, EpollEvents::getFd(events.at(i)), 0);
				sockets.close(EpollEvents::getFd(events.at(i)));
				std::cout << "FD: " << EpollEvents::getFd(events.at(i)) << " CLOSED\n";
			}
			//jules(events.at(i).data.fd, config); --> consume le fd ou stop si read trop long et le conserve dans un buffer
		}
		ready_fds--;
	}
}

int	main_server_loop(temp_config &config)
{
	int				ready_fds;
	EpollEvents		events(config.socket_limit); // throws
	ActiveRequests	requests(config.socket_limit); // throws
	ActiveResponses	responses(config.socket_limit); // throws
	Sockets			sockets(epoll_create_ex(1), config.socket_limit); // throws

	init_listen_sockets(config.interfaces, sockets); // throws
	while(1) {
		errno = 0;
		// std::cout << "MAIN LOOP\n";
		ready_fds = epoll_wait_ex(sockets.epollInst(), events.addr(), events.size(), 0); // throws
		while (ready_fds > 0) {
			open_new_sockets(ready_fds, events, sockets); // throws
			handle_active_sockets(ready_fds, events, sockets, config); // --> create read socket
		}
	}
}
