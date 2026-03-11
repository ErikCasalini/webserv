#ifndef RESPONSE_UTILS_H
# define RESPONSE_UTILS_H

# include "http_types.h"
# include <stdexcept>
# include <vector>
# include <list>
# include <string>

using std::string;
using std::list;
using std::vector;

class	bad_location : public std::runtime_error
	{
		public:
			bad_location(const std::string &str)
			: std::runtime_error(str)
			{};
	};

class	internal_error : public std::runtime_error
{
	public:
		internal_error(const std::string &str)
		: std::runtime_error(str)
		{};
};

int					evaluate_path_matching(const list<string> &path, const list<string> &location);
const location_t	&find_location(const list<string> &path, const vector<location_t> &locations);
status_t			read_file_to_body(const string &file_name, string &body);
void				set_body_headers(headers_t &headers, string body, string file_name);
bool				is_bad_method(method_t method, vector<method_t> &limit_except);
status_t			generate_indexing(const std::string &directory, std::string &body, const std::string &path);

#endif
