#include "Sockets.hpp"
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <sstream>

Sockets::Sockets(int socket_limit)
: m_epoll(socket_limit),
  m_socket_limit(socket_limit),
  m_size(0)
{
	m_sockets.resize(socket_limit);
}

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

// Do not clear
void	Sockets::close_all(void)
{
	for (int i = 0; i < m_socket_limit; i++) {
		if (m_sockets.at(i).fd != -1) {
			::close(m_sockets.at(i).fd);
			m_sockets.at(i).fd = -1;
		}
	}
}

void	Sockets::close_epoll(void)
{
	m_epoll.close_inst();
}

int	Sockets::add(socket_t &socket)
{
	if (m_size < m_socket_limit) {
		for (int i = 0; i < m_socket_limit; i++) {
			if (m_sockets.at(i).fd == -1) {
				m_sockets.at(i) = socket;
				m_size++;
				return (i);
			}
		}
	}
	throw std::logic_error("Sockets object is full");
}

socket_t	&Sockets::at(int i)
{
	return (m_sockets.at(i));
}

int	Sockets::limit(void) const
{
	return (m_socket_limit);
}

int	Sockets::size(void) const
{
	return (m_size);
}

int	Sockets::epoll_inst(void) const
{
	return (m_epoll.epoll_inst());
}

epoll_event	*Sockets::events_addr(void)
{
	return (m_epoll.events_addr());
}

epoll_event	&Sockets::events_at(int i)
{
	return (m_epoll.events_at(i));
}

size_t	Sockets::events_size(void) const
{
	return (m_epoll.events_size());
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
