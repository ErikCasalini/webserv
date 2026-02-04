#include "../src/Response.hpp"
#include "lib_test.h"

void	test_extract_uri_elem(void)
{
	std::string	uri("/src?querry");
	std::string	path;
	std::string	querry;
	extract_uri_elem(uri, path, querry);
	assert((path == "/src"));
	assert((querry == "querry"));
}

int	main(void)
{
	test(test_extract_uri_elem, "test_extract_uri_elem");
	return (0);
}
