#ifndef GENERAL_UTILS_H
# define GENERAL_UTILS_H

# include "http_types.h"
# include <string>
# include <list>
# include <ctime>

bool			is_exact_match(const std::list<std::string> &path, const std::list<std::string> &location);
file_stat		get_file_type(const std::string &file_name);
std::string		get_date(std::time_t time);

#endif
