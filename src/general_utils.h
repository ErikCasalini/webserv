#ifndef GENERAL_UTILS_H
# define GENRAL_UTILS_H

# include "http_types.h"
# include <string>
# include <list>

bool		is_exact_match(const std::list<std::string> &path, const std::list<std::string> &location);
file_stat	get_file_type(const std::string &file_name);

#endif
