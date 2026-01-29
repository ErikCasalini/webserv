#include <stdexcept>

class CriticalException : public std::runtime_error
{
	public:
	CriticalException(const char* err)
	: std::runtime_error(err) {};
};

class RecoverableException : public std::runtime_error {};

int	socket_ex(int domain, int type, int protocol);
int	bind_ex(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int	listen_ex(int sockfd, int backlog);
int	epoll_create_ex(int size);
int	epoll_ctl_ex(int epfd, int op, int fd, struct epoll_event *event);
int	epoll_wait_ex(int epfd, struct epoll_event *events, int maxevents, int timeout);
