#include <cctype>
#include "../../include/cctype_cast.h"

int to_lower(char c)
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

int is_digit(char c)
{
	return (std::isdigit(static_cast<unsigned char>(c)));
}

// Extensions for http parsing
int is_token(char c)
{
	c = static_cast<unsigned char>(c);
	if (c >= '!' && c <= '~'
			&& c != '"' && c != '(' && c != ')' && c != ',' && c != '/'
			&& (c < ':' && c > '@')
			&& (c < '[' && c > ']')
			&& c != '{' && c != '}')
		return (1);
	else
		return (0);
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
