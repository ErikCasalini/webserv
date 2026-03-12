#include "../src/ConfigLexer.h"
#include "lib_test.h"
#include <cstring>
#include <sstream>
#include <string>

using std::string;
using namespace _config_lexer;
using namespace config_files;

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
}

void test_expand_includes()
{
	std::list<string> cmp;
	cmp.push_back("http");
	cmp.push_back("{");
	cmp.push_back("test");
	cmp.push_back("t");
	cmp.push_back("}");

	std::list<string> base;
	base.push_back("http");
	base.push_back("{");
	base.push_back("include");
	base.push_back("test.types");
	base.push_back(";");
	base.push_back("}");
	expand_includes(base, "test_data/config/config");
	assert((base == cmp));

	std::list<string> comments;
	comments.push_back("http");
	comments.push_back("{");
	comments.push_back("include");
	comments.push_back("test_comments.types");
	comments.push_back(";");
	comments.push_back("}");
	expand_includes(comments, "test_data/config/config");
	assert((comments == cmp));

	cmp.clear();
	std::list<string> empty;
	expand_includes(empty, "test_data/config/config");
	assert((empty == cmp));

	cmp.clear();
	cmp.push_back("test");
	cmp.push_back("t");
	std::list<string> only_inc;
	only_inc.push_back("include");
	only_inc.push_back("test.types");
	try {
		expand_includes(only_inc, "test_data/config/config");
		assert((false && "only_inc"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: unexpected end of file") == 0));
	}

	cmp.clear();
	std::list<string> no_path;
	no_path.push_back("include");
	try {
		expand_includes(no_path, "test_data/config/config");
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
}

int main(void)
{
	TEST(test_remove_comments_nl);
	TEST(test_expand_includes);
	TEST(test_tokenize_config);
	return (0);
}
