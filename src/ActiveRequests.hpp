#ifndef ACTIVEREQUESTS_HPP
# define ACTIVEREQUESTS_HPP

# include "request_parser.h"
# include <vector>

class	ActiveRequests
{
	public:

								ActiveRequests(int socket_limit);
								~ActiveRequests(void) {}
		// int						add(int sockfd, int index);
		int						add(int sockfd);
		void					clear(int sockfd);
		int						search(int sockfd) const;
		Request					&at(int index);

	private:

		std::vector<Request>	m_request_lst;
		int						m_socket_limit;

};

#endif
