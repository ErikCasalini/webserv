#include <string.h>
#include <exception>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <cerrno>
#include "../../include/c_network_exception.h"

int socket_ex(int domain, int type, int protocol)
{
	int fd = socket(domain, type, protocol);
	if (fd == -1)
		throw CriticalException(strerror(errno));
	return (fd);
}

int	bind_ex(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int ret = bind(sockfd, addr, addrlen);
	if (ret < 0)
		throw CriticalException(strerror(errno));
	return (ret);
}

int listen_ex(int sockfd, int backlog)
{
	int ret = listen(sockfd, backlog);
	if (ret < 0)
		throw CriticalException(strerror(errno));
	return (ret);
}

int epoll_create_ex(int size)
{
	int ret = epoll_create(size);
	if (ret < 0)
		throw CriticalException(strerror(errno));
	return (ret);
}

int epoll_ctl_ex(int epfd, int op, int fd, struct epoll_event *event)
{
	int ret = epoll_ctl(epfd, op, fd, event);
	if (ret < 0)
		throw CriticalException(strerror(errno));
	return (ret);
}

int epoll_wait_ex(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	int ret = epoll_wait(epfd, events, maxevents, timeout);
	if (errno != 0 && errno != EINTR)
		throw CriticalException(strerror(errno));
	return (ret);
}
