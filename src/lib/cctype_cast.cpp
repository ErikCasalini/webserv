#include <cctype>

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
