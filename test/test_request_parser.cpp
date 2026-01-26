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
	assert((parse_method(del_buf, pos) == del));
	assert(pos == 6);

	std::string get_buf = "GET";
	pos = 0;
	assert((parse_method(get_buf, pos) == get));
	assert(pos == 3);

	std::string post_buf = "POST";
	pos = 0;
	assert((parse_method(post_buf, pos) == post));
	assert(pos == 4);

	// wrong inputs
	std::string no_method = "BLABLA \r\n";
	pos = 0;
	try {
		parse_method(no_method, pos);
		assert((false && "no_method"));
	} catch (const Request::BadRequest& e) {};

	std::string lowercase = "get";
	pos = 0;
	try {
		parse_method(lowercase, pos);
		assert((false && "lowercase"));
	} catch (const Request::BadRequest& e) {};

	std::string space_front = " GET";
	pos = 0;
	try {
		parse_method(lowercase, pos);
		assert((false && "lowercase"));
	} catch (const Request::BadRequest& e) {};

	std::string empty = "";
	pos = 0;
	try {
		parse_method(empty, pos);
		assert((false && "empty"));
	} catch (const Request::BadRequest& e) {};
}

void test_parse_target()
{
	// correct inputs
	// Parse_target include all text before the first space,
	// tests must include a space at the end of the string.
	std::string target = "/ex ";
	size_t pos = 0;
	assert((parse_target(target, pos) == "/ex"));
	assert(pos == 3);

	std::string target_2 = "/ex/ex.html ";
	pos = 0;
	assert((parse_target(target_2, pos) == "/ex/ex.html"));
	assert(pos == 11);

	// wrong inputs
	std::string end_of_buffer = "";
	pos = 0;
	try {
		parse_target(end_of_buffer, pos);
		assert((false && "end_of_buffer"));
	} catch (const Request::BadRequest& e) {};
}

void test_parse_protocol()
{
	// correct inputs
	std::string o = "HTTP/1.0";
	size_t pos = 0;
	assert((parse_protocol(o, pos) == one));
	assert(pos == 8);

	std::string o_o = "HTTP/1.1";
	pos = 0;
	assert((parse_protocol(o_o, pos) == one_one));
	assert(pos == 8);

	// wrong inputs
	std::string end_of_buffer = "";
	pos = 0;
	try {
		parse_protocol(end_of_buffer, pos);
		assert((false && "end_of_buffer"));
	} catch (const Request::BadRequest& e) {};

	std::string invalid = "HTTP/9.9";
	pos = 0;
	try {
		parse_protocol(invalid, pos);
		assert((false && "invalid"));
	} catch (const Request::BadRequest& e) {};

	std::string lowercase = "http/1.1";
	pos = 0;
	try {
		parse_protocol(lowercase, pos);
		assert((false && "lowercase"));
	} catch (const Request::BadRequest& e) {};
}

void test_extract_key()
{
	// correct inputs
	std::string host = "host:";
	size_t pos = 0;
	assert((extract_key(host, pos) == "host"));
	assert(pos == 5);

	std::string content_encoding = "content-encoding: ";
	pos = 0;
	assert((extract_key(content_encoding, pos) == "content-encoding"));
	assert(pos == 17);

	std::string upper = "HOST:";
	pos = 0;
	assert((extract_key(upper, pos) == "host"));
	assert(pos == 5);

	std::string mixed = "Host:";
	pos = 0;
	assert((extract_key(mixed, pos) == "host"));
	assert(pos == 5);

	// wrong inputs
	std::string leading_spaces = " host:";
	pos = 0;
	try {
		extract_key(leading_spaces, pos);
		assert((false && "leading_spaces"));
	} catch (const Request::BadRequest& e) {};

	std::string trailing_spaces = "host :";
	pos = 0;
	try {
		extract_key(trailing_spaces, pos);
		assert((false && "trailing_spaces"));
	} catch (const Request::BadRequest& e) {};

	std::string end_of_buffer = "";
	pos = 0;
	try {
		extract_key(end_of_buffer, pos);
		assert((false && "end_of_buffer"));
	} catch (const Request::BadRequest& e) {};

	std::string empty = ":";
	pos = 0;
	try {
		extract_key(empty, pos);
		assert((false && "empty"));
	} catch (const Request::BadRequest& e) {};

	std::string empty_2 = " :";
	pos = 0;
	try {
		extract_key(empty_2, pos);
		assert((false && "empty_2"));
	} catch (const Request::BadRequest& e) {};

}

void test_extract_values()
{
	// correct inputs
	std::string empty = "\r\n";
	size_t pos = 0;
	assert((extract_values(empty, pos) == ""));
	assert(pos == 2);

	std::string lenght = "234\r\n";
	pos = 0;
	assert((extract_values(lenght, pos) == "234"));
	assert(pos == 5);

	std::string lenght_lead = " 234\r\n";
	pos = 0;
	assert((extract_values(lenght_lead, pos) == " 234"));
	assert(pos == 6);

	std::string list = " 234 ,  4 \r\n";
	pos = 0;
	assert((extract_values(list, pos) == " 234 ,  4 "));
	assert(pos == 12);

	std::string list_empty = " 234 ,  , 4 \r\n";
	pos = 0;
	assert((extract_values(list_empty, pos) == " 234 ,  , 4 "));
	assert(pos == 14);

	// TODO: is it correct according to the rfc? (space at start of line, remove \r)
	std::string cr = "laskhdasdklhqwdlkwjhqd\rlkashdlkasdh  , 4 \r\n";
	pos = 0;
	assert((extract_values(cr, pos) == "laskhdasdklhqwdlkwjhqd\rlkashdlkasdh  , 4 "));
	assert(pos == 43);

	// wrong inputs
	std::string no_crlf = "234";
	pos = 0;
	try {
		extract_values(no_crlf, pos);
		assert((false && "no crlf"));
	} catch (const Request::BadRequest& e) {};

	std::string empty_no_crlf = "";
	pos = 0;
	try {
		extract_values(empty_no_crlf, pos);
		assert((false && "empty_no_crlf"));
	} catch (const Request::BadRequest& e) {};
}

void test_extract_headers()
{
	// correct inputs
	std::string content_length = "content-length: 123\r\n";
	size_t pos = 0;
	raw_headers_t headers;
	headers = extract_headers(content_length, pos);
	assert((headers["content-length"] == " 123"));
	assert(pos == 21);

	std::string host = "host:localhost\r\n";
	pos = 0;
	headers.clear();
	headers = extract_headers(host, pos);
	assert((headers["host"] == "localhost"));
	assert(pos == 16);

	std::string empty_val = "empty_val:\r\n";
	pos = 0;
	headers.clear();
	headers = extract_headers(empty_val, pos);
	assert((headers["empty_val"] == ""));
	assert(pos == 12);

	// wrong inputs
	std::string no_crlf = "host:123.0.0.1";
	pos = 0;
	try {
		extract_headers(no_crlf, pos);
		assert((false && "no crlf"));
	} catch (const Request::BadRequest& e) {};

	std::string no_key = ":local\r\n";
	pos = 0;
	try {
		extract_headers(no_key, pos);
		assert((false && "no key"));
	} catch (const Request::BadRequest& e) {};
}

void test_parse_content_length()
{
	// correct inputs
	raw_headers_t simple;
	simple["content-length"] = "123";
	assert((parse_content_length(simple) == 123));

	raw_headers_t leading_spaces;
	leading_spaces["content-length"] = "   123";
	assert((parse_content_length(leading_spaces) == 123));

	raw_headers_t trailing_spaces;
	trailing_spaces["content-length"] = "123    ";
	assert((parse_content_length(trailing_spaces) == 123));

	raw_headers_t spaces;
	spaces["content-length"] = "     123    ";
	assert((parse_content_length(spaces) == 123));

	raw_headers_t max_ulong;
	max_ulong["content-length"] = "18446744073709551615";
	assert((parse_content_length(max_ulong) == 18446744073709551615ul));

	// wrong inputs
	raw_headers_t spaces_inside;
	spaces_inside["content-length"] = "     123 134    ";
	try {
		parse_content_length(spaces_inside);
		assert((false && "spaces_inside"));
	} catch (const Request::BadRequest& e) {};

	raw_headers_t overflow;
	overflow["content-length"] = "18446744073709551616";
	try {
		parse_content_length(overflow);
		assert((false && "overflow"));
	} catch (const Request::BadRequest& e) {};

	raw_headers_t neg;
	neg["content-length"] = "-134";
	try {
		parse_content_length(neg);
		assert((false && "neg"));
	} catch (const Request::BadRequest& e) {};

	raw_headers_t double_neg;
	double_neg["content-length"] = "--134";
	try {
		parse_content_length(double_neg);
		assert((false && "double_neg"));
	} catch (const Request::BadRequest& e) {};

	raw_headers_t plus_neg;
	plus_neg["content-length"] = "+-134";
	try {
		parse_content_length(plus_neg);
		assert((false && "plus_neg"));
	} catch (const Request::BadRequest& e) {};

	raw_headers_t chars;
	chars["content-length"] = "hello";
	try {
		parse_content_length(chars);
		assert((false && "chars"));
	} catch (const Request::BadRequest& e) {};
}

void test_parse_connection()
{
	// correct inputs
	raw_headers_t keep_alive;
	keep_alive["connection"] = "keep-alive";
	assert((parse_connection(keep_alive) == true));

	raw_headers_t keep_alive_up;
	keep_alive_up["connection"] = "KEEP-ALIVE";
	assert((parse_connection(keep_alive_up) == true));

	raw_headers_t keep_alive_mixed;
	keep_alive_mixed["connection"] = "Keep-Alive";
	assert((parse_connection(keep_alive_mixed) == true));

	raw_headers_t close;
	close["connection"] = "close";
	assert((parse_connection(close) == false));

	raw_headers_t close_up;
	close_up["connection"] = "close";
	assert((parse_connection(close_up) == false));

	// wrong inputs
	// no wrong inputs, just treated as default -> close
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
	test(test_extract_headers, "test_extract_headers");
	test(test_parse_content_length, "test_parse_content_length");
	test(test_parse_connection, "test_parse_connection");
	return (0);
}
