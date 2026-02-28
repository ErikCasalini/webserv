#ifndef MAIN_LOOP_HPP
# define MAIN_LOOP_HPP

# include "Config.h"

int		main_server_loop(config_t &config);
void	reap_children(void);

#endif
