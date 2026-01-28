#include "ActiveResponses.hpp"
#include <vector>

ActiveResponses::ActiveResponses(int socket_limit)
: m_socket_limit(socket_limit)
{
	// m_response_lst.resize(socket_limit);
}

ActiveResponses::~ActiveResponses(void)
{}

// Response	&ActiveResponses::add(int sockfd)
// {
//
// }

void	ActiveResponses::clear(int sockfd)
{

}
