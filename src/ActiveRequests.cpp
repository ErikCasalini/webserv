#include "ActiveRequests.hpp"
#include <stdexcept>

ActiveRequests::ActiveRequests(int socket_limit)
: m_socket_limit(socket_limit)
{
	m_request_lst.resize(m_socket_limit);
}

ActiveRequests::~ActiveRequests(void)
{}

int	ActiveRequests::add(int sockfd)
{
	for (int i = 0; i < m_socket_limit; i++) {
		if (m_request_lst.at(i).m_sockfd == -1) {
			m_request_lst.at(i).m_sockfd = sockfd;
			return (i);
		}
	}
	throw std::logic_error("Attempt to add Request while ActiveRequests is full");
}

// int	ActiveRequests::add(int sockfd, int index)
// {
// 	if (index >= 0) {
// 		m_request_lst.at(index).m_sockfd = sockfd;
// 		return (index);
// 	}
// 	throw std::logic_error("Attempt to add Request to invalid index");
// }

int	ActiveRequests::search(int sockfd) const
{
	for (int i = 0; i < m_socket_limit; i++) {
		if (m_request_lst.at(i).m_sockfd == sockfd)
			return (i);
	}
	return (-1);
}

void	ActiveRequests::clear(int sockfd)
{
	for (int i = 0; i < m_socket_limit; i++) {
		if (m_request_lst.at(i).m_sockfd == sockfd) {
			m_request_lst.at(i).clear();
			return ;
		}
	}
}
Request	&ActiveRequests::at(int index) // throw
{
	return (m_request_lst.at(index));
}
