#include <exception>
#include <sys/types.h>
#include <sys/socket.h>
#include "../../include/c_network_exception.h"

int socket_ex(int domain, int type, int protocol)
{
	int fd = socket(domain, type, protocol);
	if (fd == -1)
		throw std::exception();
	return (fd);
}
