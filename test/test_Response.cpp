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

	segments = split_path("/");
	it = segments.begin();
	assert((segments.size() == 1));
	assert((*it == ""));

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

void	test_url_decode(void)
{
	std::string	url_code;

	url_code = "12";
	assert((url_decode(url_code) == 18));

	url_code = "02";
	assert((url_decode(url_code) == 2));

	url_code = "FF";
	assert((url_decode(url_code) == 255));

	url_code = "aa";
	assert((url_decode(url_code) == 170));

	try {
		url_code = "00";
		url_decode(url_code);
		assert((false));
	} catch (std::invalid_argument &e) {}

	try {
		url_code = "2F";
		url_decode(url_code);
		assert((false));
	} catch (std::invalid_argument &e) {}

	try {
		url_code = "5C";
		url_decode(url_code);
		assert((false));
	} catch (std::invalid_argument &e) {}

	try {
		url_code = "-5";
		url_decode(url_code);
		assert((false));
	} catch (std::invalid_argument &e) {}

	try {
		url_code = "F%";
		url_decode(url_code);
		assert((false));
	} catch (std::invalid_argument &e) {}

}

void	test_decode_segments(void)
{
	std::list<std::string>				segments;
	std::list<std::string>::iterator	it;

	segments.push_back("src");
	segments.push_back("src%75"); // u
	segments.push_back("%21%24"); // ! $
	segments.push_back("src%2ASRC%2E"); // * .
	segments.push_back("src%2ASRC%2Esrc"); // * .
	segments.push_back("%30"); // 0
	segments.push_back("");
	segments.push_back("");
	decode_segments(segments);
	it = segments.begin();
	assert((segments.size() == 8));
	assert((*it == "src"));
	assert((*(++it) == "srcu"));
	assert((*(++it) == "!$"));
	assert((*(++it) == "src*SRC."));
	assert((*(++it) == "src*SRC.src"));
	assert((*(++it) == "0"));
	assert((*(++it) == ""));
	assert((*(++it) == ""));

	segments.clear();
	segments.push_back("%");
	try {
		decode_segments(segments);
		assert(false);
	} catch (std::invalid_argument &e) {}

	segments.clear();
	segments.push_back("%0");
	try {
		decode_segments(segments);
		assert(false);
	} catch (std::invalid_argument &e) {}

	segments.clear();
	segments.push_back("src%");
	try {
		decode_segments(segments);
		assert(false);
	} catch (std::invalid_argument &e) {}

	segments.clear();
	segments.push_back("src%1");
	try {
		decode_segments(segments);
		assert(false);
	} catch (std::invalid_argument &e) {}

	segments.clear();
	segments.push_back("%1g");
	try {
		decode_segments(segments);
		assert(false);
	} catch (std::invalid_argument &e) {}

	segments.clear();
	segments.push_back("%xsrc");
	try {
		decode_segments(segments);
		assert(false);
	} catch (std::invalid_argument &e) {}

	segments.clear();
	segments.push_back("src%FF%%%%%%");
	try {
		decode_segments(segments);
		assert(false);
	} catch (std::invalid_argument &e) {}

	segments.clear();
	segments.push_back("0%");
	try {
		decode_segments(segments);
		assert(false);
	} catch (std::invalid_argument &e) {}
}

int	main(void)
{
	test(test_extract_uri_elem, "test_extract_uri_elem");
	test(test_split_path, "test_split_path");
	test(test_url_decode, "test_url_decode");
	test(test_decode_segments, "test_decode_segments");
	return (0);
}
