#ifndef ACTIVEMESSAGES_HPP
# define ACTIVEMESSAGES_HPP

# include "request_parser.h"
# include <vector>

template <typename T>
class	ActiveMessages
{
	public:

		ActiveMessages(int socket_limit)
		: m_socket_limit(socket_limit)
		{
			m_messages_lst.resize(m_socket_limit);
		}

		~ActiveMessages(void)
		{}

		int	add(int sockfd)
		{
			for (int i = 0; i < m_socket_limit; i++) {
				if (m_messages_lst.at(i).m_sockfd == -1) {
					m_messages_lst.at(i).m_sockfd = sockfd;
					return (i);
				}
			}
			throw std::logic_error("Attempt to add Message while ActiveMessage is full");
		}

		int	add(int sockfd, const request_t &request) // for Responses only
		{
			for (int i = 0; i < m_socket_limit; i++) {
				if (m_messages_lst.at(i).m_sockfd == -1) {
					m_messages_lst.at(i).m_sockfd = sockfd;
					m_messages_lst.at(i).set_request(request);
					return (i);
				}
			}
			throw std::logic_error("Attempt to add Message while ActiveMessage is full");
		}

		int	search(int sockfd) const
		{
			for (int i = 0; i < m_socket_limit; i++) {
				if (m_messages_lst.at(i).m_sockfd == sockfd)
					return (i);
			}
			return (-1);
		}

		void	clear(int sockfd)
		{
			for (int i = 0; i < m_socket_limit; i++) {
				if (m_messages_lst.at(i).m_sockfd == sockfd) {
					m_messages_lst.at(i).clear();
					return ;
				}
			}
		}

		T	&at(int index)
		{
			return (m_messages_lst.at(index)); // throw
		}

	private:

		std::vector<T>	m_messages_lst;
		int				m_socket_limit;

};

#endif
