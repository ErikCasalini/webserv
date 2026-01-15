#include <stdexcept>

class critical_exception : public std::runtime_error
{
	public:
	critical_exception(const char* err)
	: std::runtime_error(err) {};
};

class recoverable_exception : public std::runtime_error {};

int socket_ex(int domain, int type, int protocol);
