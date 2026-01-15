#include <string.h>
#include <exception>
#include <sys/types.h>
#include <sys/socket.h>
#include "../../include/c_network_exception.h"

int socket_ex(int domain, int type, int protocol)
{
	int fd = socket(domain, type, protocol);
	if (fd == -1)
		throw critical_exception(strerror(errno));
	return (fd);
}

int	bind_ex(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int ret = bind(sockfd, addr, addrlen);
	if (ret < 0)
		throw critical_exception(strerror(errno));
	return (ret);
}

int listen_ex(int sockfd, int backlog)
{
	int ret = listen(sockfd, backlog);
	if (ret < 0)
		throw critical_exception(strerror(errno));
	return (ret);
}
