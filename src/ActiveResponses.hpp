#ifndef ACTIVERESPONSES_HPP
# define ACTIVERESPONSES_HPP

#include <vector>

class	ActiveResponses
{
	public:

								ActiveResponses(int socket_limit);
								~ActiveResponses(void);
		// Response				&add(int sockfd);
		void					clear(int sockfd);

	private:

		// std::vector<Response>	m_response_lst;
		int						m_socket_limit;

};

#endif
