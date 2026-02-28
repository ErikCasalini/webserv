#ifndef MAIN_LOOP_HPP
# define MAIN_LOOP_HPP

# include "Config.h"

void	main_server_loop(config_t &config);
void	reap_children(void);

#endif
