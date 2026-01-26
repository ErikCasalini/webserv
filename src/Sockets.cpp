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
{
	sockaddr_in	data;

	std::memset(&data, 0, sizeof(data));
	m_sockets.resize(m_socket_limit, std::make_pair(-1, data));
}

Sockets::~Sockets(void)
{
	for (size_t i = 0; i < m_socket_limit; i++) {
		if (m_sockets.at(i).first != -1)
			::close(m_sockets.at(i).first);
	}
}

int	Sockets::close(int fd)
{
	int	ret = 1;

	for (int i = 0; i < m_socket_limit; i++) {
		if (m_sockets.at(i).first == fd) {
			ret = ::close(m_sockets.at(i).first);
			m_sockets.at(i).first = -1;
			std::memset(&m_sockets.at(i).second, 0, sizeof(m_sockets.at(i).second));
			m_size--;
			return (ret);
		}
	}
	throw std::logic_error("Socket fd not present in Sockets object");
}

void	Sockets::add(int sockfd, sockaddr_in &peer_data)
{
	if (m_size < m_socket_limit) {
		for (int i = 0; i < m_socket_limit; i++) {
			if (m_sockets.at(i).first == -1) {
				m_sockets.at(i).first = sockfd;
				m_sockets.at(i).second = peer_data;
				m_size++;
				return ;
			}
		}
	}
	throw std::logic_error("Sockets object is full");
}

std::string Sockets::info(int fd) const
{
	std::ostringstream	ret;
	uint32_t			ip;
	uint16_t			port;

	for (int i = 0; i < m_socket_limit; i++) {
		if (m_sockets.at(i).first == fd) {
			ip = ntohl(m_sockets.at(i).second.sin_addr.s_addr); // Extraction and good formating of raw data bits
			port = ntohs(m_sockets.at(i).second.sin_port);

			ret << "Socket: fd="
				<< m_sockets.at(i).first
				<< " peer_address="
				<< ((ip >> 24) & 0XFF)
				<< '.'
				<< ((ip >> 16) & 0XFF)
				<< '.'
				<< ((ip >> 8) & 0XFF)
				<< '.'
				<< (ip & 0XFF)
				<< ":"
				<< port;

			return (ret.str());
		}
	}
	throw std::logic_error("Socket fd not present in Sockets object");
}

int	Sockets::epollInst(void) const
{
	return (m_epoll_instance);
}

int	Sockets::limit(void) const
{
	return (m_socket_limit);
}

sockaddr_in	Sockets::peerData(int fd) const
{
	for (int i = 0; i < m_socket_limit; i++) {
		if (m_sockets.at(i).first == fd) {
			return (m_sockets.at(i).second);
		}
	}
	throw std::logic_error("Socket fd not present in Sockets object");
}

int	Sockets::size(void) const
{
	return (m_size);
}

//DEBUG
int	Sockets::debugFd(int i) const
{
	return (m_sockets.at(i).first);
}

bool	Sockets::exist(int fd) const
{
	for (int i = 0; i < m_socket_limit; i++) {
		if (m_sockets.at(i).first == fd) {
			return (true);
		}
	}
	return (false);
}
