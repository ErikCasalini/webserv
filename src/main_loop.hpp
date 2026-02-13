#ifndef MAIN_LOOP_HPP
# define MAIN_LOOP_HPP

# include <vector>
# include <utility>
# include <string>
# include <sys/epoll.h>
# include "Config.h"

int	main_server_loop(config_t &config);

#endif
