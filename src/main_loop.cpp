#include "main_loop.hpp"
#include "CgiParser.h"
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
#include "Cookies.hpp"

using std::string;
using std::vector;
using std::logic_error;
using std::runtime_error;

extern volatile sig_atomic_t int_signal;

void	init_listen_sockets(vector<server_t> &servers, Sockets &sockets)
{
	vector<server_t>::const_iterator	serv_it = servers.begin();
	epoll_event							event;
	int									server_id = 0;

	if (servers.size() == 0)
		throw CriticalException("\033[1;31mNo servers declared in config file\033[0m");

	while (serv_it < servers.end() && sockets.size() < sockets.limit()) {
		vector<listen_t>::const_iterator lis_it = serv_it->listen.begin();
		if (lis_it == serv_it->listen.end())
			throw CriticalException("\033[1;31mNo listen interface for declared server\033[0m");

		while (lis_it < serv_it->listen.end()) {
			if (sockets.size() >= sockets.limit())
				throw CriticalException("\033[1;31mNot enought connections allowed to satisfy all the listen interfaces\033[0m");
			socket_t socket;
			socket.fd = socket_ex(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
			socket.local_data.sin_family = AF_INET;
			socket.local_data.sin_addr.s_addr = htonl(lis_it->ip);
			socket.local_data.sin_port = htons(lis_it->port);
			socket.server_id = server_id;
			socket.socktype = passive;

			// REUSE SOCKET AFTER SHUTDOWN
			int opt = 1;
			setsockopt(socket.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

			try {
				bind_ex(socket.fd, reinterpret_cast<struct sockaddr*>(&socket.local_data), sizeof(socket.local_data));
				listen_ex(socket.fd, 1024);
			}
			catch (CriticalException &e) {
				close(socket.fd);
				throw (e);
			}

			int i = sockets.add(socket);
			event = EpollManager::create(&sockets.at(i), EPOLLIN);
			epoll_ctl_ex(sockets.epoll_inst(), EPOLL_CTL_ADD, socket.fd, &event);
			lis_it++;
		}
		serv_it++;
		server_id++;
	}
}

void	set_active_socket(socket_t &new_socket, Sockets &sockets)
{
	int			i = sockets.add(new_socket);
	epoll_event	event = EpollManager::create(&sockets.at(i), EPOLLIN);

	errno = 0;
	epoll_ctl(sockets.epoll_inst(), EPOLL_CTL_ADD, new_socket.fd, &event);
	switch (errno) {
		case 0:
			std::cout << "\033[1;32m[NEW CONNECTION]\033[0m " << new_socket << '\n';
			break;
		case ENOMEM:
		case ENOSPC:
			std::cout << "\033[1;31m[SOCKET ERROR]\033[0m " << strerror(errno) << " | " << new_socket << " | [CLOSED]\n";
			sockets.close(new_socket); // --> silenlty close unhandlable socket and continue draining.
			errno = 0;
			break;
		default:
			throw CriticalException(string("epoll_ctl(): ") + strerror(errno));
	}
}

void	accept_new_connections(socket_t *listen_socket, Sockets &sockets)
{
	socket_t	new_socket;
	bool		stop_draining = false;

	if (listen_socket == NULL)
		throw logic_error("Invalid socket address");

	while (!stop_draining && sockets.size() < sockets.limit()) {
		errno = 0;
		new_socket.fd = accept(listen_socket->fd, reinterpret_cast<sockaddr*>(&new_socket.peer_data), &new_socket.peer_data_len);
		switch (errno) {
			case 0:
				new_socket.socktype = active;
				new_socket.server_id = listen_socket->server_id;
				getsockname(new_socket.fd, reinterpret_cast<sockaddr*>(&new_socket.local_data), &new_socket.local_data_len);
				new_socket.last_activity = std::time(NULL);
				set_active_socket(new_socket, sockets);
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
				throw CriticalException(string("accept(): ") + strerror(errno));
		}
	}
}

void	close_connection(socket_t *socket, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses)
{
	if (socket == NULL)
		throw logic_error("Invalid socket address");

	epoll_ctl_ex(sockets.epoll_inst(), EPOLL_CTL_DEL, socket->fd, NULL);
	requests.clear(socket); // if address is present
	responses.clear(socket); // if address is present
	sockets.close(*socket);
}

void	handle_error(epoll_event &event, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses)
{
	epoll_item_t	*item = static_cast<epoll_item_t*>(event.data.ptr);

	if (item == NULL)
		throw logic_error("Invalid item address");

	if (item->type == sockt) {
		socket_t	*socket = static_cast<socket_t*>(item);

		if (socket->socktype == passive)
			throw runtime_error("\033[1;31m[FATAL ERROR]\033[0m Listen Socket corrupted");
		// CLOSE
		std::cout << "\033[1;31m[SOCKET INTERNAL ERROR]\033[0m " << *socket << " | \033[1;32m[CLOSED]\033[0m\n";
		close_connection(socket, sockets, requests, responses);
	}
	else if (item->type == cgi) {
		Cgi			*cgi = static_cast<Cgi*>(item);
		int			i = responses.search(cgi->get_socket());

		if (i == -1)
			throw logic_error("Attempt to dereference nonexistent Response");

		// RESET CGI, TRACK SOCKET AGAIN, SEND ERR 500
		responses.at(i).handle_cgi_error(sockets);
	}
}

void	handle_read_event(epoll_event &event, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses)
{
	epoll_item_t	*item = static_cast<epoll_item_t*>(event.data.ptr);
	status_t		req_status;

	if (item == NULL)
		throw std::logic_error("Invalid item address");

	if (item->type == sockt) {
		socket_t	*sock = static_cast<socket_t*>(item);

		if (sock->socktype == passive)
			accept_new_connections(sock, sockets);
		else {
			// CHECK REQUEST
			int	i_req = requests.search(sock);
			if (i_req == -1)
				i_req = requests.add(sock);
			try {
				requests.at(i_req).parse();
				}
			catch (Request::ConnectionClosed &e) {
				std::cout << "\033[1;35m[PEER CLOSED]\033[0m " << *sock << '\n';
				close_connection(sock, sockets, requests, responses);
				return ;
			}
			req_status = requests.at(i_req).get_infos().status;
			if (req_status != parsing) {
				// CREATING RESPONSE
				requests.at(i_req).m_socket->last_activity = std::time(NULL);
				int i_resp = responses.add(sock, requests.at(i_req).get_infos());
				event.events = EPOLLOUT;
				epoll_ctl_ex(sockets.epoll_inst(), EPOLL_CTL_MOD, sock->fd, &event);
				std::cout << requests.at(i_req).get_infos() << '\n';
				requests.at(i_req).clear_infos();
				responses.at(i_resp).set_status(req_status);
				if (req_status == ok)
					responses.at(i_resp).parse_uri();
				responses.at(i_resp).process(sockets);
			}
		}
	}
	else if (item->type == cgi) {
		Cgi	*cgi = static_cast<Cgi*>(item);
		int		i = responses.search(cgi->get_socket());

		if (i == -1)
			throw logic_error("Attempt to dereference nonexistent Response");

		if (cgi->timeout() || cgi->read_child_response(sockets.epoll_inst()) == -1)
			responses.at(i).handle_cgi_error(sockets);
		else if (cgi->get_status() == done) {
			// READING FROM CHILD DONE -> ADD SOCKET TRACKING AGAIN AND HANDLE RESPONSE
			epoll_event	new_event = EpollManager::create(cgi->get_socket(), EPOLLOUT);
			epoll_ctl(sockets.epoll_inst(), EPOLL_CTL_ADD, cgi->get_socket()->fd, &new_event);
		}
	}
}

void	handle_client_disconnected(epoll_event &event, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses)
{
	epoll_item_t	*item = static_cast<epoll_item_t*>(event.data.ptr);

	if (item == NULL)
		throw logic_error("Invalid item address");

	if (item->type == sockt) {
		socket_t	*socket = static_cast<socket_t*>(item);

		if (socket->socktype == passive)
			throw runtime_error("\033[1;31m[FATAL ERROR]\033[0m Listen Socket corrupted");
		// CLOSE
		std::cout << "\033[1;35m[PEER CLOSED]\033[0m " << *socket << '\n'; // pour debug
		close_connection(socket, sockets, requests, responses);
	}
	else if (item->type == cgi) {
		Cgi		*cgi = static_cast<Cgi*>(item);
		int			i = responses.search(cgi->get_socket());

		if (i == -1)
			throw logic_error("Attempt to dereference nonexistent Response");

		if (cgi->get_status() == write_to_child)
			// RESET CGI, TRACK SOCKET AGAIN, SEND ERR 500
			responses.at(i).handle_cgi_error(sockets);
		else if (cgi->get_status() == read_from_child) {
			// CHILD CLOSED --> CHECK EXIT STATUS
			int		wstatus = 0;
			pid_t	w = waitpid(cgi->get_child_pid(), &wstatus, WNOHANG);

			if (w < 0 || !WIFEXITED(wstatus) || WEXITSTATUS(wstatus) != 0) {
			// CHILD OR WAITPID FAILED --> CLEAN, SET FD TRACKING AGAIN, SEND ERR 500
				cgi->reset_child_pid();
				responses.at(i).handle_cgi_error(sockets);
				return ;
			}
			// CHILD SUCCEEDED OR NOT EXITED YET--> CLEAN, SET FD TRACKING AGAIN, HANDLE RESPONSE
			cgi->reset_state(sockets.epoll_inst());

			if (!responses.at(i).get_location()->cgi_nph) {
				CgiParser cgi_response(&responses.at(i));
				cgi_response.parse();
			}

			epoll_event	new_event = EpollManager::create(cgi->get_socket(), EPOLLOUT);
			responses.at(i).set_status(sending_resp);
			epoll_ctl_ex(sockets.epoll_inst(), EPOLL_CTL_ADD, cgi->get_socket()->fd, &new_event);
		}
		else
			throw logic_error("Epoll received a Cgi event in a forbidden state");
	}
}

void	handle_write_event(epoll_event &event, Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses)
{
	epoll_item_t	*item = static_cast<epoll_item_t*>(event.data.ptr);
	int				i;

	if (item == NULL)
		throw logic_error("Invalid item address");

	if (item->type == sockt) {
		socket_t	*sock = static_cast<socket_t*>(item);
		i = responses.search(sock);

		if (i == -1)
			throw logic_error("Attempt to dereference nonexistent Response");

		if (responses.at(i).send_response() == -1)
			// ERROR --> CLOSE
			close_connection(sock, sockets, requests, responses);
		else {
			if (responses.at(i).get_status() == sending_resp)
			// CONTINUE SENDING...
				return ;
			if (responses.at(i).get_headers().keep_alive == false) {
			// DONE --> CLOSE
				std::cout << "\033[1;35m[CONNECTION CLOSED]\033[0m " << *sock << '\n';
				close_connection(sock, sockets, requests, responses);
			}
			else {
			// DONE --> KEEP ALIVE
				event.events = EPOLLIN;
				epoll_ctl_ex(sockets.epoll_inst(), EPOLL_CTL_MOD, sock->fd, &event);
				responses.at(i).m_socket->last_activity = std::time(NULL);
				responses.at(i).clear();
			}
		}
	}
	else if (item->type == cgi) {
		Cgi		*cgi = static_cast<Cgi*>(item);
		i = responses.search(cgi->get_socket());

		if (i == -1)
			throw logic_error("Attempt to dereference nonexistent Response");

		if (cgi->timeout() || cgi->write_body_to_child(sockets.epoll_inst()) == -1)
			// RESET CGI, TRACK SOCKET AGAIN, SEND ERR 500
			responses.at(i).handle_cgi_error(sockets);
	}
}

void	terminate_pending_cgi(Sockets &sockets, ActiveMessages<Response> &responses)
{
	for (size_t i = 0; i < responses.size(); i++) {
		if (responses.at(i).cgi_timeout())
			responses.at(i).handle_cgi_error(sockets);
	}
}

void	close_pending_connections(Sockets &sockets, ActiveMessages<Request> &requests, ActiveMessages<Response> &responses, config_t &config)
{
	for (size_t i_sock = 0; i_sock < sockets.limit(); i_sock++) {
		if (sockets.at(i_sock).timeout(config)) {
			int	i_resp = responses.search(&sockets.at(i_sock));

			if (i_resp == -1 || responses.at(i_resp).cgi_timeout()) {
				std::cout << "\033[1;33m[TIMEOUT]\033[0m " << sockets.at(i_sock) << '\n';
				close_connection(&sockets.at(i_sock), sockets, requests, responses);
			}
		}
	}
}

void	main_server_loop(config_t &config)
{
	int							ready_fds;
	Cookies						cookie_jar;
	Sockets						sockets(config.events.max_connections);
	ActiveMessages<Request>		requests(config, cookie_jar);
	ActiveMessages<Response>	responses(config, cookie_jar);

	init_listen_sockets(config.http.server, sockets);
	while(1) {
		errno = 0;
		ready_fds = epoll_wait_ex(sockets.epoll_inst(), sockets.events_addr(), sockets.events_size(), 0);
		for (int i = 0; i < ready_fds; i++) {
			if (int_signal) {
				std::cout << "\nQuitting...\n";
				return ;
			}
			if (sockets.events_at(i).events & EPOLLERR)
				handle_error(sockets.events_at(i), sockets, requests, responses);
			else if (sockets.events_at(i).events & EPOLLIN)
				handle_read_event(sockets.events_at(i), sockets, requests, responses);
			else if (sockets.events_at(i).events & EPOLLHUP)
				handle_client_disconnected(sockets.events_at(i), sockets, requests, responses);
			else if (sockets.events_at(i).events & EPOLLOUT)
				handle_write_event(sockets.events_at(i), sockets, requests, responses);
		}
		if (int_signal) {
				std::cout << "\nQuitting...\n";
				return ;
		}
		terminate_pending_cgi(sockets, responses);
		close_pending_connections(sockets, requests, responses, config);
		cookie_jar.remove_expired();
	}
}
