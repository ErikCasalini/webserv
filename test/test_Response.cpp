#include "../src/Response.hpp"
#include "lib_test.h"

using namespace _Response;

void	test_extract_uri_elem(void)
{
	std::string	uri("/src?querry");
	std::string	path;
	std::string	querry;
	extract_uri_elem(uri, path, querry);
	assert((path == "/src"));
	assert((querry == "querry"));

	uri = "/src?querry#";
	path.clear();
	querry.clear();
	extract_uri_elem(uri, path, querry);
	assert((path == "/src"));
	assert((querry == "querry"));

	uri = "/src?querry#o";
	path.clear();
	querry.clear();
	extract_uri_elem(uri, path, querry);
	assert((path == "/src"));
	assert((querry == "querry"));

	uri = "/sr/c/?que#rry#";
	path.clear();
	querry.clear();
	extract_uri_elem(uri, path, querry);
	assert((path == "/sr/c/"));
	assert((querry == "que"));

	uri = "/src?";
	path.clear();
	querry.clear();
	extract_uri_elem(uri, path, querry);
	assert((path == "/src"));
	assert((querry == ""));

	uri = "/src####";
	path.clear();
	querry.clear();
	extract_uri_elem(uri, path, querry);
	assert((path == "/src"));
	assert((querry == ""));

	uri = "/?###";
	path.clear();
	querry.clear();
	extract_uri_elem(uri, path, querry);
	assert((path == "/"));
	assert((querry == ""));

	uri = "/###?ok";
	path.clear();
	querry.clear();
	extract_uri_elem(uri, path, querry);
	assert((path == "/"));
	assert((querry == ""));

	uri = "/?ok#?ok";
	path.clear();
	querry.clear();
	extract_uri_elem(uri, path, querry);
	assert((path == "/"));
	assert((querry == "ok"));

	uri = "/oui????";
	path.clear();
	querry.clear();
	extract_uri_elem(uri, path, querry);
	assert((path == "/oui"));
	assert((querry == "???"));

}

int	main(void)
{
	test(test_extract_uri_elem, "test_extract_uri_elem");
	return (0);
}
