#include "response_utils.h"
#include "general_utils.h"
#include "http_types.h"
#include <fstream>
#include <vector>
#include <list>
#include <sys/types.h>
#include <dirent.h>
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

const location_t	*find_location(const list<string> &path, const vector<location_t> &locations) // assume location path starts with '/'
{
	vector<location_t>::const_iterator	it_loc = locations.begin();
	vector<location_t>::const_iterator	ret = locations.end();

	if (locations.size() == 0)
		throw (bad_location("No locations set inside server"));

	for (int best_match = 0, match_rate; it_loc < locations.end(); it_loc++) {
		if (it_loc->exact_match) {
			if (is_exact_match(path, it_loc->path))
				return (&(*it_loc));
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

	return (&(*ret));
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

void	set_body_headers(headers_t &headers, const string &body, const string &file_name, const config_t & config)
{
	headers.content_length = body.size();
	if (file_name.rfind(".") == std::string::npos
		|| config.http.types.count(file_name.substr(file_name.rfind(".") + 1)) == 0)
		headers.content_type = config.http.default_type;
	else
		headers.content_type = config.http.types.at(file_name.substr(file_name.rfind(".") + 1));
}

bool	is_bad_method(method_t method, const vector<method_t> &limit_except)
{
	for (vector<method_t>::const_iterator it = limit_except.begin(); it != limit_except.end(); it++) {
		if (method == *it)
			return (false);
	}
	return (true);
}

status_t	generate_indexing(const std::string &directory, std::string &body, const std::string &uri_path)
{
	std::string		indexing;
	DIR				*dir_stream;
	struct dirent	*dir_data;

	errno = 0;
	dir_stream = opendir(directory.c_str());
	if (dir_stream == NULL) {
		if (errno == EACCES)
			return (forbidden);
		return (internal_err);
	}

	while ((dir_data = readdir(dir_stream))) {
		if (errno != 0)
			return (internal_err);
		indexing.append("<a href=\"");
		indexing.append(dir_data->d_name);
		if (dir_data->d_type == DT_DIR)
			indexing.append("/");
		indexing.append("\">");
		indexing.append(dir_data->d_name);
		if (dir_data->d_type == DT_DIR)
			indexing.append("/");
		indexing.append("</a>\n");
	}
	body = "<html>\n<body>\n<h1>Index of ";
	body.append(uri_path);
	body.append("</h1><hr><pre>");
	body.append(indexing);
	body.append("</pre><hr></body>\n</html>");
	closedir(dir_stream);
	return (ok);
}
