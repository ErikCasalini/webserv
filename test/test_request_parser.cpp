#include "../src/request_parser.h"
#include "lib_test.h"

// TODO: check with a file if the crlf is translated to a single \n
// std::string buffer = "GET /exemple.com HTTP/1.1\r\n";

using _Request::parse_method;

void test_parse_method()
{
	// correct inputs
	std::string del_buf = "DELETE \r\n";
	size_t pos = 0;
	assert(parse_method(del_buf, pos) == del);
	assert(pos == 7);

	std::string get_buf = "GET \r\n";
	pos = 0;
	assert(parse_method(get_buf, pos) == get);
	assert(pos == 4);

	std::string post_buf = "POST \r\n";
	pos = 0;
	assert(parse_method(post_buf, pos) == post);
	assert(pos == 5);

	// wrong inputs
	std::string no_space = "POST";
	pos = 0;
	try {
		parse_method(no_space, pos);
		assert(false);
	} catch (const std::exception& e) {};

	std::string no_method = "BLABLA \r\n";
	pos = 0;
	try {
		parse_method(no_method, pos);
		assert(false);
	} catch (const std::exception& e) {};

	std::string lowercase = "get \r\n";
	pos = 0;
	try {
		parse_method(lowercase, pos);
		assert(false);
	} catch (const std::exception& e) {};

	std::string space_front = " GET \r\n";
	pos = 0;
	try {
		parse_method(lowercase, pos);
		assert(false);
	} catch (const std::exception& e) {};

	std::string empty = "";
	pos = 0;
	try {
		parse_method(empty, pos);
		assert(false);
	} catch (const std::exception& e) {};
}

int main(void)
{
	test(test_parse_method, "test_parse_method");
	return (0);
}
