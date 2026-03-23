#include "../src/request_parser.h"
#include "../src/http_types.h"
#include "../src/Config.h"
#include "lib_test.h"

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
	} catch (const Request::NotImplemented& e) {};

	std::string lowercase = "get";
	pos = 0;
	try {
		parse_method(lowercase, pos);
		assert((false && "lowercase"));
	} catch (const Request::NotImplemented& e) {};

	std::string space_front = " GET";
	pos = 0;
	try {
		parse_method(lowercase, pos);
		assert((false && "lowercase"));
	} catch (const Request::NotImplemented& e) {};

	std::string empty = "";
	pos = 0;
	try {
		parse_method(empty, pos);
		assert((false && "empty"));
	} catch (const Request::NotImplemented& e) {};
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

void test_parse_headers()
{
	// correct inputs
	request_t request;
	request.method = get;
	request.target = "/";
	request.protocol = one;
	size_t pos = 0;

	std::string b_alive = "connection: keep-alive" CRLF CRLF;
	pos = 0;
	config_t config;
	headers_t alive = parse_headers(b_alive, pos, request, config);
	assert((alive.keep_alive == true));

	std::string b_alive_trail = "connection: keep-alive" CRLF CRLF
		"anything: random" CRLF CRLF;
	pos = 0;
	headers_t alive_trail = parse_headers(b_alive_trail, pos, request, config);
	assert((alive_trail.keep_alive == true));

	std::string b_alive_lead = "anything: random" CRLF
		"connection: keep-alive" CRLF CRLF;
	pos = 0;
	headers_t alive_lead = parse_headers(b_alive_lead, pos, request, config);
	assert((alive_lead.keep_alive == true));

	std::string b_length = "content-length: 123" CRLF CRLF;
	pos = 0;
	request.method = post;
	headers_t length = parse_headers(b_length, pos, request, config);
	assert((length.content_length == 123));

	std::string b_length_alive = "content-length: 123" CRLF
		"connection: keep-alive" CRLF CRLF;
	pos = 0;
	request.method = post;
	headers_t length_alive = parse_headers(b_length_alive, pos, request, config);
	assert((length_alive.content_length == 123));
	assert((length_alive.keep_alive == true));

	std::string b_length_alive_rand = "anything: random" CRLF
		"content-length: 123" CRLF
		"anything: random" CRLF
		"anything: random" CRLF
		"connection: keep-alive" CRLF
		"anything: random" CRLF CRLF;
	pos = 0;
	request.method = post;
	headers_t length_alive_rand = parse_headers(b_length_alive_rand, pos, request, config);
	assert((length_alive_rand.content_length == 123));
	assert((length_alive_rand.keep_alive == true));

	std::string b_no_headers = CRLF CRLF;
	pos = 0;
	request.method = get;
	headers_t no_headers = parse_headers(b_no_headers, pos, request, config);
	assert((no_headers.content_length == 0));
	assert((no_headers.keep_alive == false));

	std::string b_merged = "connection: keep-alive" "anything: random" "content-length: 123" CRLF CRLF;
	pos = 0;
	headers_t merged = parse_headers(b_merged, pos, request, config);
	// nginx don't parse it like that it just checks the substring "keep-alive" and return true
	// but I think it's more clean to do it like we do.
	assert((merged.keep_alive == false));

	// wrong inputs
	std::string b_no_colon = "connection keep-alive" CRLF CRLF;
	pos = 0;
	try {
		headers_t no_colon = parse_headers(b_no_colon, pos, request, config);
		assert((false && "no_colon"));
	}
	catch (const Request::BadRequest& e) {}

	std::string b_one_crlf = "connection: keep-alive" CRLF;
	pos = 0;
	try {
		headers_t one_crlf = parse_headers(b_one_crlf, pos, request, config);
		assert((false && "one_crlf"));
	}
	catch (const Request::BadRequest& e) {}
}

void test_extract_body()
{
	// correct inputs
	string simple_b = "123456789";
	request_t simple_r;
	simple_r.headers.content_length = 9;
	size_t pos = 0;
	string simple = extract_body(simple_b, pos, simple_r);
	assert((simple == "123456789"));
	assert((pos == 9));
	assert((simple_b[pos] == '\0'));
	assert((simple_r.status == ok));

	string more_b = "123456789" 
					"GET / HTTP/1.0";
	request_t more_r;
	more_r.headers.content_length = 9;
	pos = 0;
	string more = extract_body(more_b, pos, more_r);
	assert((more == "123456789"));
	assert((pos == 9));
	assert((more_b[pos] == 'G'));
	assert((more_r.status == ok));

	string less_b = "123";
	request_t less_r;
	less_r.headers.content_length = 9;
	pos = 0;
	string less = extract_body(less_b, pos, less_r);
	assert((less == "123"));
	assert((pos == 3));
	assert((less_b[pos] == '\0'));
	assert((less_r.status == parsing));
}

void test_parse()
{
	config_t config;
	// correct inputs
	Request s_l(config, "GET / HTTP/1.0" CRLF CRLF);
	s_l.parse();
	request_t start_line = s_l.get_infos();
	assert((start_line.method == get));
	assert((start_line.target == "/"));
	assert((start_line.protocol == one));
	assert((start_line.status == ok));

	Request l_crlf(config, CRLF CRLF CRLF "GET / HTTP/1.0" CRLF CRLF);
	l_crlf.parse();
	request_t leading_crlf = l_crlf.get_infos();
	assert((leading_crlf.method == get));
	assert((leading_crlf.target == "/"));
	assert((leading_crlf.protocol == one));
	assert((leading_crlf.status == ok));

	Request o_o(config, "GET / HTTP/1.1" CRLF CRLF);
	o_o.parse();
	request_t oneone = o_o.get_infos();
	assert((oneone.method == get));
	assert((oneone.target == "/"));
	assert((oneone.protocol == one_one));
	assert((oneone.status == ok));

	Request c_l(config, "POST / HTTP/1.0" CRLF
				"content-length: 2" CRLF
				CRLF
				"ok");
	c_l.parse();
	request_t content_length = c_l.get_infos();
	assert((content_length.method == post));
	assert((content_length.target == "/"));
	assert((content_length.protocol == one));
	assert((content_length.status == ok));
	assert((content_length.headers.content_length == 2));
	assert((content_length.body == "ok"));

	Request r_h(config, "POST / HTTP/1.0" CRLF
				"rand0: asd" CRLF
				"rand1: asd" CRLF
				"rand2:" CRLF
				"content-length: 2" CRLF
				"rand3:" CRLF
				CRLF
				"ok");
	r_h.parse();
	request_t random_headers = r_h.get_infos();
	assert((random_headers.method == post));
	assert((random_headers.target == "/"));
	assert((random_headers.protocol == one));
	assert((random_headers.status == ok));
	assert((random_headers.headers.content_length == 2));
	assert((content_length.body == "ok"));

	Request d_r(config, "POST / HTTP/1.0" CRLF
				"content-length: 2" CRLF
				CRLF
				"ok"
				"GET / HTTP/1.0" CRLF);
	d_r.parse();
	request_t double_request = d_r.get_infos();
	assert((double_request.method == post));
	assert((double_request.target == "/"));
	assert((double_request.protocol == one));
	assert((double_request.status == ok));
	assert((double_request.headers.content_length == 2));
	assert((content_length.body == "ok"));

	Request p_b(config, "POST / HTTP/1.0" CRLF
				"content-length: 123" CRLF
				CRLF
				"ok");
	p_b.parse();
	request_t partial_body = p_b.get_infos();
	assert((partial_body.method == post));
	assert((partial_body.target == "/"));
	assert((partial_body.protocol == one));
	assert((partial_body.status == parsing));
	assert((partial_body.headers.content_length == 123));
	assert((content_length.body == "ok"));

	// wrong inputs (expected, no exceptions)
	// buffer filling in progress
	Request o_cl(config, "GET / HTTP/1.0" CRLF);
	o_cl.parse();
	request_t one_crlf = o_cl.get_infos();
	assert((one_crlf.status == parsing));

	Request o_n(config, "GET /" CRLF CRLF);
	o_n.parse();
	request_t o_nine = o_n.get_infos();
	assert((o_nine.method == get));
	assert((o_nine.target == "/"));
	assert((o_nine.protocol == zero_nine));
	assert((o_nine.status == bad_request));

	Request j_t_c(config, "" CRLF CRLF);
	j_t_c.parse();
	request_t just_two_crlf = j_t_c.get_infos();
	assert((just_two_crlf.status == not_implemented));

	// Multiple recv requests
	Request m(config, "GET / HTTP/1.0" CRLF);
	m.parse();
	request_t multiple = m.get_infos();
	assert((multiple.status == parsing));
	m._append_buffer(CRLF);
	m.parse();
	multiple = m.get_infos();
	assert((multiple.method == get));
	assert((multiple.target == "/"));
	assert((multiple.protocol == one));
	assert((multiple.status == ok));
	m.clear_infos();
	m._append_buffer("POST /post HTTP/1.0" CRLF
						"content-length: 3" CRLF
						CRLF
						"ok");
	m.parse();
	multiple = m.get_infos();
	assert((multiple.method == post));
	assert((multiple.target == "/post"));
	assert((multiple.protocol == one));
	assert((multiple.headers.content_length == 3));
	assert((multiple.body == "ok"));
	assert((multiple.status == parsing));
	m._append_buffer("!DELETE /del HTTP/1.0" CRLF
						CRLF);
	m.parse();
	multiple = m.get_infos();
	assert((multiple.method == post));
	assert((multiple.target == "/post"));
	assert((multiple.protocol == one));
	assert((multiple.headers.content_length == 3));
	assert((multiple.body == "ok!"));
	assert((multiple.status == ok));
	m.clear_infos();
	m.parse();
	multiple = m.get_infos();
	assert((multiple.method == del));
	assert((multiple.target == "/del"));
	assert((multiple.protocol == one));
	assert((multiple.headers.content_length == 0));
	assert((multiple.body == ""));
	assert((multiple.status == ok));
}

int main(void)
{
	// start line
	TEST(test_parse_method);
	TEST(test_parse_target);
	TEST(test_parse_protocol);
	// headers
	TEST(test_parse_headers);
	// body
	TEST(test_extract_body);
	// class methods
	TEST(test_parse);
	return (0);
}
