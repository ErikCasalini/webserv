#include "main_loop.hpp"
#include "Sockets.hpp"
#include "ActiveMessages.hpp"
#include "request_parser.h"
#include "Response.hpp"
#include "../include/c_network_exception.h"
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <iostream>
#include <cstring>
#include <cerrno>
#include "Config.h"

extern volatile sig_atomic_t int_signal;

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
			socket.local_data.sin_family = AF_INET;
			socket.local_data.sin_addr.s_addr = htonl(lis_it->ip);
			socket.local_data.sin_port = htons(lis_it->port);
			socket.server_id = server_id;
			socket.socktype = passive;

			// REUSE SOCKET AFTER SHUTDOWN
			int opt = 1;
			setsockopt(socket.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

			bind_ex(socket.fd, reinterpret_cast<struct sockaddr*>(&socket.local_data), sizeof(socket.local_data)); // throws
			listen_ex(socket.fd, 1024); // throws

			int i = sockets.add(socket);
			event = EpollManager::create(&sockets.at(i), EPOLLIN);
			epoll_ctl_ex(sockets.epoll_inst(), EPOLL_CTL_ADD, socket.fd, &event); // throws
			lis_it++;
		}
		serv_it++;
		server_id++;
	}
}

void	set_active_socket(socket_t &new_socket, Sockets &sockets)
{
	int			i = sockets.add(new_socket); // Le check de socket.size() est fait avant, on ne peut pas etre ici si sockets est full
	epoll_event	event = EpollManager::create(&sockets.at(i), EPOLLIN);

	errno = 0;
	epoll_ctl(sockets.epoll_inst(), EPOLL_CTL_ADD, new_socket.fd, &event);
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
				new_socket.socktype = active;
				new_socket.server_id = listen_socket->server_id;
				getsockname(new_socket.fd, reinterpret_cast<sockaddr*>(&new_socket.local_data), &new_socket.local_data_len);
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

	epoll_ctl_ex(sockets.epoll_inst(), EPOLL_CTL_DEL, socket->fd, NULL); // throws
	requests.clear(socket); // if address is present
	responses.clear(socket); // if address is present
	sockets.close(*socket);
}

void	handle_error(epoll_event &event, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses, config_t &config)
{
	epoll_item_t	*item = static_cast<epoll_item_t*>(event.data.ptr);

	if (item == NULL)
		throw std::logic_error("Invalid item address");

	if (item->type == sockt) {
		socket_t	*socket = static_cast<socket_t*>(item);

		if (socket->socktype = passive)
			throw std::runtime_error("[FATAL ERROR] Listen Socket corrupted");
		// CLOSE
		std::cout << "[SOCKET INTERNAL ERROR] " << *socket << " | [CLOSED]\n";
		close_connection(socket, sockets, requests, responses);
	}
	else if (item->type == pipeline) {
		pipes_t		*pipes = static_cast<pipes_t*>(item);
		epoll_event	new_event = EpollManager::create(pipes->response_socket, EPOLLOUT);
		int			i = responses.search(pipes->response_socket);

		if (i == -1)
			throw std::logic_error("Attempt to dereference nonexistent Response");
		// CLEAN
		responses.at(i).clear_cgi_pipes(sockets.epoll_inst());
		responses.at(i).terminate_child();
		// SET FD TRACKING AGAIN AND SEND ERR 500
		std::cout << "[CGI INTERNAL ERROR] Error on child pipe" << " | [SENDING ERR 500]\n";
		epoll_ctl_ex(sockets.epoll_inst(), EPOLL_CTL_ADD, pipes->response_socket->fd, &new_event);
		responses.at(i).set_status(internal_err);
		responses.at(i).process(config, sockets.epoll_inst());
	}
}

void	handle_read_event(epoll_event &event, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses, config_t &config)
{
	epoll_item_t	*item = static_cast<epoll_item_t*>(event.data.ptr);
	status_t		req_status;

	if (item == NULL)
		throw std::logic_error("Invalid item address");

	if (item->type == sockt) {
		socket_t	*sock = static_cast<socket_t*>(item);

		if (sock->socktype == passive)
			accept_new_connections(sock, sockets); // throws
		else {
			// CHECK REQUEST
			int	i_req = requests.search(sock);
			if (i_req == -1)
				i_req = requests.add(sock); // throws
			try {
				requests.at(i_req).parse();
				}
			catch (Request::ConnectionClosed &e) {
				std::cout << "[PEER CLOSED] " << *sock << " | [CLOSED]\n"; // pour debug
				close_connection(sock, sockets, requests, responses);
				return ;
			}
			req_status = requests.at(i_req).get_infos().status;
			if (req_status != parsing) {
				// CREATING RESPONSE
				int i_resp = responses.add(sock, requests.at(i_req).get_infos());// throws if full, vue que aucun READ ne peut arriver tant qu'on a pas send et effacée la response, ca ne peut pas arriver (1 response par fd max)
				event.events = EPOLLOUT;
				epoll_ctl_ex(sockets.epoll_inst(), EPOLL_CTL_MOD, sock->fd, &event);
				std::cout << requests.at(i_req).get_infos() << '\n'; // DEBUG
				requests.at(i_req).clear_infos();
				if (req_status == bad_request)
					responses.at(i_req).set_status(bad_request);
				else
					responses.at(i_resp).parse_uri();
				responses.at(i_resp).process(config, sockets.epoll_inst()); // unset tracking of socket fd if CGI
			}
		}
	}
	else if (item->type == pipeline) {
		pipes_t	*pipes = static_cast<pipes_t*>(item);
		int		i = responses.search(pipes->response_socket);
		ssize_t	ret;

		if (i == -1)
			throw std::logic_error("Attempt to dereference nonexistent Response");

		responses.at(i).read_cgi_response(sockets.epoll_inst(), config);
	}
}

void	handle_client_disconnected(epoll_event &event, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses, config_t &config)
{
	epoll_item_t	*item = static_cast<epoll_item_t*>(event.data.ptr);

	if (item == NULL)
		throw std::logic_error("Invalid item address");

	if (item->type == sockt) {
		socket_t	*socket = static_cast<socket_t*>(item);

		if (socket->socktype = passive)
			throw std::runtime_error("[FATAL ERROR] Listen Socket corrupted");
		// CLOSE
		std::cout << "[PEER CLOSED] " << *socket << " | [CLOSED]\n"; // pour debug
		close_connection(socket, sockets, requests, responses);
	}
	else if (item->type == pipeline) {
		pipes_t		*pipes = static_cast<pipes_t*>(item);
		int			i = responses.search(pipes->response_socket);

		if (i == -1)
			throw std::logic_error("Attempt to dereference nonexistent Response");

		if (responses.at(i).get_status() == writing_to_cgi)
			// CLEAN, SET FD TRACKING AGAIN, SEND ERR 500
			responses.at(i).handle_cgi_error(sockets.epoll_inst(), config);
		else if (responses.at(i).get_status() == reading_from_cgi) {
			// CLIENT CLOSED --> CLEAN
			responses.at(i).clear_cgi_pipes(sockets.epoll_inst());
			responses.at(i).terminate_child();
			// SET FD TRACKING AGAIN AND HANDLE RESPONSE
			epoll_event	new_event = EpollManager::create(pipes->response_socket, EPOLLOUT);
			responses.at(i).set_status(sending_resp);
			epoll_ctl_ex(sockets.epoll_inst(), EPOLL_CTL_ADD, pipes->response_socket->fd, &new_event);
		}
	}
}

void	handle_write_event(epoll_event &event, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses, config_t &config)
{
	epoll_item_t	*item = static_cast<epoll_item_t*>(event.data.ptr);
	int				i;

	if (item == NULL)
		throw std::logic_error("Invalid item address");

	if (item->type == sockt) {
		socket_t	*sock = static_cast<socket_t*>(item);
		i = responses.search(sock);

		if (i == -1)
			throw std::logic_error("Attempt to dereference nonexistent Response");

		if (responses.at(i).send_response() == -1)
			// ERROR --> CLOSE
			close_connection(sock, sockets, requests, responses);
		else {
			if (responses.at(i).get_status() == sending_resp)
			// CONTINUE SENDING...
				return ;
			if (responses.at(i).get_request().headers.keep_alive == false)
			// DONE --> CLOSE
				close_connection(sock, sockets, requests, responses);
			else {
			// DONE --> KEEP ALIVE
				event.events = EPOLLIN;
				epoll_ctl_ex(sockets.epoll_inst(), EPOLL_CTL_MOD, sock->fd, &event);
				responses.at(i).clear();
			}
		}
	}
	else if (item->type == pipeline) {
		pipes_t		*pipes = static_cast<pipes_t*>(item);
		i = responses.search(pipes->response_socket);

		if (i == -1)
			throw std::logic_error("Attempt to dereference nonexistent Response");

		responses.at(i).write_body_to_cgi(sockets.epoll_inst(), config);
	}
}

void	reap_children(void)
{
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

void	main_server_loop(config_t &config)
{
	int							ready_fds;
	Sockets						sockets(config.events.max_connections); // throws
	ActiveMessages<Request>		requests(config.events.max_connections); // throws
	ActiveMessages<Response>	responses(config.events.max_connections); // throws

	init_listen_sockets(config.http.server, sockets); // throws
	while(1) {
		errno = 0;
		ready_fds = epoll_wait_ex(sockets.epoll_inst(), sockets.events_addr(), sockets.events_size(), 0); // throws
		for (int i = 0; i < ready_fds; i++) {
			if (int_signal)
				return ;
			if (sockets.events_at(i).events & EPOLLERR)
				handle_error(sockets.events_at(i), sockets, requests, responses, config);
			else if (sockets.events_at(i).events & EPOLLIN)
				handle_read_event(sockets.events_at(i), sockets, requests, responses, config);
			else if (sockets.events_at(i).events & EPOLLHUP)
				handle_client_disconnected(sockets.events_at(i), sockets, requests, responses, config);
			else if (sockets.events_at(i).events & EPOLLOUT)
				handle_write_event(sockets.events_at(i), sockets, requests, responses, config);
		}
		if (int_signal)
			return ;
		reap_children();
	}
}
