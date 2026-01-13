#include <stdexcept>

class critical_exception : public std::runtime_error {};

class recoverable_exception : public std::runtime_error {};

int socket_ex(int domain, int type, int protocol);
