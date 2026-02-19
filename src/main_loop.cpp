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
#include "Config.h"

void	init_listen_sockets(std::vector<server_t> &servers, Sockets &sockets)
{
	std::vector<server_t>::const_iterator	serv_it = servers.begin();
	epoll_event								event;
	int										server_id = 0;

	while (serv_it < servers.end() && sockets.size() < sockets.limit()) {
		std::vector<listen_t>::const_iterator lis_it = serv_it->listen.begin(); // empecher de lancer si serveur n'a aucun listen (ou pas des serv)

		while (lis_it < serv_it->listen.end() && sockets.size() < sockets.limit()) {
			socket_t socket;
			socket.fd = socket_ex(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0); // throws
			socket.data.sin_family = AF_INET;
			socket.data.sin_addr.s_addr = htonl(lis_it->ip);
			socket.data.sin_port = htons(lis_it->port);
			socket.server_id = server_id;
			socket.type = passive;

			// REUSE SOCKET AFTER SHUTDOWN
			int opt = 1;
			setsockopt(socket.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

			bind_ex(socket.fd, reinterpret_cast<struct sockaddr*>(&socket.data), sizeof(socket.data)); // throws
			listen_ex(socket.fd, 1024); // throws

			int i = sockets.add(socket);
			event = EpollEvents::create(&sockets.at(i), EPOLLIN);
			epoll_ctl_ex(sockets.epollInst(), EPOLL_CTL_ADD, socket.fd, &event); // throws
			lis_it++;
		}
		serv_it++;
		server_id++;
	}
}

void	set_active_socket(socket_t &new_socket, Sockets &sockets)
{
	int			i = sockets.add(new_socket); // Le check de socket.size() est fait avant, on ne peut pas etre ici si sockets est full
	epoll_event	event = EpollEvents::create(&sockets.at(i), EPOLLIN);

	errno = 0;
	epoll_ctl(sockets.epollInst(), EPOLL_CTL_ADD, new_socket.fd, &event);
	switch (errno) {
		case 0:
			std::cout << "[NEW CONNECTION] " << new_socket << '\n';
			break;
		case ENOMEM:
		case ENOSPC:
			std::cout << "[SOCKET ERROR] " << strerror(errno) << " | " << new_socket << " | [CLOSED]\n";
			sockets.close(new_socket); // --> silenlty close unhandlable socket and continue draining.
			errno = 0;
			break;
		default:
			throw CriticalException(strerror(errno));
	}
}

void	accept_new_connections(socket_t *listen_socket, Sockets &sockets)
{
	socket_t	new_socket;
	bool		stop_draining = false;

	if (socket == NULL)
		throw std::logic_error("Invalid socket address");

	while (!stop_draining && sockets.size() < sockets.limit()) {
		errno = 0;
		new_socket.fd = accept(listen_socket->fd, reinterpret_cast<sockaddr*>(&new_socket.peer_data), &new_socket.peer_data_len);
		switch (errno) {
			case 0:
				new_socket.type = active;
				new_socket.server_id = listen_socket->server_id;
				getsockname(new_socket.fd, reinterpret_cast<sockaddr*>(&new_socket.data), &new_socket.peer_data_len);
				set_active_socket(new_socket, sockets); // throws
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

void	close_connection(socket_t *socket, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses)
{
	if (socket == NULL)
		throw std::logic_error("Invalid socket address");

	epoll_ctl_ex(sockets.epollInst(), EPOLL_CTL_DEL, socket->fd, NULL); // throws
	requests.clear(socket); // if address is present
	responses.clear(socket); // if address is present
	sockets.close(*socket);
}

void	handle_error(epoll_event &event, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses)
{
	socket_t *socket = static_cast<socket_t*>(event.data.ptr);

	if (socket == NULL)
		throw std::logic_error("Invalid socket address");

	if (socket->type == passive)
		throw std::runtime_error("[FATAL ERROR] Listen Socket corrupted");

	std::cout << "[SOCKET INTERNAL ERROR] " << *socket << " | [CLOSED]\n";
	close_connection(socket, sockets, requests, responses);
}

void	handle_read_event(epoll_event &event, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses, config_t &config)
{
	socket_t *socket = static_cast<socket_t*>(event.data.ptr);

	if (socket == NULL)
		throw std::logic_error("Invalid socket address");

	if (socket->type == passive)
		accept_new_connections(socket, sockets); // throws

	else {
		int	i_req = requests.search(socket);
		if (i_req == -1)
			i_req = requests.add(socket); // throws
		try {
			requests.at(i_req).parse();
			}
		catch (Request::ConnectionClosed &e) {
			std::cout << "[PEER CLOSED] " << *socket << " | [CLOSED]\n"; // pour debug
			close_connection(socket, sockets, requests, responses);
			return ;
		}
		if (requests.at(i_req).get_infos().status) {
			int i_resp = responses.add(socket, requests.at(i_req).get_infos());// throws if full, vue que aucun READ ne peut arriver tant qu'on a pas send et effacée la response, ca ne peut pas arriver (1 response par fd max)
			event.events = EPOLLOUT;
			epoll_ctl_ex(sockets.epollInst(), EPOLL_CTL_MOD, socket->fd, &event); // throws
			std::cout << requests.at(i_req).get_infos() << '\n'; // DEBUG
			requests.at(i_req).clear_infos();
			responses.at(i_resp).parse_uri();
			responses.at(i_resp).process(config);
			// After process, buffer is ready to be sent, or waiting to be filled by cgi
			// Sending function must check for m_status --> if writing = continue | if cgi = continue waiting/receivng | else = send buffer
			// This function will never be entered again for this request fd, untill the response is not fully sent and deleted
		}
	}
}

void	handle_client_disconnected(epoll_event &event, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses)
{
	socket_t *socket = static_cast<socket_t*>(event.data.ptr);

	if (socket == NULL)
		throw std::logic_error("Invalid socket address");

	if (socket->type == passive)
		throw std::runtime_error("[FATAL ERROR] Listen Socket corrupted");

	std::cout << "[PEER CLOSED] " << *socket << " | [CLOSED]\n"; // pour debug
	close_connection(socket, sockets, requests, responses);
}

void	handle_write_event(epoll_event &event, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses)
{
	socket_t	*socket = static_cast<socket_t*>(event.data.ptr);
	int			i;
	int			ret;

	if (socket == NULL)
		throw std::logic_error("Invalid socket address");

	i =responses.search(socket);
	if (i == -1)
		throw std::logic_error("Attempt to send inexisting Response");

	// if (responses.at(i).get_status() == waiting_cgi)
		// wait cgi
	// else
	switch (responses.at(i).send_response()) {
		case -1:
			close_connection(socket, sockets, requests, responses);
			break ;
		default:
			if (responses.at(i).get_status() == writing)
				break ;
			if (responses.at(i).get_request().headers.keep_alive == false)
				close_connection(socket, sockets, requests, responses);
			else {
				event.events = EPOLLIN;
				epoll_ctl_ex(sockets.epollInst(), EPOLL_CTL_MOD, socket->fd, &event);
				responses.at(i).clear();
			}
			break ;
	}
}

int	main_server_loop(config_t &config)
{
	int							ready_fds;
	EpollEvents					events(config.events.max_connections); // throws
	ActiveMessages<Request>		requests(config.events.max_connections); // throws
	ActiveMessages<Response>	responses(config.events.max_connections); // throws
	Sockets						sockets(epoll_create_ex(1), config.events.max_connections); // throws

	init_listen_sockets(config.http.server, sockets); // throws
	while(1) {
		errno = 0;
		ready_fds = epoll_wait_ex(sockets.epollInst(), events.addr(), events.size(), 0); // throws
		for (int i = 0; i < ready_fds; i++) {
			if (events.at(i).events & EPOLLERR)
				handle_error(events.at(i), sockets, requests, responses);
			else if (events.at(i).events & EPOLLIN)
				handle_read_event(events.at(i), sockets, requests, responses, config);
			else if (events.at(i).events & EPOLLHUP)
				handle_client_disconnected(events.at(i), sockets, requests, responses);
			else if (events.at(i).events & EPOLLOUT)
				handle_write_event(events.at(i), sockets, requests, responses);
		}
	}
}
