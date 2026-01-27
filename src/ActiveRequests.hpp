#ifndef ACTIVEREQUESTS_HPP
# define ACTIVEREQUESTS_HPP

# include "request_parser.h"
# include <vector>

class	ActiveRequests
{
	public:

								ActiveRequests(int socket_limit);
								~ActiveRequests(void) {}
		Request					&add(int sockfd);
		void					remove(int sockfd);

	private:

		std::vector<Request>	m_request_lst;
		int						m_socket_limit;

};

#endif
