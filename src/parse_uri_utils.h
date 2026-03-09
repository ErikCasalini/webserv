#ifndef PARSE_URI_UTILS_H
# define PARSE_URI_UTILS_H

# include <string>
# include <list>

void					extract_uri_elem(std::string&uri, std::string &path, std::string &querry);
std::list<std::string>	split_path(const std::string &path);
unsigned char			url_decode(const std::string &url_code);
void					decode_segments(std::list<std::string> &segments);
std::string				create_path(std::list<std::string> &segments);

#endif
