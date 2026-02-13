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

		int	add(socket_t *socket) // Must be the socket_t present in Sockets (for correct address)
		{
			if (socket == NULL)
				throw std::logic_error("Attempt to add invalid socket");
			for (int i = 0; i < m_socket_limit; i++) {
				if (m_messages_lst.at(i).m_socket == NULL) {
					m_messages_lst.at(i).m_socket = socket;
					return (i);
				}
			}
			throw std::logic_error("Attempt to add Message while ActiveMessage is full");
		}

		int	add(socket_t *socket, const request_t &request) // for Responses only
		{
			if (socket == NULL)
				throw std::logic_error("Attempt to add invalid socket");
			for (int i = 0; i < m_socket_limit; i++) {
				if (m_messages_lst.at(i).m_socket == NULL) {
					m_messages_lst.at(i).m_socket = socket;
					m_messages_lst.at(i).set_request(request);
					return (i);
				}
			}
			throw std::logic_error("Attempt to add Message while ActiveMessage is full");
		}

		int	search(socket_t *socket) const
		{
			if (socket == NULL)
				throw std::logic_error("Attempt to search invalid socket");
			for (int i = 0; i < m_socket_limit; i++) {
				if (m_messages_lst.at(i).m_socket == socket)
					return (i);
			}
			return (-1);
		}

		void	clear(socket_t *socket)
		{
			if (socket == NULL)
				throw std::logic_error("Attempt to clear invalid socket");
			for (int i = 0; i < m_socket_limit; i++) {
				if (m_messages_lst.at(i).m_socket == socket) {
					m_messages_lst.at(i).clear();
					return ;
				}
			}
			throw std::logic_error("Attempt to clear untracked socket");
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
