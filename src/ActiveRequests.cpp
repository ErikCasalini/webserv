#include "ActiveRequests.hpp"
#include <stdexcept>

ActiveRequests::ActiveRequests(int socket_limit)
: m_socket_limit(socket_limit)
{
	m_request_lst.resize(m_socket_limit);
}

ActiveRequests::~ActiveRequests(void)
{}

/*
	If sockfd is already associated with a request object
		--> return this object
	Else
		--> sockfd is associated with the empty request
		object closest to the top of the list and return this object
*/
Request	&ActiveRequests::add(int sockfd)
{
	int	index = -1;

	for (int i = 0; i < m_socket_limit; i++) {
		if (m_request_lst.at(i).m_sockfd == sockfd)
			return (m_request_lst.at(i));
		if (index < 0)
			index = i;
		if (i == m_socket_limit)
			throw std::logic_error("Attempt to add a new Request to"
								" ActiveRequests vector while full");
	}
	m_request_lst.at(index).m_sockfd = sockfd;
	return (m_request_lst.at(index));
}

void	ActiveRequests::remove(int sockfd)
{
	for (int i = 0; i < m_socket_limit; i++) {
		if (m_request_lst.at(i).m_sockfd == sockfd) {
			m_request_lst.at(i).clear();
			return ;
		}
	}
	throw std::logic_error("Attempt to remove inexistent Request");
}
