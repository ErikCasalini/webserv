#ifndef MAIN_LOOP_HPP
# define MAIN_LOOP_HPP

# include <vector>
# include <utility>
# include <string>
# include <sys/epoll.h>

typedef std::vector<epoll_event> vec_events;

typedef struct	temp_config
{
	std::vector<std::pair<const std::string, const short> > interfaces;
	int	socket_limit;
}				temp_config;

typedef enum	sock_type
{
	active,
	passive
}				sock_type;

int	main_server_loop(temp_config &config);

#endif
