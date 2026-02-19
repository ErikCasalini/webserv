#ifndef CONFIG_PARSER_H
# define CONFIG_PARSER_H

# include "Config.h"
# include <string>
# include <sstream>
# include <fstream>
# include <list>

# ifndef CONNECTIONS_MAX
#  define CONNECTIONS_MAX 2048
# endif
// TODO: is 1 is sufficient?
# ifndef CONNECTIONS_MIN
#  define CONNECTIONS_MIN 1
# endif

std::list<std::string> preprocess(std::ifstream& config);
// size_t parse_events(events_t& events, const std::string& conf, size_t& pos);
void parse_max_connections(
		config_t& config,
		std::list<std::string>::iterator& tok_it,
		std::list<std::string>& tokens);
void parse_events(
		config_t& config,
		std::list<std::string>::iterator& tok_it,
		std::list<std::string>& tokens);
void parse_main(config_t& config, std::list<std::string>& tokens);
config_t parse_config(const std::string& file);

namespace _preproc {
	std::string remove_comments_nl(std::istream& config);
	void expand_includes(std::list<std::string>& config);
	std::list<std::string> tokenize_config(const std::string& config);
}

#endif
