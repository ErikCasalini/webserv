#include "config_parser.h"
#include <cstdlib>
#include <limits>
#include <stdexcept>

using std::string;

typedef void (*parse_func)(config_t&, std::list<string>::iterator&, std::list<string>&);

static void advance_tok(
		std::list<string>::iterator& tok_it,
		std::list<string>& tokens)
{
	if (tok_it == tokens.end())
		throw std::runtime_error("config: unexpected end of file");
	if (++tok_it == tokens.end())
		throw std::runtime_error("config: unexpected end of file");
}

static void consume_symbol(
		string s,
		std::list<string>::iterator& tok_it,
		std::list<string>& tokens)
{
	if (tok_it == tokens.end())
		throw std::runtime_error("config: unexpected end of file");
	if (*tok_it != s) {
		string error = "config: missing '";
		error += s;
		error += '\'';
		throw std::runtime_error(error.c_str());
	}
	if (++tok_it == tokens.end())
		throw std::runtime_error("config: unexpected end of file");
}

void parse_max_connections(
		config_t& config,
		std::list<string>::iterator& tok_it,
		std::list<string>& tokens)
{
	advance_tok(tok_it, tokens);
	if ((*tok_it).at(0) == '-')
		throw std::runtime_error("config: max_connections can't be negative");
	char* end = NULL;
	errno = 0;
	const unsigned long n = std::strtoul((*tok_it).c_str(), &end, 10);
	if (*end || errno
			|| n > CONNECTIONS_MAX || n < CONNECTIONS_MIN
			|| n > std::numeric_limits<unsigned int>::max()) {
		errno = 0;
		throw std::runtime_error("config: invalid max_connections value");
	}
	config.events.max_connections = static_cast<unsigned int>(n);
	advance_tok(tok_it, tokens);
	consume_symbol(";", tok_it, tokens);
}

void parse_events(
		config_t& config,
		std::list<string>::iterator& tok_it,
		std::list<string>& tokens)
{
	advance_tok(tok_it, tokens);
	consume_symbol("{", tok_it, tokens);
	std::map<string, parse_func> events_map;
	events_map["max_connections"] = parse_max_connections;
	try {
		while (tok_it != tokens.end() && *tok_it != "}") {
			const std::list<string>::iterator tmp_it = tok_it;
			events_map.at(*tok_it)(config, tok_it, tokens);
			events_map.erase(*tmp_it);
		}
	} catch (const std::out_of_range& e) {
		throw std::runtime_error("config: invalid field");
	}
	// Manually iter because we can be at the end of the list after events.
	if (tok_it == tokens.end())
		throw std::runtime_error("config: unexpected end of file");
	if (*tok_it != "}")
		throw std::runtime_error("config: missing '");
	++tok_it;
}

// Check for tok_it.end() before and after iteration.
bool extract_boolean_field(
		std::list<string>::iterator& tok_it,
		std::list<string>& tokens)
{
	if (*tok_it == "true") {
		advance_tok(tok_it, tokens);
		return (true);
	} else if (*tok_it == "false") {
		advance_tok(tok_it, tokens);
		return (false);
	} else {
		throw std::runtime_error("config: expected boolean");
	}
}

void parse_autoindex(
		config_t& config,
		std::list<string>::iterator& tok_it,
		std::list<string>& tokens)
{
	advance_tok(tok_it, tokens);
	config.http.autoindex = extract_boolean_field(tok_it, tokens);
	consume_symbol(";", tok_it, tokens);
}

void parse_root(
		config_t& config,
		std::list<string>::iterator& tok_it,
		std::list<string>& tokens)
{
	advance_tok(tok_it, tokens);
	string path = *tok_it;
	if (path.at(0) != '/')
		path.insert(0, "/");
	if (path.at(path.length() - 1) != '/')
		path.append("/");
	advance_tok(tok_it, tokens);
	config.http.root = path;
	consume_symbol(";", tok_it, tokens);
}

void parse_http(
		config_t& config,
		std::list<string>::iterator& tok_it,
		std::list<string>& tokens)
{
	advance_tok(tok_it, tokens);
	consume_symbol("{", tok_it, tokens);

	std::map<string, parse_func> http_map;
	// http_map["server"] = parse_server;
	http_map["root"] = parse_root;
	http_map["autoindex"] = parse_autoindex;
	// http_map["error_page"] = parse_error_page;
	// http_map["max_body"] = parse_max_body;
	// http_map["keepalive_timeout"] = parse_keepalive_timeout;

	try {
		while (tok_it != tokens.end()) {
			const std::list<string>::iterator tmp_it = tok_it;
			http_map.at(*tok_it)(config, tok_it, tokens);
			// We can have an arbitrary number of servers
			if (*tok_it != "server")
				http_map.erase(*tmp_it);
		}
	} catch (const std::out_of_range& e) {
		throw std::runtime_error("config: invalid field");
	}

	// Manually iter because we can be at the end of the list after http.
	if (*tok_it != "}")
		throw std::runtime_error("config: missing '}'");
	++tok_it;
}

void parse_main(config_t& config, std::list<string>& tokens)
{
	std::map<string, parse_func> main_map;
	main_map["events"] = parse_events;
	main_map["http"] = parse_http;

	std::list<string>::iterator tok_it = tokens.begin();
	try {
		while (tok_it != tokens.end()) {
			const std::list<string>::iterator tmp_it = tok_it;
			main_map.at(*tok_it)(config, tok_it, tokens);
			main_map.erase(*tmp_it);
		}
	} catch (const std::out_of_range& e) {
		throw std::runtime_error("config: invalid field");
	}
}

// the name of the config file is given as an argument to the program
config_t parse_config(const string& file)
{
	std::ifstream raw(file.c_str());
	if (!raw.good())
		throw std::runtime_error("config: error trying to open config file");

	std::list<string> tokens = preprocess(raw);

	config_t config;
	parse_main(config, tokens);
	return (config);
}
