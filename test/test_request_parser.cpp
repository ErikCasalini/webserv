#include "../src/request_parser.h"
#include "lib_test.h"

// TODO: check with a file if the crlf is translated to a single \n
// std::string buffer = "GET /exemple.com HTTP/1.1\r\n";

using namespace _Request;

void test_parse_method()
{
	// correct inputs
	std::string del_buf = "DELETE";
	size_t pos = 0;
	assert(parse_method(del_buf, pos) == del);
	assert(pos == 6);

	std::string get_buf = "GET";
	pos = 0;
	assert(parse_method(get_buf, pos) == get);
	assert(pos == 3);

	std::string post_buf = "POST";
	pos = 0;
	assert(parse_method(post_buf, pos) == post);
	assert(pos == 4);

	// wrong inputs
	std::string no_method = "BLABLA \r\n";
	pos = 0;
	try {
		parse_method(no_method, pos);
		assert(false);
	} catch (const Request::BadRequest& e) {};

	std::string lowercase = "get";
	pos = 0;
	try {
		parse_method(lowercase, pos);
		assert(false);
	} catch (const Request::BadRequest& e) {};

	std::string space_front = " GET";
	pos = 0;
	try {
		parse_method(lowercase, pos);
		assert(false);
	} catch (const Request::BadRequest& e) {};

	std::string empty = "";
	pos = 0;
	try {
		parse_method(empty, pos);
		assert(false);
	} catch (const Request::BadRequest& e) {};
}

void test_parse_target()
{
	// correct inputs
	// Parse_target include all text before the first space,
	// tests must include a space at the end of the string.
	std::string target = "/ex ";
	size_t pos = 0;
	assert(parse_target(target, pos) == "/ex");
	assert(pos == 3);

	std::string target_2 = "/ex/ex.html ";
	pos = 0;
	assert(parse_target(target_2, pos) == "/ex/ex.html");
	assert(pos == 11);

	// wrong inputs
	std::string end_of_buffer = "";
	pos = 0;
	try {
		parse_target(end_of_buffer, pos);
		assert(false);
	} catch (const Request::BadRequest& e) {};
}

int main(void)
{
	test(test_parse_method, "test_parse_method");
	test(test_parse_target, "test_parse_target");
	return (0);
}
