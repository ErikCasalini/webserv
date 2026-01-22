#include "../src/request_parser.h"
#include "../src/http_types.h"
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

void test_parse_protocol()
{
	// correct inputs
	std::string o = "HTTP/1.0";
	size_t pos = 0;
	assert(parse_protocol(o, pos) == one);
	assert(pos == 8);

	std::string o_o = "HTTP/1.1";
	pos = 0;
	assert(parse_protocol(o_o, pos) == one_one);
	assert(pos == 8);

	// wrong inputs
	std::string end_of_buffer = "";
	pos = 0;
	try {
		parse_protocol(end_of_buffer, pos);
		assert(false);
	} catch (const Request::BadRequest& e) {};

	std::string invalid = "HTTP/9.9";
	pos = 0;
	try {
		parse_protocol(invalid, pos);
		assert(false);
	} catch (const Request::BadRequest& e) {};

	std::string lowercase = "http/1.1";
	pos = 0;
	try {
		parse_protocol(lowercase, pos);
		assert(false);
	} catch (const Request::BadRequest& e) {};
}

void test_extract_key()
{
	// correct inputs
	std::string host = "host:";
	size_t pos = 0;
	assert(extract_key(host, pos) == "host");
	assert(pos == 5);

	std::string content_encoding = "content-encoding: ";
	pos = 0;
	assert(extract_key(content_encoding, pos) == "content-encoding");
	assert(pos == 17);

	std::string upper = "HOST:";
	pos = 0;
	assert(extract_key(upper, pos) == "host");
	assert(pos == 5);

	std::string mixed = "Host:";
	pos = 0;
	assert(extract_key(mixed, pos) == "host");
	assert(pos == 5);

	// wrong inputs
	std::string leading_spaces = " host:";
	pos = 0;
	try {
		extract_key(leading_spaces, pos);
		assert(false);
	} catch (const Request::BadRequest& e) {};

	std::string trailing_spaces = "host :";
	pos = 0;
	try {
		extract_key(trailing_spaces, pos);
		assert(false);
	} catch (const Request::BadRequest& e) {};

	std::string end_of_buffer = "";
	pos = 0;
	try {
		extract_key(end_of_buffer, pos);
		assert(false);
	} catch (const Request::BadRequest& e) {};

	std::string empty = ":";
	pos = 0;
	try {
		extract_key(empty, pos);
		assert(false);
	} catch (const Request::BadRequest& e) {};

	std::string empty_2 = " :";
	pos = 0;
	try {
		extract_key(empty_2, pos);
		assert(false);
	} catch (const Request::BadRequest& e) {};

}

void test_extract_values()
{
	// correct inputs
	std::string empty = "\r\n";
	size_t pos = 0;
	assert(extract_values(empty, pos)[0] == "");
	assert(pos == 2);

	std::string lenght = "234\r\n";
	pos = 0;
	assert(extract_values(lenght, pos)[0] == "234");
	assert(pos == 5);

	std::string lenght_lead = " 234\r\n";
	pos = 0;
	assert(extract_values(lenght_lead, pos)[0] == " 234");
	assert(pos == 6);

	std::string list = " 234 ,  4 \r\n";
	pos = 0;
	assert(extract_values(list, pos)[0] == " 234 ,  4 ");
	assert(pos == 12);

	std::string list_empty = " 234 ,  , 4 \r\n";
	pos = 0;
	assert(extract_values(list_empty, pos)[0] == " 234 ,  , 4 ");
	assert(pos == 14);

	// wrong inputs
	std::string no_crlf = "234";
	pos = 0;
	try {
		extract_values(no_crlf, pos);
		assert(false);
	} catch (const Request::BadRequest& e) {};

	std::string empty_no_crlf = "";
	pos = 0;
	try {
		extract_values(empty_no_crlf, pos);
		assert(false);
	} catch (const Request::BadRequest& e) {};
}

int main(void)
{
	// start line
	test(test_parse_method, "test_parse_method");
	test(test_parse_target, "test_parse_target");
	test(test_parse_protocol, "test_parse_protocol");
	// headers
	test(test_extract_key, "test_extract_key");
	test(test_extract_values, "test_extract_values");
	return (0);
}
