#include "general_utils.h"
#include "http_types.h"
#include "response_utils.h"
#include <list>
#include <cerrno>
#include <ctime>
#include <sys/stat.h>

using std::list;
using std::string;

bool	is_exact_match(const list<string> &path, const list<string> &location)
{
	list<string>::const_iterator	it_path = path.begin();
	list<string>::const_iterator	it_loc = location.begin();

	if (path.size() == 0 || location.size() == 0)
		throw (bad_location("Empty path or location segments"));

	if (path.size() != location.size())
		return (false);

	for (size_t i = 0; i < path.size(); i++, it_path++, it_loc++) {
		if (*it_path != *it_loc)
			return (false);
	}
	return (true);
}

file_stat	get_file_type(const string &file_name)
{
	struct stat	target_stats;

	errno = 0;
	stat(file_name.c_str(), &target_stats);
	switch (errno) {
		case 0:
			break ;
		case ENOENT:
		case ENOTDIR:
			return (nonexistent);
		case EACCES:
			return (bad_perms);
		default:
			return (error);
	}

	if (S_ISDIR(target_stats.st_mode & S_IFMT))
		return (dir);
	else
		return (file);
}

string	get_date(std::time_t time)
{
	const size_t buf_len = sizeof("ddd, nn mmm yyyy hh:mm:ss GMT");
	char buf[buf_len];
	const char *format = "%a, %d %b %Y %H:%M:%S GMT";

	std::strftime(buf, buf_len, format, std::localtime(&time));

	return (buf);
}
