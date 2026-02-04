#include "main_loop.hpp"
#include "Sockets.hpp"
#include "EpollEvents.hpp"
#include "ActiveMessages.hpp"
#include "request_parser.h"
#include "Response.hpp"
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
	epoll_event	event = EpollEvents::create(EPOLLIN, sockfd, active);

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
			throw CriticalException(strerror(errno));
	}
}

void	accept_new_connections(int listen_fd, Sockets &sockets)
{
	int			new_sockfd;
	bool		stop_draining = false;
	sockaddr_in	peer_data;
	socklen_t	data_len;

	while (!stop_draining && sockets.size() < sockets.limit()) {
		std::memset(&peer_data, 0, sizeof(peer_data));
		data_len = sizeof(peer_data);
		errno = 0;
		new_sockfd = accept(listen_fd, reinterpret_cast<sockaddr*>(&peer_data), &data_len);
		switch (errno) {
			case 0:
				set_active_socket(new_sockfd, sockets, peer_data); // throws
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
				throw CriticalException(strerror(errno));
		}
	}
}

void	close_connection(int sockfd, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses)
{
	epoll_ctl_ex(sockets.epollInst(), EPOLL_CTL_DEL, sockfd, NULL); // throws
	requests.clear(sockfd); // si present
	responses.clear(sockfd); // si present
	sockets.close(sockfd);
}

void	handle_error(epoll_event &event, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses)
{
	int	sockfd;

	if (EpollEvents::getSockType(event) == passive)
		throw std::runtime_error("[FATAL ERROR] Listen Socket corrupted");

	sockfd = EpollEvents::getFd(event);
	std::cout << "[SOCKET INTERNAL ERROR] " << sockets.info(sockfd) << " | [CLOSED]\n";
	close_connection(sockfd, sockets, requests, responses);
}

void	handle_read_event(epoll_event &event, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses)
{
	int	sockfd = EpollEvents::getFd(event);

	if (EpollEvents::getSockType(event) == passive)
		accept_new_connections(sockfd, sockets); // throws

	else {
		int	i_req = requests.search(sockfd);
		if (i_req == -1)
			i_req = requests.add(sockfd); // throws
		if (requests.at(i_req).read_socket() == 0) { // throws
			std::cout << "[PEER CLOSED] " << sockets.info(sockfd) << " | [CLOSED]\n"; // pour debug
			close_connection(sockfd, sockets, requests, responses);
			return ;
		}
		requests.at(i_req).parse();
		if (requests.at(i_req).get_request().status) {
			int i_resp = responses.add(sockfd, requests.at(i_req).get_request());// throws if full, vue que aucun READ ne peut arriver tant qu'on a pas send et effacée la response, ca ne peut pas arriver (1 response par fd max)
			event.events = EPOLLOUT;
			epoll_ctl_ex(sockets.epollInst(), EPOLL_CTL_MOD, sockfd, &event); // throws
			std::cout << requests.at(i_req).get_request() << '\n'; // DEBUG
			requests.at(i_req).clear_request();
			responses.at(i_resp).parse_uri();
		}
	}
}

void	handle_client_disconnected(epoll_event &event, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses)
{
	int	sockfd = EpollEvents::getFd(event);

	if (EpollEvents::getSockType(event) == passive)
		throw std::runtime_error("[FATAL ERROR] Listen Socket corrupted");

	std::cout << "[PEER CLOSED] " << sockets.info(sockfd) << " | [CLOSED]\n"; // pour debug
	close_connection(sockfd, sockets, requests, responses);
}

void	handle_write_event(epoll_event &event, Sockets &sockets, ActiveMessages<Response> &responses)
{
	int	sockfd = EpollEvents::getFd(event);
	int	i = responses.search(sockfd);
	std::string content(responses.at(i).get_path() + " " + responses.at(i).get_querry());

	// DEBUG
	send(sockfd, content.c_str(), content.size(), SOCK_NONBLOCK);
	event.events = EPOLLIN;
	epoll_ctl_ex(sockets.epollInst(), EPOLL_CTL_MOD, sockfd, &event);
	responses.at(i).clear(); // on enleve pour l'instant
}

int	main_server_loop(temp_config &config)
{
	int							ready_fds;
	EpollEvents					events(config.socket_limit); // throws
	ActiveMessages<Request>		requests(config.socket_limit); // throws
	ActiveMessages<Response>	responses(config.socket_limit); // throws
	Sockets						sockets(epoll_create_ex(1), config.socket_limit); // throws

	init_listen_sockets(config.interfaces, sockets); // throws
	while(1) {
		errno = 0;
		ready_fds = epoll_wait_ex(sockets.epollInst(), events.addr(), events.size(), 0); // throws
		for (int i = 0; i < ready_fds; i++) {
			if (events.at(i).events & EPOLLERR)
				handle_error(events.at(i), sockets, requests, responses);
			else if (events.at(i).events & EPOLLIN)
				handle_read_event(events.at(i), sockets, requests, responses);
			else if (events.at(i).events & EPOLLHUP)
				handle_client_disconnected(events.at(i), sockets, requests, responses);
			else if (events.at(i).events & EPOLLOUT)
				handle_write_event(events.at(i), sockets, responses);
		}
	}
}
