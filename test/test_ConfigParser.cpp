#include "../src/ConfigParser.h"
#include "../src/ConfigLexer.h"
#include "lib_test.h"

using std::string;
using std::list;

// void test_parse_max_connections()
// {
// 	list<string> base_lst;
//
// 	base_lst.push_back("events");
// 	base_lst.push_back("{");
// 	base_lst.push_back("max_connections");
// 	base_lst.push_back("257");
// 	base_lst.push_back(";");
// 	base_lst.push_back("}");
// 	ConfigParser base_c(base_lst, "placeholder_file", "/path/webserv");
// 	config_t base_conf = base_c.parse();
// 	assert((base_conf.events.max_connections == 257));
//
// 	list<string> min_lst;
// 	min_lst.push_back("events");
// 	min_lst.push_back("{");
// 	min_lst.push_back("max_connections");
// 	std::stringstream ss;
// 	ss << CONNECTIONS_MIN;
// 	min_lst.push_back(ss.str());
// 	min_lst.push_back(";");
// 	min_lst.push_back("}");
// 	ConfigParser min_c(min_lst, "placeholder_file", "/path/webserv");
// 	config_t min_conf = min_c.parse();
// 	assert((min_conf.events.max_connections == CONNECTIONS_MIN));
//
// 	list<string> max_lst;
// 	max_lst.push_back("events");
// 	max_lst.push_back("{");
// 	max_lst.push_back("max_connections");
// 	ss.str("");
// 	ss.clear();
// 	ss << CONNECTIONS_MAX;
// 	max_lst.push_back(ss.str());
// 	max_lst.push_back(";");
// 	max_lst.push_back("}");
// 	ConfigParser max_c(max_lst, "placeholder_file", "/path/webserv");
// 	config_t max_conf = max_c.parse();
// 	assert((max_conf.events.max_connections == CONNECTIONS_MAX));
//
// 	list<string> neg_lst;
// 	neg_lst.push_back("events");
// 	neg_lst.push_back("{");
// 	neg_lst.push_back("max_connections");
// 	neg_lst.push_back("-1");
// 	neg_lst.push_back(";");
// 	neg_lst.push_back("}");
// 	ConfigParser neg_c(neg_lst, "placeholder_file", "/path/webserv");
// 	try {
// 		neg_c.parse();
// 		assert((false && "neg"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(),  "config: max_connections value can't be negative '-1'") == 0));
// 	}
//
// 	list<string> zero_lst;
// 	zero_lst.push_back("events");
// 	zero_lst.push_back("{");
// 	zero_lst.push_back("max_connections");
// 	zero_lst.push_back("0");
// 	zero_lst.push_back(";");
// 	zero_lst.push_back("}");
// 	ConfigParser zero_c(zero_lst, "placeholder_file", "/path/webserv");
// 	try {
// 		zero_c.parse();
// 		assert((false && "zero"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: invalid max_connections value '0'") == 0));
// 	}
//
// 	list<string> bigger_lst;
// 	bigger_lst.push_back("events");
// 	bigger_lst.push_back("{");
// 	bigger_lst.push_back("max_connections");
// 	ss.str("");
// 	ss.clear();
// 	ss << CONNECTIONS_MAX + 1;
// 	bigger_lst.push_back(ss.str());
// 	bigger_lst.push_back(";");
// 	bigger_lst.push_back("}");
// 	ConfigParser bigger_c(bigger_lst, "placeholder_file", "/path/webserv");
// 	try {
// 		bigger_c.parse();
// 		assert((false && "bigger"));
// 	} catch (const std::runtime_error& e) {
// 		// assert((std::strcmp(e.what(), "config: invalid max_connections value") == 0));
// 	}
//
// 	list<string> smaller_lst;
// 	smaller_lst.push_back("events");
// 	smaller_lst.push_back("{");
// 	smaller_lst.push_back("max_connections");
// 	ss.str("");
// 	ss.clear();
// 	ss << CONNECTIONS_MIN - 1;
// 	smaller_lst.push_back(ss.str());
// 	smaller_lst.push_back(";");
// 	smaller_lst.push_back("}");
// 	ConfigParser smaller_c(smaller_lst, "placeholder_file", "/path/webserv");
// 	try {
// 		smaller_c.parse();
// 		assert((false && "smaller"));
// 	} catch (const std::runtime_error& e) {
// 		// assert((std::strcmp(e.what(), "config: invalid max_connections value") == 0));
// 	}
//
// 	list<string> eof_lst;
// 	eof_lst.push_back("events");
// 	eof_lst.push_back("{");
// 	eof_lst.push_back("max_connections");
// 	eof_lst.push_back("257");
// 	ConfigParser eof_c(eof_lst, "placeholder_file", "/path/webserv");
// 	try {
// 		eof_c.parse();
// 		assert((false && "eof"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: unexpected end of file") == 0));
// 	}
//
// 	list<string> nosep_lst;
// 	nosep_lst.push_back("events");
// 	nosep_lst.push_back("{");
// 	nosep_lst.push_back("max_connections");
// 	nosep_lst.push_back("257");
// 	nosep_lst.push_back("}");
// 	ConfigParser nosep_c(nosep_lst, "placeholder_file", "/path/webserv");
// 	try {
// 		nosep_c.parse();
// 		assert((false && "nosep"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: unexpected token '}'") == 0));
// 	}
//
// 	list<string> nonum_lst;
// 	nonum_lst.push_back("events");
// 	nonum_lst.push_back("{");
// 	nonum_lst.push_back("max_connections");
// 	nonum_lst.push_back("test");
// 	nonum_lst.push_back("}");
// 	ConfigParser nonum_c(nonum_lst, "placeholder_file", "/path/webserv");
// 	try {
// 		nonum_c.parse();
// 		assert((false && "nonum"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(),
// 					"config: invalid max_connections value 'test'") == 0));
// 	}
// }
//
// void test_parse_events()
// {
// 	list<string> base_lst;
// 	base_lst.push_back("events");
// 	base_lst.push_back("{");
// 	base_lst.push_back("max_connections");
// 	base_lst.push_back("257");
// 	base_lst.push_back(";");
// 	base_lst.push_back("}");
// 	ConfigParser base_c(base_lst, "placeholder_file", "/path/webserv");
// 	config_t base_conf = base_c.parse();
// 	assert((base_conf.events.max_connections == 257));
//
// 	list<string> redef_lst;
// 	redef_lst.push_back("events");
// 	redef_lst.push_back("{");
// 	redef_lst.push_back("max_connections");
// 	redef_lst.push_back("258");
// 	redef_lst.push_back(";");
// 	redef_lst.push_back("max_connections");
// 	redef_lst.push_back("257");
// 	redef_lst.push_back(";");
// 	redef_lst.push_back("}");
// 	ConfigParser redef_c(redef_lst, "placeholder_file", "/path/webserv");
// 	config_t redef_conf = redef_c.parse();
// 	assert((redef_conf.events.max_connections == 257));
//
// 	list<string> noopen_lst;
// 	noopen_lst.push_back("events");
// 	noopen_lst.push_back("max_connections");
// 	noopen_lst.push_back("257");
// 	noopen_lst.push_back(";");
// 	noopen_lst.push_back("}");
// 	ConfigParser noopeen_c(noopen_lst, "placeholder_file", "/path/webserv");
// 	try {
// 		config_t noopen_conf = noopeen_c.parse();
// 		assert((false && "noopen"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: unexpected token 'max_connections'") == 0));
// 	}
//
// 	list<string> nofield_lst;
// 	nofield_lst.push_back("events");
// 	nofield_lst.push_back("{");
// 	nofield_lst.push_back("}");
// 	ConfigParser nofield_c(nofield_lst, "placeholder_file", "/path/webserv");
// 	config_t nofield_conf = nofield_c.parse();
// 	assert((nofield_conf.events.max_connections == 256)); // 256 is init/default value
// }
//
// void test_parse_http()
// {
// 	list<string> base_lst;
// 	base_lst.push_back("http");
// 	base_lst.push_back("{");
// 	base_lst.push_back("autoindex");
// 	base_lst.push_back("true");
// 	base_lst.push_back(";");
// 	base_lst.push_back("keepalive_timeout");
// 	base_lst.push_back("10");
// 	base_lst.push_back(";");
// 	base_lst.push_back("max_body_size");
// 	base_lst.push_back("10000");
// 	base_lst.push_back(";");
// 	base_lst.push_back("root");
// 	base_lst.push_back("/root/");
// 	base_lst.push_back(";");
// 	base_lst.push_back("}");
// 	ConfigParser base_c(base_lst, "placeholder_file", "/path/webserv");
// 	config_t base_conf = base_c.parse();
// 	assert((base_conf.http.autoindex == true));
// 	assert((base_conf.http.keepalive_timeout == 10));
// 	assert((base_conf.http.max_body_size == 10000));
// 	assert((base_conf.http.root == "/root/"));
// }
//
// void test_parse_listen()
// {
// 	list<string> base_lst;
// 	base_lst.push_back("http");
// 	base_lst.push_back("{");
// 	base_lst.push_back("server");
// 	base_lst.push_back("{");
// 	base_lst.push_back("listen");
// 	base_lst.push_back("127.0.0.1:4242");
// 	base_lst.push_back(";");
// 	base_lst.push_back("}");
// 	base_lst.push_back("}");
// 	ConfigParser base_c(base_lst, "placeholder_file", "/path/webserv");
// 	config_t base_conf = base_c.parse();
// 	assert((base_conf.http.server[0].listen[0].ip == 0x7F000001));
// 	assert((base_conf.http.server[0].listen[0].port == 4242));
//
// 	list<string> noip_lst;
// 	noip_lst.push_back("http");
// 	noip_lst.push_back("{");
// 	noip_lst.push_back("server");
// 	noip_lst.push_back("{");
// 	noip_lst.push_back("listen");
// 	noip_lst.push_back(":4242");
// 	noip_lst.push_back(";");
// 	noip_lst.push_back("}");
// 	noip_lst.push_back("}");
// 	ConfigParser noip_c(noip_lst, "placeholder_file", "/path/webserv");
// 	try {
// 		config_t noip_conf = noip_c.parse();
// 		assert((false && "noip"));
// 	} catch (const ConfigParser::InvalidValue& e) {
// 		assert((std::strcmp(e.what(), "config: empty ip value") == 0));
// 	}
//
// 	list<string> noport_lst;
// 	noport_lst.push_back("http");
// 	noport_lst.push_back("{");
// 	noport_lst.push_back("server");
// 	noport_lst.push_back("{");
// 	noport_lst.push_back("listen");
// 	noport_lst.push_back("127.0.0.1:");
// 	noport_lst.push_back(";");
// 	noport_lst.push_back("}");
// 	noport_lst.push_back("}");
// 	ConfigParser noport_c(noport_lst, "placeholder_file", "/path/webserv");
// 	try {
// 		config_t noport_conf = noport_c.parse();
// 		assert((false && "noport"));
// 	} catch (const ConfigParser::InvalidValue& e) {
// 		assert((std::strcmp(e.what(), "config: empty port value") == 0));
// 	}
//
// 	list<string> nosemicolon_lst;
// 	nosemicolon_lst.push_back("http");
// 	nosemicolon_lst.push_back("{");
// 	nosemicolon_lst.push_back("server");
// 	nosemicolon_lst.push_back("{");
// 	nosemicolon_lst.push_back("listen");
// 	nosemicolon_lst.push_back("127.0.0.1 4242");
// 	nosemicolon_lst.push_back(";");
// 	nosemicolon_lst.push_back("}");
// 	nosemicolon_lst.push_back("}");
// 	ConfigParser nosemicolon_c(nosemicolon_lst, "placeholder_file", "/path/webserv");
// 	try {
// 		config_t nosemicolon_conf = nosemicolon_c.parse();
// 		assert((false && "nosemicolon"));
// 	} catch (const ConfigParser::InvalidValue& e) {
// 		assert((std::strcmp(e.what(), "config: missing ':' between ip and port") == 0));
// 	}
// }
//
// void test_parse_storage()
// {
// 	list<string> base_lst;
// 	base_lst.push_back("http");
// 	base_lst.push_back("{");
// 	base_lst.push_back("server");
// 	base_lst.push_back("{");
// 	base_lst.push_back("storage");
// 	base_lst.push_back("url");
// 	base_lst.push_back("real/path/");
// 	base_lst.push_back(";");
// 	base_lst.push_back("}");
// 	base_lst.push_back("}");
// 	ConfigParser base_c(base_lst, "placeholder_file", "/path/webserv");
// 	config_t base_conf = base_c.parse();
// 	list<string>::iterator base_it = base_conf.http.server[0].storage.first.begin();
// 	assert((*base_it++ == "/"));
// 	assert((*(base_it++) == "path"));
// 	assert((*(base_it++) == "/"));
// 	assert((*(base_it++) == "url"));
// 	assert((*(base_it) == "/"));
// 	assert((base_conf.http.server[0].storage.second == "/path/real/path/"));
// }
//
// void test_parse_server_inheritance()
// {
// 	list<string> base_lst;
// 	base_lst.push_back("http");
// 	base_lst.push_back("{");
// 	base_lst.push_back("max_body_size");
// 	base_lst.push_back("11");
// 	base_lst.push_back(";");
// 	base_lst.push_back("server");
// 	base_lst.push_back("{");
// 	base_lst.push_back("}");
// 	base_lst.push_back("server");
// 	base_lst.push_back("{");
// 	base_lst.push_back("max_body_size");
// 	base_lst.push_back("44");
// 	base_lst.push_back(";");
// 	base_lst.push_back("}");
// 	base_lst.push_back("}");
// 	ConfigParser base_c(base_lst, "placeholder_file", "/path/webserv");
// 	config_t base_conf = base_c.parse();
// 	assert((base_conf.http.max_body_size == 11));
// 	assert((base_conf.http.server[0].max_body_size == 11));
// 	assert((base_conf.http.server[1].max_body_size == 44));
// }
//
// void test_parse_types()
// {
// 	list<string> base_lst;
// 	base_lst.push_back("http");
// 	base_lst.push_back("{");
// 	base_lst.push_back("types");
// 	base_lst.push_back("{");
// 	base_lst.push_back("text/css");
// 	base_lst.push_back("css");
// 	base_lst.push_back(";");
// 	base_lst.push_back("text/html");
// 	base_lst.push_back("html");
// 	base_lst.push_back("htm");
// 	base_lst.push_back("shtml");
// 	base_lst.push_back(";");
// 	base_lst.push_back("}");
// 	base_lst.push_back("}");
// 	ConfigParser base_c(base_lst, "placeholder_file", "/path/webserv");
// 	config_t base_conf = base_c.parse();
// 	assert((base_conf.http.types.at("css") == "text/css"));
// 	assert((base_conf.http.types.at("html") == "text/html"));
// 	assert((base_conf.http.types.at("htm") == "text/html"));
// 	assert((base_conf.http.types.at("shtml") == "text/html"));
// }

void test_full_text_config(char* exec_path)
{
	// std::cout << exec_path << '\n';
	string path(exec_path);
	path = path.substr(0, path.rfind('/'));
	ConfigParser full_c("test_data/config/full_config", exec_path);
	config_t full_conf = full_c.parse();
	assert((full_conf.events.max_connections == 3));

	assert((full_conf.http.autoindex == false));
	const char* err_50x = "<html>\n"
						  "<head><title>50x Server Error</title></head>\n"
						  "<body>\n"
						  "<center><h1>50x Server Error</h1></center>\n"
						  "<hr><center>webserv/2026</center>\n"
						  "</body>\n"
						  "</html>\n";
	assert((std::strcmp(full_conf.http.error_page[500].c_str(), err_50x) == 0));
	assert((std::strcmp(full_conf.http.error_page[501].c_str(), err_50x) == 0));
	assert((std::strcmp(full_conf.http.error_page[502].c_str(), err_50x) == 0));
	const char* err_400 = "<html>\n"
						  "<head><title>400 Bad Request</title></head>\n"
						  "<body>\n"
						  "<center><h1>400 Bad Request</h1></center>\n"
						  "<hr><center>webserv/2026</center>\n"
						  "</body>\n"
						  "</html>\n";
	assert((std::strcmp(full_conf.http.error_page[400].c_str(), err_400) == 0));
	const char* err_404 = "<html>\n"
						  "<head><title>404 Not Found</title></head>\n"
						  "<body>\n"
						  "<center><h1>404 Not Found</h1></center>\n"
						  "<hr><center>webserv/2026</center>\n"
						  "</body>\n"
						  "</html>\n";
	assert((std::strcmp(full_conf.http.error_page[404].c_str(), err_404) == 0));
	assert((full_conf.http.keepalive_timeout == 10));
	assert((full_conf.http.max_body_size == 1048576));
	assert((std::strcmp(full_conf.http.root.c_str(), (path + "/servers/").c_str()) == 0));
	// Doesn't test all types.
	assert((std::strcmp(full_conf.http.types["html"].c_str(), "text/html") == 0));
	assert((std::strcmp(full_conf.http.types["htm"].c_str(), "text/html") == 0));
	assert((std::strcmp(full_conf.http.types["shtml"].c_str(), "text/html") == 0));
	assert((std::strcmp(full_conf.http.types["deb"].c_str(), "application/octet-stream") == 0));
	assert((std::strcmp(full_conf.http.types["webm"].c_str(), "video/webm") == 0));
	assert((std::strcmp(full_conf.http.default_type.c_str(), "application/octet-stream") == 0));

	server_t server = full_conf.http.server[0];
	assert((server.autoindex == true));
	assert((std::strcmp(server.error_page[404].c_str(), err_404) == 0));
	// Test field inheritance:
	assert((std::strcmp(server.error_page[500].c_str(), err_50x) == 0));
	assert((server.listen[0].ip == 0x7F000001));
	assert((server.listen[0].port == 4242));
	assert((std::strcmp(server.root.c_str(), (path + "/servers/test/html/").c_str()) == 0));
	assert((server.max_body_size == 16384));
	assert((server.redirection.first == 301));
	assert((std::strcmp(server.redirection.second.c_str(), "/newpage") == 0));
	// assert((std::strcmp(server.storage.first.c_str(), "/newpage") == 0));
	std::list<string>::iterator storage_it = server.storage.first.begin();
	assert((std::strcmp((*storage_it++).c_str(), "/") == 0));
	assert((std::strcmp((*storage_it++).c_str(), "data") == 0));
	assert((std::strcmp((*storage_it++).c_str(), "/") == 0));
	assert((std::strcmp((*storage_it++).c_str(), "storage") == 0));
	assert((std::strcmp((*storage_it++).c_str(), "/") == 0));
	assert((std::strcmp(server.storage.second.c_str(), "/real/path/") == 0));

	location_t location = full_conf.http.server[0].locations[0];
	assert((location.exact_match == true));
	std::list<string>::iterator path_it = location.path.begin();
	assert((std::strcmp((*path_it).c_str(), "/") == 0));
	assert((location.autoindex == false));
	assert((location.cgi == false));
	const char* err_403 = "<html>\n"
						  "<head><title>403 Forbidden</title></head>\n"
						  "<body>\n"
						  "<center><h1>403 Forbidden</h1></center>\n"
						  "<hr><center>webserv/2026</center>\n"
						  "</body>\n"
						  "</html>\n";
	assert((std::strcmp(location.error_page[403].c_str(), err_403) == 0));
	assert((std::strcmp(location.index.c_str(), "index.html") == 0));
	assert((location.limit_except[0] == get));
	assert((location.limit_except[1] == del));
	assert((location.redirection.first == 301));
	assert((std::strcmp(location.redirection.second.c_str(), "/newpage") == 0));
	// Test field inheritance:
	assert((std::strcmp(location.root.c_str(), (path + "/servers/test/html/").c_str()) == 0));
}

//
// void test_parse_config()
// {
// 	config_t base_conf;
// 	list<string> base_lst;
// 	base_lst.push_back("events");
// 	base_lst.push_back("{");
// 	base_lst.push_back("max_connections");
// 	base_lst.push_back("257");
// 	base_lst.push_back(";");
// 	base_lst.push_back("}");
// 	std::list<std::string>::iterator base_it = base_lst.begin(),
// 	parse_config(base_conf, base_it, base_lst);
// 	assert((base_conf.events.max_connections == 257));
//
// 	config_t double_conf;
// 	list<string> double_lst;
// 	double_lst.push_back("events");
// 	double_lst.push_back("{");
// 	double_lst.push_back("max_connections");
// 	double_lst.push_back("257");
// 	double_lst.push_back(";");
// 	double_lst.push_back("}");
// 	double_lst.push_back("events");
// 	double_lst.push_back("{");
// 	double_lst.push_back("max_connections");
// 	double_lst.push_back("257");
// 	double_lst.push_back(";");
// 	double_lst.push_back("}");
// 	try {
// 		parse_config(double_conf, double_lst);
// 		assert((false && "double"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: invalid field") == 0));
// 	}
//
// 	config_t novalid_conf;
// 	list<string> novalid_lst;
// 	novalid_lst.push_back("{");
// 	novalid_lst.push_back("max_connections");
// 	novalid_lst.push_back("257");
// 	novalid_lst.push_back(";");
// 	novalid_lst.push_back("}");
// 	novalid_lst.push_back("{");
// 	novalid_lst.push_back("max_connections");
// 	novalid_lst.push_back("257");
// 	novalid_lst.push_back(";");
// 	novalid_lst.push_back("}");
// 	try {
// 		parse_config(novalid_conf, novalid_lst);
// 		assert((false && "novalid"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: invalid field") == 0));
// 	}
// }
//
// void test_extract_boolean_field()
// {
// 	list<string> true_lst;
// 	true_lst.push_back("true");
// 	true_lst.push_back(";");
// 	list<string>::iterator true_it = true_lst.begin();
// 	bool true_b = extract_boolean_field(true_it, true_lst);
// 	assert((true_b == true));
// 	assert((*true_it == ";"));
//
// 	list<string> false_lst;
// 	false_lst.push_back("false");
// 	false_lst.push_back(";");
// 	list<string>::iterator false_it = false_lst.begin();
// 	bool false_b = extract_boolean_field(false_it, false_lst);
// 	assert((false_b == false));
// 	assert((*false_it == ";"));
//
// 	list<string> nobool_lst;
// 	nobool_lst.push_back("nobool");
// 	nobool_lst.push_back(";");
// 	list<string>::iterator nobool_it = nobool_lst.begin();
// 	try {
// 		extract_boolean_field(nobool_it, nobool_lst);
// 		assert((false && "nobool"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: expected boolean") == 0));
// 	}
//
// 	list<string> noval_lst;
// 	noval_lst.push_back(";");
// 	noval_lst.push_back("}");
// 	list<string>::iterator noval_it = noval_lst.begin();
// 	try {
// 		extract_boolean_field(noval_it, noval_lst);
// 		assert((false && "noval"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: expected boolean") == 0));
// 	}
//
// 	list<string> one_lst;
// 	one_lst.push_back("true");
// 	list<string>::iterator one_it = one_lst.begin();
// 	try {
// 		extract_boolean_field(one_it, one_lst);
// 		assert((false && "one"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: unexpected end of file") == 0));
// 	}
// }
//
// void test_parse_autoindex()
// {
// 	config_t true_conf;
// 	list<string> true_lst;
// 	true_lst.push_back("autoindex");
// 	true_lst.push_back("true");
// 	true_lst.push_back(";");
// 	true_lst.push_back("}");
// 	list<string>::iterator true_it = true_lst.begin();
// 	parse_autoindex(true_conf, true_it, true_lst);
// 	assert((true_conf.http.autoindex == true));
// 	assert((*true_it == "}"));
//
// 	config_t nosep_conf;
// 	list<string> nosep_lst;
// 	nosep_lst.push_back("autoindex");
// 	nosep_lst.push_back("true");
// 	nosep_lst.push_back("}");
// 	list<string>::iterator nosep_it = nosep_lst.begin();
// 	try {
// 		parse_autoindex(nosep_conf, nosep_it, nosep_lst);
// 		assert((false && "nosep"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: missing ';'") == 0));
// 	}
//
// 	config_t nokey_conf;
// 	list<string> nokey_lst;
// 	nokey_lst.push_back("true");
// 	nokey_lst.push_back(";");
// 	list<string>::iterator nokey_it = nokey_lst.begin();
// 	try {
// 		parse_autoindex(nokey_conf, nokey_it, nokey_lst);
// 		assert((false && "nokey"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: expected boolean") == 0));
// 	}
//
// 	config_t end_conf;
// 	list<string> end_lst;
// 	list<string>::iterator end_it = end_lst.end();
// 	try {
// 		parse_autoindex(end_conf, end_it, end_lst);
// 		assert((false && "end"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: unexpected end of file") == 0));
// 	}
// }
//
// void test_parse_root()
// {
// 	config_t base_conf;
// 	list<string> base_lst;
// 	base_lst.push_back("root");
// 	base_lst.push_back("/base/");
// 	base_lst.push_back(";");
// 	base_lst.push_back("}");
// 	list<string>::iterator base_it = base_lst.begin();
// 	parse_root(base_conf, base_it, base_lst);
// 	assert((base_conf.http.root == "/base/"));
// 	assert((*base_it == "}"));
//
// 	config_t nobslash_conf;
// 	list<string> nobslash_lst;
// 	nobslash_lst.push_back("root");
// 	nobslash_lst.push_back("base/");
// 	nobslash_lst.push_back(";");
// 	nobslash_lst.push_back("}");
// 	list<string>::iterator nobslash_it = nobslash_lst.begin();
// 	parse_root(nobslash_conf, nobslash_it, nobslash_lst);
// 	assert((nobslash_conf.http.root == "/base/"));
// 	assert((*nobslash_it == "}"));
//
// 	config_t noeslash_conf;
// 	list<string> noeslash_lst;
// 	noeslash_lst.push_back("root");
// 	noeslash_lst.push_back("/base");
// 	noeslash_lst.push_back(";");
// 	noeslash_lst.push_back("}");
// 	list<string>::iterator noeslash_it = noeslash_lst.begin();
// 	parse_root(noeslash_conf, noeslash_it, noeslash_lst);
// 	assert((noeslash_conf.http.root == "/base/"));
// 	assert((*noeslash_it == "}"));
//
// 	config_t noslash_conf;
// 	list<string> noslash_lst;
// 	noslash_lst.push_back("root");
// 	noslash_lst.push_back("base");
// 	noslash_lst.push_back(";");
// 	noslash_lst.push_back("}");
// 	list<string>::iterator noslash_it = noslash_lst.begin();
// 	parse_root(noslash_conf, noslash_it, noslash_lst);
// 	assert((noslash_conf.http.root == "/base/"));
// 	assert((*noslash_it == "}"));
//
// 	config_t eof_conf;
// 	list<string> eof_lst;
// 	eof_lst.push_back("root");
// 	list<string>::iterator eof_it = eof_lst.begin();
// 	try {
// 		parse_root(eof_conf, eof_it, eof_lst);
// 		assert((false && "eof_conf"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: unexpected end of file") == 0));
// 	}
//
// 	config_t eof2_conf;
// 	list<string> eof2_lst;
// 	eof2_lst.push_back("root");
// 	eof2_lst.push_back("/base/");
// 	list<string>::iterator eof2_it = eof2_lst.begin();
// 	try {
// 		parse_root(eof2_conf, eof2_it, eof2_lst);
// 		assert((false && "eof2_conf"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: unexpected end of file") == 0));
// 	}
//
// 	config_t nosep_conf;
// 	list<string> nosep_lst;
// 	nosep_lst.push_back("root");
// 	nosep_lst.push_back("/base/");
// 	nosep_lst.push_back("}");
// 	list<string>::iterator nosep_it = nosep_lst.begin();
// 	try {
// 		parse_root(nosep_conf, nosep_it, nosep_lst);
// 		assert((false && "nosep_conf"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: missing ';'") == 0));
// 	}
// }
//
// void test_parse_max_body_size()
// {
// 	config_t base_conf;
// 	list<string> base_lst;
// 	base_lst.push_back("max_body_size");
// 	base_lst.push_back("257");
// 	base_lst.push_back(";");
// 	base_lst.push_back("}");
// 	list<string>::iterator base_it = base_lst.begin();
// 	parse_max_body_size(base_conf, base_it, base_lst);
// 	assert((base_conf.http.max_body_size == 257));
// 	assert((*base_it == "}"));
//
// 	config_t max_conf;
// 	list<string> max_lst;
// 	max_lst.push_back("max_body_size");
// 	std::stringstream ss;
// 	ss << std::numeric_limits<unsigned int>::max();
// 	max_lst.push_back(ss.str());
// 	max_lst.push_back(";");
// 	max_lst.push_back("}");
// 	list<string>::iterator max_it = max_lst.begin();
// 	parse_max_body_size(max_conf, max_it, max_lst);
// 	assert((max_conf.http.max_body_size == std::numeric_limits<unsigned int>::max()));
// 	assert((*max_it == "}"));
//
// 	config_t zero_conf;
// 	list<string> zero_lst;
// 	zero_lst.push_back("max_body_size");
// 	zero_lst.push_back("0");
// 	zero_lst.push_back(";");
// 	zero_lst.push_back("}");
// 	list<string>::iterator zero_it = zero_lst.begin();
// 	parse_max_body_size(zero_conf, zero_it, zero_lst);
// 	assert((zero_conf.http.max_body_size == 0));
// 	assert((*zero_it == "}"));
//
// 	config_t neg_conf;
// 	list<string> neg_lst;
// 	neg_lst.push_back("max_body_size");
// 	neg_lst.push_back("-1");
// 	neg_lst.push_back(";");
// 	neg_lst.push_back("}");
// 	list<string>::iterator neg_it = neg_lst.begin();
// 	try {
// 		parse_max_body_size(neg_conf, neg_it, neg_lst);
// 		assert((false && "neg"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: max_body_size value can't be negative") == 0));
// 	}
//
// 	config_t bigger_conf;
// 	list<string> bigger_lst;
// 	bigger_lst.push_back("max_body_size");
// 	ss.str("");
// 	ss.clear();
// 	ss << static_cast<unsigned long>(std::numeric_limits<unsigned int>::max()) + 1;
// 	bigger_lst.push_back(ss.str());
// 	bigger_lst.push_back(";");
// 	bigger_lst.push_back("}");
// 	list<string>::iterator bigger_it = bigger_lst.begin();
// 	try {
// 		parse_max_body_size(bigger_conf, bigger_it, bigger_lst);
// 		assert((false && "bigger"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: invalid max_body_size value") == 0));
// 	}
//
// 	config_t eof_conf;
// 	list<string> eof_lst;
// 	eof_lst.push_back("max_body_size");
// 	eof_lst.push_back("257");
// 	list<string>::iterator eof_it = eof_lst.begin();
// 	try {
// 		parse_max_body_size(eof_conf, eof_it, eof_lst);
// 		assert((false && "eof"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: unexpected end of file") == 0));
// 	}
//
// 	config_t nosep_conf;
// 	list<string> nosep_lst;
// 	nosep_lst.push_back("max_body_size");
// 	nosep_lst.push_back("257");
// 	nosep_lst.push_back("}");
// 	list<string>::iterator nosep_it = nosep_lst.begin();
// 	try {
// 		parse_max_body_size(nosep_conf, nosep_it, nosep_lst);
// 		assert((false && "nosep"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(),
// 					"config: missing ';'") == 0));
// 	}
//
// 	config_t nonum_conf;
// 	list<string> nonum_lst;
// 	nonum_lst.push_back("max_body_size");
// 	nonum_lst.push_back("test");
// 	list<string>::iterator nonum_it = nonum_lst.begin();
// 	try {
// 		parse_max_body_size(nonum_conf, nonum_it, nonum_lst);
// 		assert((false && "nonum"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(),
// 					"config: invalid max_body_size value") == 0));
// 	}
// }
//
// void test_parse_keepalive_timeout()
// {
// 	config_t base_conf;
// 	list<string> base_lst;
// 	base_lst.push_back("keepalive_timeout");
// 	base_lst.push_back("257");
// 	base_lst.push_back(";");
// 	base_lst.push_back("}");
// 	list<string>::iterator base_it = base_lst.begin();
// 	parse_keepalive_timeout(base_conf, base_it, base_lst);
// 	assert((base_conf.http.keepalive_timeout == 257));
// 	assert((*base_it == "}"));
//
// 	config_t max_conf;
// 	list<string> max_lst;
// 	max_lst.push_back("keepalive_timeout");
// 	std::stringstream ss;
// 	ss << KEEP_ALIVE_TIMEOUT_MAX;
// 	max_lst.push_back(ss.str());
// 	max_lst.push_back(";");
// 	max_lst.push_back("}");
// 	list<string>::iterator max_it = max_lst.begin();
// 	parse_keepalive_timeout(max_conf, max_it, max_lst);
// 	assert((max_conf.http.keepalive_timeout == KEEP_ALIVE_TIMEOUT_MAX));
// 	assert((*max_it == "}"));
//
// 	config_t zero_conf;
// 	list<string> zero_lst;
// 	zero_lst.push_back("keepalive_timeout");
// 	zero_lst.push_back("0");
// 	zero_lst.push_back(";");
// 	zero_lst.push_back("}");
// 	list<string>::iterator zero_it = zero_lst.begin();
// 	parse_keepalive_timeout(zero_conf, zero_it, zero_lst);
// 	assert((zero_conf.http.keepalive_timeout == 0));
// 	assert((*zero_it == "}"));
//
// 	config_t neg_conf;
// 	list<string> neg_lst;
// 	neg_lst.push_back("keepalive_timeout");
// 	neg_lst.push_back("-1");
// 	neg_lst.push_back(";");
// 	neg_lst.push_back("}");
// 	list<string>::iterator neg_it = neg_lst.begin();
// 	try {
// 		parse_keepalive_timeout(neg_conf, neg_it, neg_lst);
// 		assert((false && "neg"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: keepalive_timeout value can't be negative") == 0));
// 	}
//
// 	config_t bigger_conf;
// 	list<string> bigger_lst;
// 	bigger_lst.push_back("keepalive_timeout");
// 	ss.str("");
// 	ss.clear();
// 	ss << KEEP_ALIVE_TIMEOUT_MAX + 1;
// 	bigger_lst.push_back(ss.str());
// 	bigger_lst.push_back(";");
// 	bigger_lst.push_back("}");
// 	list<string>::iterator bigger_it = bigger_lst.begin();
// 	try {
// 		parse_keepalive_timeout(bigger_conf, bigger_it, bigger_lst);
// 		assert((false && "bigger"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: invalid keepalive_timeout value") == 0));
// 	}
//
// 	config_t eof_conf;
// 	list<string> eof_lst;
// 	eof_lst.push_back("keepalive_timeout");
// 	eof_lst.push_back("257");
// 	list<string>::iterator eof_it = eof_lst.begin();
// 	try {
// 		parse_keepalive_timeout(eof_conf, eof_it, eof_lst);
// 		assert((false && "eof"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(), "config: unexpected end of file") == 0));
// 	}
//
// 	config_t nosep_conf;
// 	list<string> nosep_lst;
// 	nosep_lst.push_back("keepalive_timeout");
// 	nosep_lst.push_back("257");
// 	nosep_lst.push_back("}");
// 	list<string>::iterator nosep_it = nosep_lst.begin();
// 	try {
// 		parse_keepalive_timeout(nosep_conf, nosep_it, nosep_lst);
// 		assert((false && "nosep"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(),
// 					"config: missing ';'") == 0));
// 	}
//
// 	config_t nonum_conf;
// 	list<string> nonum_lst;
// 	nonum_lst.push_back("keepalive_timeout");
// 	nonum_lst.push_back("test");
// 	list<string>::iterator nonum_it = nonum_lst.begin();
// 	try {
// 		parse_keepalive_timeout(nonum_conf, nonum_it, nonum_lst);
// 		assert((false && "nonum"));
// 	} catch (const std::runtime_error& e) {
// 		assert((std::strcmp(e.what(),
// 					"config: invalid keepalive_timeout value") == 0));
// 	}
// }
int main(int argc, char *argv[])
{
	if (argc != 1)
		return (1);
	// TEST(test_parse_max_connections);
	// TEST(test_parse_events);
	// TEST(test_parse_http);
	// TEST(test_parse_listen);
	// TEST(test_parse_storage);
	// TEST(test_parse_server_inheritance);
	// TEST(test_parse_types);
	TEST_STR(test_full_text_config, argv[0]);
	// TEST(test_parse_config);
	// TEST(test_extract_boolean_field);
	// TEST(test_parse_autoindex);
	// TEST(test_parse_root);
	// TEST(test_parse_max_body_size);
	// TEST(test_parse_keepalive_timeout);
	return (0);
}
