#include "../src/config_parser.h"
#include "lib_test.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

using std::string;
using namespace _preproc;

void test_remove_comments_nl()
{
	std::stringstream config;

	config << "one line";
	string one_line = remove_comments_nl(config);
	assert(one_line == "one line ");

	config.str("");
	config.clear();
	config << "test";
	string empty = remove_comments_nl(config);
	assert(empty == "test ");

	config.str("");
	config.clear();
	config << "location#comment\n/\n{";
	string nl_separator = remove_comments_nl(config);
	assert((nl_separator == "location / { "));

	config.str("");
	config.clear();
	config << " 	#comment  \n    \n   #comment\n\n";
	string comments_only = remove_comments_nl(config);
	assert((comments_only == ""));

	config.str("");
	config.clear();
	config << " 	#comment  \n    location\n   #comment\n/ {\n";
	string mix = remove_comments_nl(config);
	assert((mix == "    location / { "));

// EXEMPLE CONFIG RESULT
// 	config.str("");
// 	config.clear();
// 	config << 
// "# Comments '#' can be everywhere on the line, all subsequent chars will be ignored until '\\n'.\n"
// "# Spaces are not semantic (outside of key value separators) and can be either SP ot HTAB.\n"
// "events {\n"
// "	# the max number of simultaneous connections (client + server conections)\n"
// "	# open sockets, Nginx use 2 connections at start\n"
// "	worker_connections 3;\n"
// "}\n"
// "\n"
// "http {\n"
// "	# include raw file content (must be structured)\n"
// "	include			mime.types; # fill 'types' block\n"
// "	default_type	application/octet-stream;\n"
// "\n"
// "	keepalive_timeout	10;\n"
// "\n"
// "	server {\n"
// "		listen 127.0.0.1:4242;\n"
// "		# server_name test_serv;\n"
// "		location / {\n"
// "			# root filesystem, absolute form or relative to server executable file\n"
// "			# (if symlink resolve to source)\n"
// "			root	/home/juportie/Documents/webserv/html;\n"
// "			# file to serve for '/' path\n"
// "			index	index.html;\n"
// "		}\n"
// "		# if call /src return redirect\n"
// "		# if call /src/ return index or list directory if autoindex on\n"
// "		location /src {\n"
// "			# root filesystem, absolute form or relative to server executable file\n"
// "			# (if symlink resolve to source)\n"
// "			root	/home/juportie/Documents/webserv;\n"
// "			# file to serve for '/' path\n"
// "			index	Config.h;\n"
// "			# list directory tree\n"
// "			autoindex on;\n"
// "			# limit location to methods\n"
// "			limit_except GET {\n"
// "				allow 127.0.0.1;\n"
// "				deny all;\n"
// "			}\n"
// "		}\n"
// "        # redirect server error pages to the static page /404.html\n"
// "        error_page	404	/404.html;\n"
// "	}\n"
// "}\n";
// 	string full = remove_comments_nl(config);
// 	std::cout << full << '\n';
}

void test_expand_includes()
{
	std::list<string> cmp;
	cmp.push_back("http");
	cmp.push_back("{");
	cmp.push_back("test");
	cmp.push_back("t");
	cmp.push_back(";");
	cmp.push_back("}");

	std::list<string> base;
	base.push_back("http");
	base.push_back("{");
	base.push_back("include");
	base.push_back("test_data/config/test.types");
	base.push_back(";");
	base.push_back("}");
	expand_includes(base);
	// for (std::list<string>::iterator it = base.begin(); it != base.end(); ++it)
	// 	std::cout << '|' << *it << "|\n";
	assert((base == cmp));

	std::list<string> comments;
	comments.push_back("http");
	comments.push_back("{");
	comments.push_back("include");
	comments.push_back("test_data/config/test_comments.types");
	comments.push_back(";");
	comments.push_back("}");
	expand_includes(comments);
	// for (std::list<string>::iterator it = comments.begin(); it != comments.end(); ++it)
	// 	std::cout << '|' << *it << "|\n";
	assert((comments == cmp));

	cmp.clear();
	cmp.push_back("test");
	cmp.push_back("t");
	std::list<string> only_inc;
	only_inc.push_back("include");
	only_inc.push_back("test_data/config/test.types");
	expand_includes(only_inc);
	// for (std::list<string>::iterator it = only_inc.begin(); it != only_inc.end(); ++it)
	// 	std::cout << '|' << *it << "|\n";
	assert((only_inc == cmp));

	cmp.clear();
	std::list<string> empty;
	expand_includes(empty);
	// for (std::list<string>::iterator it = empty.begin(); it != empty.end(); ++it)
	// 	std::cout << '|' << *it << "|\n";
	assert((empty == cmp));

	cmp.clear();
	std::list<string> no_path;
	no_path.push_back("include");
	try {
		expand_includes(no_path);
		assert((false && "no_path"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: unexpected end of file") == 0));
	}
}

void test_tokenize_config()
{
	std::list<string> cmp;
	cmp.push_back("http");
	cmp.push_back("{");
	cmp.push_back("key");
	cmp.push_back("value");
	cmp.push_back(";");
	cmp.push_back("}");

	std::list<string> simple = tokenize_config("http { key value; }");
	assert((simple == cmp));

	std::list<string> multi_sep = tokenize_config("	 \nhttp {	 key value; }	");
	assert((multi_sep == cmp));

	std::list<string> minimum_spaces = tokenize_config("http{key value;}");
	assert((minimum_spaces == cmp));

	cmp.push_back("key");
	cmp.push_back("value");
	std::list<string> end_val = tokenize_config("http {	 key value; } key value");
	assert((end_val == cmp));

	// std::list<string> complex = tokenize_config(
	// 	"http {\n"
	// 	"	include			test_data/config/mime.types;\n"
	// 	"	default_type	application/octet-stream;\n"
	// 	"\n"
	// 	"	server {\n"
	// 	"		listen 127.0.0.1:4242;\n"
	// 	"		location / {\n"
	// 	"			root	/home/juportie/Documents/webserv/html;\n"
	// 	"			index	index.html;\n"
	// 	"		}\n"
	// 	"	}\n"
	// 	"}\n");
	// for (std::list<string>::iterator it = complex.begin(); it != complex.end(); ++it) { 
	// 	std::cout << *it << '\n';
	// }
}

int main(void)
{
	TEST(test_remove_comments_nl);
	TEST(test_expand_includes);
	TEST(test_tokenize_config);
	return (0);
}
