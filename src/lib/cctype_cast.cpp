#include <cctype>
#include "../../include/cctype_cast.h"

int to_lowercase(char c)
{
	return (std::tolower(static_cast<unsigned char>(c)));
}

int is_graph(char c)
{
	return (std::isgraph(static_cast<unsigned char>(c)));
}

int is_space(char c)
{
	return (std::isspace(static_cast<unsigned char>(c)));
}

void strltrim(std::string& str, int (*func)(char))
{
	size_t pos = 0;
	while (func(str[pos]))
		++pos;
	str.erase(0, pos);
}

void strrtrim(std::string& str, int (*func)(char))
{
	size_t len = str.length();
	if (len == 0)
		return ;
	size_t pos = len - 1;
	while (pos > 0 && func(str[pos]))
		--pos;
	str.erase(pos + 1, len - pos);
}

void strtrim(std::string& str, int (*func)(char))
{
	strltrim(str, func);
	strrtrim(str, func);
}
