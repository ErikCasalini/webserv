#include "../src/Response.hpp"
#include "lib_test.h"
#include <list>

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

void	test_split_path(void)
{
	std::list<std::string>				segments;
	std::list<std::string>::iterator	it;
	segments = split_path("/test/");
	it = segments.begin();
	assert((segments.size() == 2));
	assert((*it == "test"));
	assert((*++it == ""));

	segments = split_path("/test");
	it = segments.begin();
	assert((segments.size() == 1));
	assert((*it == "test"));

	segments = split_path("/test/deux/");
	it = segments.begin();
	assert((segments.size() == 3));
	assert((*it == "test"));
	assert((*(++it) == "deux"));
	assert((*(++it) == ""));

	segments = split_path("/test///");
	it = segments.begin();
	assert((segments.size() == 4));
	assert((*(it) == "test"));
	assert((*(++it) == ""));
	assert((*(++it) == ""));
	assert((*(++it) == ""));

	segments = split_path("/test///./..///");
	assert((segments.size() == 8));
	it = segments.begin();
	assert((*(it) == "test"));
	assert((*(++it) == ""));
	assert((*(++it) == ""));
	assert((*(++it) == "."));
	assert((*(++it) == ".."));
	assert((*(++it) == ""));
	assert((*(++it) == ""));
	assert((*(++it) == ""));

}

int	main(void)
{
	test(test_extract_uri_elem, "test_extract_uri_elem");
	test(test_split_path, "test_split_path");
	return (0);
}
