#include "parse_uri_utils.h"
#include "http_types.h"
#include <cstdlib>
#include <string>
#include <list>

using std::string;
using std::list;
using std::stringstream;
using std::runtime_error;
using std::invalid_argument;

void	extract_uri_elem(string&uri, string &path, string &querry)
{
	size_t	q_pos = uri.find('?');
	size_t	end = uri.find('#');

	if (q_pos != string::npos && q_pos < end) { // if querry present
		path.assign(uri, 0, q_pos);
		q_pos++; // after '?'
		if (end != string::npos)
			querry.assign(uri, q_pos, end - q_pos); // querry is everything between ? and #
		else
			querry.assign(uri, q_pos); // querry is everything after '?' (or nothing if q_pos = m_request.target.size)
	}
	else
		path.assign(uri, 0, end);
}

list<string>	split_path(const string &path) // assumes path starts with '/'
{
	stringstream	stream_path(path);
	string			temp;
	list<string>	segments;
	bool			is_dir = false;

	if (stream_path.str().at(stream_path.str().size() - 1) == '/')
		is_dir = true;

	std::getline(stream_path, temp, '/');
	if (stream_path.bad())
		throw runtime_error("Internal reading path error");

	while (std::getline(stream_path, temp, '/')) {
		segments.push_back("/");
		if (temp.size())
			segments.push_back(temp);
	}
	if (stream_path.bad())
		throw runtime_error("Internal reading path error");

	if (is_dir)
		segments.push_back("/");

	return (segments);
}

unsigned char	url_decode(const string &url_code)
{
	if (!isxdigit(url_code[0])
		|| !isxdigit(url_code[1]))
		throw invalid_argument("Wrong url encoding format");

	int tmp = std::strtol(url_code.c_str(), NULL, 16);
	if (tmp == 0x0 || tmp == 0x2F || tmp == 0x5C)
		throw invalid_argument("Url expands to forbidden symbol");
	return (static_cast<unsigned char>(tmp));
}

void	decode_segments(list<string> &segments)
{
	list<string>::iterator	it_list = segments.begin();
	size_t					pos;

	for (size_t i = 0; i < segments.size(); i++, it_list++) {
		string				decoded; // new decoded string
		string::iterator	it_seg = it_list->begin(); // original string iterator

		decoded.reserve(it_list->size());
		pos = 0;
		while ((pos = it_list->find('%', pos)) != string::npos) {
			if (it_list->begin() + pos >= it_list->end() - 2) // if '%' is one of the 2 last chars --> bad URL encoding (%A or %)
				throw invalid_argument("Wrong url encoding format");
			decoded.append(*it_list, it_seg - it_list->begin(), pos - (it_seg - it_list->begin()));
			decoded.append(1, url_decode(it_list->substr(pos + 1, 2)));
			pos += 3;
			it_seg = it_list->begin() + pos;
		}
		decoded.append(*it_list, it_seg - it_list->begin());
		*it_list = decoded;
	}
}

string	create_path(list<string> &segments) // assumes segments starts with "/"
{
	string			ret;
	list<string>	new_path;

	while (segments.size()) {
		if (segments.front() == "/") {
			new_path.push_back(segments.front());
			segments.pop_front();
			while (segments.size() && segments.front() == "/")
				segments.pop_front();
		}
		else if (segments.front() == ".") {
			segments.pop_front();
			if (segments.size())
				segments.pop_front();
		}
		else if (segments.front() == "..") {
			if (new_path.size() > 1) {
				new_path.pop_back();
				new_path.pop_back();
			}
			segments.pop_front();
			if (segments.size())
				segments.pop_front();
		}
		else {
			new_path.push_back(segments.front());
			segments.pop_front();
		}
	}

	for (list<string>::iterator it = new_path.begin(); it != new_path.end(); it++) {
		ret.append(*it);
	}

	segments = new_path; // update m_path_segments
	return (ret); // return string format path
}
