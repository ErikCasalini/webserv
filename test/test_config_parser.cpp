#include "../src/config_parser.h"
#include "lib_test.h"

using std::string;
using std::list;

void test_parse_max_connections()
{
	config_t base_conf;
	list<string> base_lst;
	base_lst.push_back("max_connections");
	base_lst.push_back("257");
	base_lst.push_back(";");
	base_lst.push_back("}");
	list<string>::iterator base_it = base_lst.begin();
	parse_max_connections(base_conf, base_it, base_lst);
	assert((base_conf.events.max_connections == 257));
	assert((*base_it == "}"));

	config_t min_conf;
	list<string> min_lst;
	min_lst.push_back("max_connections");
	std::stringstream ss;
	ss << CONNECTIONS_MIN;
	min_lst.push_back(ss.str());
	min_lst.push_back(";");
	min_lst.push_back("}");
	list<string>::iterator min_it = min_lst.begin();
	parse_max_connections(min_conf, min_it, min_lst);
	assert((min_conf.events.max_connections == CONNECTIONS_MIN));
	assert((*min_it == "}"));

	config_t max_conf;
	list<string> max_lst;
	max_lst.push_back("max_connections");
	ss.str("");
	ss.clear();
	ss << CONNECTIONS_MAX;
	max_lst.push_back(ss.str());
	max_lst.push_back(";");
	max_lst.push_back("}");
	list<string>::iterator max_it = max_lst.begin();
	parse_max_connections(max_conf, max_it, max_lst);
	assert((max_conf.events.max_connections == CONNECTIONS_MAX));
	assert((*max_it == "}"));

	config_t neg_conf;
	list<string> neg_lst;
	neg_lst.push_back("max_connections");
	neg_lst.push_back("-1");
	neg_lst.push_back(";");
	neg_lst.push_back("}");
	list<string>::iterator neg_it = neg_lst.begin();
	try {
		parse_max_connections(neg_conf, neg_it, neg_lst);
		assert((false && "neg"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: max_connections can't be negative") == 0));
	}

	config_t zero_conf;
	list<string> zero_lst;
	zero_lst.push_back("max_connections");
	zero_lst.push_back("0");
	zero_lst.push_back(";");
	zero_lst.push_back("}");
	list<string>::iterator zero_it = zero_lst.begin();
	try {
		parse_max_connections(zero_conf, zero_it, zero_lst);
		assert((false && "zero"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: invalid max_connections value") == 0));
	}

	config_t bigger_conf;
	list<string> bigger_lst;
	bigger_lst.push_back("max_connections");
	ss.str("");
	ss.clear();
	ss << CONNECTIONS_MAX + 1;
	bigger_lst.push_back(ss.str());
	bigger_lst.push_back(";");
	bigger_lst.push_back("}");
	list<string>::iterator bigger_it = bigger_lst.begin();
	try {
		parse_max_connections(bigger_conf, bigger_it, bigger_lst);
		assert((false && "bigger"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: invalid max_connections value") == 0));
	}

	config_t smaller_conf;
	list<string> smaller_lst;
	smaller_lst.push_back("max_connections");
	ss.str("");
	ss.clear();
	ss << CONNECTIONS_MIN - 1;
	smaller_lst.push_back(ss.str());
	smaller_lst.push_back(";");
	smaller_lst.push_back("}");
	list<string>::iterator smaller_it = smaller_lst.begin();
	try {
		parse_max_connections(smaller_conf, smaller_it, smaller_lst);
		assert((false && "smaller"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: invalid max_connections value") == 0));
	}

	config_t eof_conf;
	list<string> eof_lst;
	eof_lst.push_back("max_connections");
	eof_lst.push_back("257");
	list<string>::iterator eof_it = eof_lst.begin();
	try {
		parse_max_connections(eof_conf, eof_it, eof_lst);
		assert((false && "eof"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: unexpected end of file") == 0));
	}

	config_t nosep_conf;
	list<string> nosep_lst;
	nosep_lst.push_back("max_connections");
	nosep_lst.push_back("257");
	nosep_lst.push_back("}");
	list<string>::iterator nosep_it = nosep_lst.begin();
	try {
		parse_max_connections(nosep_conf, nosep_it, nosep_lst);
		assert((false && "nosep"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(),
					"config: missing ';'") == 0));
	}

	config_t nonum_conf;
	list<string> nonum_lst;
	nonum_lst.push_back("max_connections");
	nonum_lst.push_back("test");
	list<string>::iterator nonum_it = nonum_lst.begin();
	try {
		parse_max_connections(nonum_conf, nonum_it, nonum_lst);
		assert((false && "nonum"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(),
					"config: invalid max_connections value") == 0));
	}
}

void test_parse_events()
{
	config_t base_conf;
	list<string> base_lst;
	base_lst.push_back("events");
	base_lst.push_back("{");
	base_lst.push_back("max_connections");
	base_lst.push_back("257");
	base_lst.push_back(";");
	base_lst.push_back("}");
	list<string>::iterator base_it = base_lst.begin();
	parse_events(base_conf, base_it, base_lst);
	assert((base_conf.events.max_connections == 257));
	assert((base_it == base_lst.end()));

	config_t redef_conf;
	list<string> redef_lst;
	redef_lst.push_back("events");
	redef_lst.push_back("{");
	redef_lst.push_back("max_connections");
	redef_lst.push_back("258");
	redef_lst.push_back(";");
	redef_lst.push_back("max_connections");
	redef_lst.push_back("257");
	redef_lst.push_back(";");
	redef_lst.push_back("}");
	list<string>::iterator redef_it = redef_lst.begin();
	try {
		parse_events(redef_conf, redef_it, redef_lst);
		assert((false && "redef"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(),
					"config: invalid field") == 0));
	}

	config_t noopen_conf;
	list<string> noopen_lst;
	noopen_lst.push_back("events");
	noopen_lst.push_back("max_connections");
	noopen_lst.push_back("257");
	noopen_lst.push_back(";");
	noopen_lst.push_back("}");
	list<string>::iterator noopen_it = noopen_lst.begin();
	try {
		parse_events(noopen_conf, noopen_it, noopen_lst);
		assert((false && "noopen"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: missing '{'") == 0));
	}

	config_t nofield_conf;
	list<string> nofield_lst;
	nofield_lst.push_back("events");
	nofield_lst.push_back("{");
	nofield_lst.push_back("}");
	list<string>::iterator nofield_it = nofield_lst.begin();
	parse_events(nofield_conf, nofield_it, nofield_lst);
	assert((nofield_conf.events.max_connections == 256)); // 256 is init/default value
	assert((nofield_it == nofield_lst.end()));
}

void test_parse_main()
{
	config_t base_conf;
	list<string> base_lst;
	base_lst.push_back("events");
	base_lst.push_back("{");
	base_lst.push_back("max_connections");
	base_lst.push_back("257");
	base_lst.push_back(";");
	base_lst.push_back("}");
	parse_main(base_conf, base_lst);
	assert((base_conf.events.max_connections == 257));

	config_t double_conf;
	list<string> double_lst;
	double_lst.push_back("events");
	double_lst.push_back("{");
	double_lst.push_back("max_connections");
	double_lst.push_back("257");
	double_lst.push_back(";");
	double_lst.push_back("}");
	double_lst.push_back("events");
	double_lst.push_back("{");
	double_lst.push_back("max_connections");
	double_lst.push_back("257");
	double_lst.push_back(";");
	double_lst.push_back("}");
	try {
		parse_main(double_conf, double_lst);
		assert((false && "double"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: invalid field") == 0));
	}

	config_t novalid_conf;
	list<string> novalid_lst;
	novalid_lst.push_back("{");
	novalid_lst.push_back("max_connections");
	novalid_lst.push_back("257");
	novalid_lst.push_back(";");
	novalid_lst.push_back("}");
	novalid_lst.push_back("{");
	novalid_lst.push_back("max_connections");
	novalid_lst.push_back("257");
	novalid_lst.push_back(";");
	novalid_lst.push_back("}");
	try {
		parse_main(novalid_conf, novalid_lst);
		assert((false && "novalid"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: invalid field") == 0));
	}
}

void test_extract_boolean_field()
{
	list<string> true_lst;
	true_lst.push_back("true");
	true_lst.push_back(";");
	list<string>::iterator true_it = true_lst.begin();
	bool true_b = extract_boolean_field(true_it, true_lst);
	assert((true_b == true));
	assert((*true_it == ";"));

	list<string> false_lst;
	false_lst.push_back("false");
	false_lst.push_back(";");
	list<string>::iterator false_it = false_lst.begin();
	bool false_b = extract_boolean_field(false_it, false_lst);
	assert((false_b == false));
	assert((*false_it == ";"));

	list<string> nobool_lst;
	nobool_lst.push_back("nobool");
	nobool_lst.push_back(";");
	list<string>::iterator nobool_it = nobool_lst.begin();
	try {
		extract_boolean_field(nobool_it, nobool_lst);
		assert((false && "nobool"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: expected boolean") == 0));
	}

	list<string> noval_lst;
	noval_lst.push_back(";");
	noval_lst.push_back("}");
	list<string>::iterator noval_it = noval_lst.begin();
	try {
		extract_boolean_field(noval_it, noval_lst);
		assert((false && "noval"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: expected boolean") == 0));
	}

	list<string> one_lst;
	one_lst.push_back("true");
	list<string>::iterator one_it = one_lst.begin();
	try {
		extract_boolean_field(one_it, one_lst);
		assert((false && "one"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: unexpected end of file") == 0));
	}
}

void test_parse_autoindex()
{
	config_t true_conf;
	list<string> true_lst;
	true_lst.push_back("autoindex");
	true_lst.push_back("true");
	true_lst.push_back(";");
	true_lst.push_back("}");
	list<string>::iterator true_it = true_lst.begin();
	parse_autoindex(true_conf, true_it, true_lst);
	assert((true_conf.http.autoindex == true));
	assert((*true_it == "}"));

	config_t nosep_conf;
	list<string> nosep_lst;
	nosep_lst.push_back("autoindex");
	nosep_lst.push_back("true");
	nosep_lst.push_back("}");
	list<string>::iterator nosep_it = nosep_lst.begin();
	try {
		parse_autoindex(nosep_conf, nosep_it, nosep_lst);
		assert((false && "nosep"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: missing ';'") == 0));
	}

	config_t nokey_conf;
	list<string> nokey_lst;
	nokey_lst.push_back("true");
	nokey_lst.push_back(";");
	list<string>::iterator nokey_it = nokey_lst.begin();
	try {
		parse_autoindex(nokey_conf, nokey_it, nokey_lst);
		assert((false && "nokey"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: expected boolean") == 0));
	}

	config_t end_conf;
	list<string> end_lst;
	list<string>::iterator end_it = end_lst.end();
	try {
		parse_autoindex(end_conf, end_it, end_lst);
		assert((false && "end"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: unexpected end of file") == 0));
	}
}

void test_parse_root()
{
	config_t base_conf;
	list<string> base_lst;
	base_lst.push_back("root");
	base_lst.push_back("/base/");
	base_lst.push_back(";");
	base_lst.push_back("}");
	list<string>::iterator base_it = base_lst.begin();
	parse_root(base_conf, base_it, base_lst);
	assert((base_conf.http.root == "/base/"));
	assert((*base_it == "}"));

	config_t nobslash_conf;
	list<string> nobslash_lst;
	nobslash_lst.push_back("root");
	nobslash_lst.push_back("base/");
	nobslash_lst.push_back(";");
	nobslash_lst.push_back("}");
	list<string>::iterator nobslash_it = nobslash_lst.begin();
	parse_root(nobslash_conf, nobslash_it, nobslash_lst);
	assert((nobslash_conf.http.root == "/base/"));
	assert((*nobslash_it == "}"));

	config_t noeslash_conf;
	list<string> noeslash_lst;
	noeslash_lst.push_back("root");
	noeslash_lst.push_back("/base");
	noeslash_lst.push_back(";");
	noeslash_lst.push_back("}");
	list<string>::iterator noeslash_it = noeslash_lst.begin();
	parse_root(noeslash_conf, noeslash_it, noeslash_lst);
	assert((noeslash_conf.http.root == "/base/"));
	assert((*noeslash_it == "}"));

	config_t noslash_conf;
	list<string> noslash_lst;
	noslash_lst.push_back("root");
	noslash_lst.push_back("base");
	noslash_lst.push_back(";");
	noslash_lst.push_back("}");
	list<string>::iterator noslash_it = noslash_lst.begin();
	parse_root(noslash_conf, noslash_it, noslash_lst);
	assert((noslash_conf.http.root == "/base/"));
	assert((*noslash_it == "}"));

	config_t eof_conf;
	list<string> eof_lst;
	eof_lst.push_back("root");
	list<string>::iterator eof_it = eof_lst.begin();
	try {
		parse_root(eof_conf, eof_it, eof_lst);
		assert((false && "eof_conf"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: unexpected end of file") == 0));
	}

	config_t eof2_conf;
	list<string> eof2_lst;
	eof2_lst.push_back("root");
	eof2_lst.push_back("/base/");
	list<string>::iterator eof2_it = eof2_lst.begin();
	try {
		parse_root(eof2_conf, eof2_it, eof2_lst);
		assert((false && "eof2_conf"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: unexpected end of file") == 0));
	}

	config_t nosep_conf;
	list<string> nosep_lst;
	nosep_lst.push_back("root");
	nosep_lst.push_back("/base/");
	nosep_lst.push_back("}");
	list<string>::iterator nosep_it = nosep_lst.begin();
	try {
		parse_root(nosep_conf, nosep_it, nosep_lst);
		assert((false && "nosep_conf"));
	} catch (const std::runtime_error& e) {
		assert((std::strcmp(e.what(), "config: missing ';'") == 0));
	}
}

int main(void)
{
	TEST(test_parse_max_connections);
	TEST(test_parse_events);
	TEST(test_parse_main);
	TEST(test_extract_boolean_field);
	TEST(test_parse_autoindex);
	TEST(test_parse_root);
	return (0);
}
