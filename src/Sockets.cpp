#include "Sockets.hpp"
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <sstream>

Sockets::Sockets(int epoll_instance, int socket_limit)
: m_epoll_instance(epoll_instance),
  m_socket_limit(socket_limit),
  m_size(0)
{}

Sockets::~Sockets(void)
{
	for (int i = 0; i < m_socket_limit; i++) {
		if (m_sockets.at(i).fd != -1)
			::close(m_sockets.at(i).fd);
	}
}

void	Sockets::close(socket_t &socket)
{
	for (int i = 0; i < m_socket_limit; i++) {
		if (m_sockets.at(i) == socket) {
			::close(m_sockets.at(i).fd);
			m_sockets.at(i).clear();
			m_size--;
			return ;
		}
	}
	throw std::logic_error("Socket fd not present in Sockets object");
}

socket_t	&Sockets::add(socket_t &socket)
{
	if (m_size < m_socket_limit) {
		for (int i = 0; i < m_socket_limit; i++) {
			if (m_sockets.at(i).fd == -1) {
				m_sockets.at(i) = socket;
				m_size++;
				return (m_sockets.at(i));
			}
		}
	}
	throw std::logic_error("Sockets object is full");
}

// std::string Sockets::info(socket_t &socket) const
// {
// 	for (int i = 0; i < m_socket_limit; i++) {
// 		if (m_sockets.at(i) == socket) {
// 			return (m_sockets.at(i).
// 		}
// 	}
// 	throw std::logic_error("Socket fd not present in Sockets object");
// }

int	Sockets::epollInst(void) const
{
	return (m_epoll_instance);
}

int	Sockets::limit(void) const
{
	return (m_socket_limit);
}

// sockaddr_in	Sockets::peerData(socket_t &socket) const
// {
// 	for (int i = 0; i < m_socket_limit; i++) {
// 		if (m_sockets.at(i) == socket) {
// 			return (m_sockets.at(i).peer_data);
// 		}
// 	}
// 	throw std::logic_error("Socket fd not present in Sockets object");
// }

int	Sockets::size(void) const
{
	return (m_size);
}

//DEBUG
socket_t	Sockets::debugFd(int i) const
{
	return (m_sockets.at(i));
}

bool	Sockets::exist(int fd) const
{
	for (int i = 0; i < m_socket_limit; i++) {
		if (m_sockets.at(i).fd == fd) {
			return (true);
		}
	}
	return (false);
}
