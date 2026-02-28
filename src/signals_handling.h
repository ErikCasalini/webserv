#ifndef SIGNAL_HANDLING_H
# define SIGNAL_HANDLING_H

#include <csignal>

extern volatile sig_atomic_t int_signal;

void	set_signal_handlers(void);

#endif
