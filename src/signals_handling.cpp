#include <csignal>
#include <cstdlib>
#include <stdexcept>

volatile sig_atomic_t int_signal = 0;

void int_handler(int sig)
{
	if (sig == SIGINT)
		int_signal = 1;
}

void	set_signal_handlers(void)
{
	if (std::signal(SIGINT, int_handler) == SIG_ERR)
		throw std::runtime_error("Signal() failed");
	if (std::signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		throw std::runtime_error("Signal() failed");
}
