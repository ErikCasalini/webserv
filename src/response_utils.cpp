#include "response_utils.h"
#include "general_utils.h"
#include "http_types.h"
#include <fstream>
#include <vector>
#include <list>
#include "Config.h"

using std::vector;
using std::list;
using std::string;
using std::ifstream;

int	evaluate_path_matching(const list<string> &path, const list<string> &location)
{
	list<string>::const_iterator	it_path = path.begin();
	list<string>::const_iterator	it_loc = location.begin();
	list<string>::const_iterator	loc_end = location.end();
	int								match_rate = 0;

	if (path.size() == 0 || location.size() == 0)
		throw (bad_location("Empty path or location segments"));
	if (path.size() < location.size())
		return (0);

	for (; it_loc != loc_end; it_path++, it_loc++) {
		if (*it_path != *it_loc)
			return (0);
		match_rate++;
	}
	return (match_rate);
}

const location_t	&find_location(const list<string> &path, const vector<location_t> &locations) // assume location path starts with '/'
{
	vector<location_t>::const_iterator	it_loc = locations.begin();
	vector<location_t>::const_iterator	ret = locations.end();

	for (int best_match = 0, match_rate; it_loc < locations.end(); it_loc++) {
		if (it_loc->exact_match) {
			if (is_exact_match(path, it_loc->path))
				return (*it_loc);
		}
		else {
			match_rate = evaluate_path_matching(path, it_loc->path);
			if (match_rate > best_match) {
				best_match = match_rate;
				ret = it_loc;
			}
		}
	}
	if (ret == locations.end())
		throw (bad_location("Uri matchs no locations"));

	return (*ret);
}

status_t	read_file_to_body(const string &file_name, string &body)
{
	switch (get_file_type(file_name)) {
		case nonexistent:
			return (not_found);
		case bad_perms:
		case dir:
			return (forbidden);
		case error:
			return (internal_err);
		default: //file
			break;
	}

	if (access(file_name.c_str(), R_OK) == -1)
		return (forbidden);

	string		temp;
	ifstream	file(file_name.c_str());

	if (file.fail())
		return (internal_err);

	while (std::getline(file, temp))
		body.append(temp);

	if (file.bad())
		return (internal_err);
	else
		return (ok);
}

void	set_body_headers(headers_t &headers, string body, string file_name)
{
	(void)file_name;
	headers.content_length = body.size();
	headers.content_type = "text/html"; // remplacer par fonction qui cherche (si trouve pas -> bit stream)
}

bool	is_bad_method(method_t method, vector<method_t> &limit_except)
{
	for (vector<method_t>::const_iterator it = limit_except.begin(); it != limit_except.end(); it++) {
		if (method == *it)
			return (false);
	}
	return (true);
}
