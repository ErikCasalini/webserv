#include "ConfigParser.h"
#include "ConfigLexer.h"
#include "Response.hpp"
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <fstream>

using std::string;

void ConfigParser::advance()
{
	if (m_tok_it == m_tokens.end())
		throw ConfigParser::UnexpectedEof();
	if (++m_tok_it == m_tokens.end())
		throw ConfigParser::UnexpectedEof();
}

void ConfigParser::advance(string symbol)
{
	if (m_tok_it == m_tokens.end())
		throw ConfigParser::UnexpectedEof();
	if (*m_tok_it != symbol)
		throw ConfigParser::UnexpectedToken(*m_tok_it);
	if (++m_tok_it == m_tokens.end())
		throw ConfigParser::UnexpectedEof();
}

void ConfigParser::consume()
{
	if (m_tok_it == m_tokens.end())
		throw ConfigParser::UnexpectedEof();
	m_tokens.erase(m_tok_it++);
	if (m_tok_it == m_tokens.end())
		throw ConfigParser::UnexpectedEof();
}

void ConfigParser::consume(string symbol)
{
	if (m_tok_it == m_tokens.end())
		throw ConfigParser::UnexpectedEof();
	if (*m_tok_it != symbol)
		throw ConfigParser::UnexpectedToken(*m_tok_it);
	m_tokens.erase(m_tok_it++);
	if (m_tok_it == m_tokens.end())
		throw ConfigParser::UnexpectedEof();
}

unsigned long ConfigParser::value_to_ul(
				const string& value,
				unsigned long min,
				unsigned long max,
				string error_scope)
{
	if (value.at(0) == '-')
		throw ConfigParser::InvalidValue(
			error_scope + " value can't be negative '" + value + "'");
	char* end = NULL;
	errno = 0;
	const unsigned long n = std::strtoul(value.c_str(), &end, 10);
	if (*end || errno || n < min || n > max) {
		errno = 0;
		throw ConfigParser::InvalidValue(
				"invalid " + error_scope + " value '" + value + "'");
	}
	return (n);
}

bool ConfigParser::extract_boolean()
{
	if (*m_tok_it == "true")
		return (true);
	else if (*m_tok_it == "false")
		return (false);
	else
		throw ConfigParser::InvalidValue("expected boolean");
}

unsigned int ConfigParser::parse_max_connections()
{
	consume();
	const unsigned long n = value_to_ul(
								*m_tok_it,
								CONNECTIONS_MIN,
								CONNECTIONS_MAX,
								"max_connections");
	consume();
	consume(";");
	return (static_cast<unsigned int>(n));
}

void ConfigParser::parse_events()
{
	consume();
	consume("{");
	while (m_tok_it != m_tokens.end()) {
		if (*m_tok_it == "max_connections") {
			m_config.events.max_connections
				= parse_max_connections();
		} else if (*m_tok_it == "}") {
			m_tokens.erase(m_tok_it++);
			return;
		} else {
			throw ConfigParser::UnexpectedToken(*m_tok_it);
		}
	}
	throw ConfigParser::UnexpectedEof();
}

bool ConfigParser::parse_autoindex()
{
	consume();
	bool val = extract_boolean();
	consume();
	consume(";");
	return (val);
}

std::list<string>::iterator ConfigParser::get_error_filename_pos()
{
	if (m_tok_it == m_tokens.end())
		throw ConfigParser::UnexpectedEof();
	std::list<string>::iterator it = m_tok_it;
	while (*it != ";")
		++it;
	return (--it);
}

static string error_file_to_string(const string& filename)
{
	std::ifstream file(filename.c_str());
	if (!file.good())
		throw std::runtime_error("config: error trying to open error file");
	std::stringstream s_content;
	s_content << file.rdbuf();
	string content = s_content.str();
	if (content.length() > ERROR_PAGE_LEN_MAX)
		throw std::runtime_error("config: error file too big");
	return (content);
}

// TODO(PERF): Avoid error pages duplicates in config_t.
// Make it a class and store all the errors pages in a map
// with their path as a key and content as value.
// Make the error_pages fields in config_t be pointers to the
// corresponding errors content.
void ConfigParser::parse_error_page(error_page_t& error_page)
{
	consume();
	std::list<string>::iterator filename_it
		= get_error_filename_pos();
	std::vector<int> err_nums;
	while (m_tok_it != filename_it) {
		const unsigned long n = value_to_ul(
									*m_tok_it,
									0,
									999,
									"error_page error number");
		err_nums.push_back(static_cast<int>(n));
		consume();
	}
	const string filename = config_files::resolve_include_path(*filename_it, m_conf_path);
	string page = error_file_to_string(filename);
	consume();
	for (size_t i = 0; i < err_nums.size(); ++i) {
		if (error_page.size() > ERROR_PAGE_MAX)
			throw ConfigParser::InvalidValue(
					"too many error_pages number");
		// TODO: PERF: only load the file once and make all the numbers point to the same file
		error_page[err_nums[i]] = page;
	}
	consume(";");
}

unsigned int ConfigParser::parse_keepalive_timeout()
{
	consume();
	const unsigned long n = value_to_ul(
								*m_tok_it,
								0,
								KEEP_ALIVE_TIMEOUT_MAX,
								"keepalive_timeout");
	consume();
	consume(";");
	return (static_cast<unsigned int>(n));
}

unsigned int ConfigParser::parse_max_body_size()
{
	consume();
	const unsigned long n = value_to_ul(
								*m_tok_it,
								0,
								std::numeric_limits<unsigned int>::max(),
								"max_body_size");
	consume();
	consume(";");
	return (static_cast<unsigned int>(n));
}

string ConfigParser::prepare_path_slicing(string path)
{
	if (path.at(0) != '/')
		path.insert(0, "/");
	if (path.at(path.length() - 1) != '/')
		path.append("/");
	return (path);
}

string ConfigParser::parse_root()
{
	consume();
	string path = prepare_path_slicing(*m_tok_it);
	consume();
	consume(";");
	return (path);
}

server_t ConfigParser::inherit_from_http()
{
	server_t server;
	server.autoindex = m_config.http.autoindex;
	server.error_page = m_config.http.error_page;
	server.max_body_size = m_config.http.max_body_size;
	server.root = m_config.http.root;
	return (server);
}

void ConfigParser::skip_block()
{
	int depth = 0;
	// location can contain one or two more tokens
	// ('=' and path) than other blocks.
	if (*m_tok_it == "location") {
		advance();
		if (*m_tok_it == "=")
			advance();
	}
	advance();
	advance("{");
	++depth;
	while (depth > 0) {
		if (*m_tok_it == "{")
			++depth;
		else if (*m_tok_it == "}")
			--depth;
		advance();
	}
}

u_int32_t ConfigParser::ip_to_ui32(string ip)
{
	if (ip.length() < 1)
		throw ConfigParser::InvalidValue("empty ip value");
	u_int32_t val = 0;
	string::size_type start = 0;
	string o;

	for (int i = 0; i < 4; ++i) {
		if (i < 3) {
			string::size_type end = ip.find(".", start);
			o = ip.substr(start, end - start);
			start = end + 1;
		} else {
			o = ip.substr(start);
		}
		if (o.length() < 1)
			throw ConfigParser::InvalidValue("empty ip octet");
		unsigned long octet = value_to_ul(o, 0, 127, "ip");
		val = (val << 8) | static_cast<u_int32_t>(octet);
	}
	return (val);
}

u_int16_t ConfigParser::port_to_ui16(string port)
{
	if (port.length() < 1)
		throw ConfigParser::InvalidValue("empty port value");
	unsigned long octet = value_to_ul(port, 0, 65535, "port");
	return (static_cast<u_int16_t>(octet));
}

listen_t ConfigParser::parse_listen()
{
	consume();
	listen_t listen;
	string::size_type sep = (*m_tok_it).find(":");
	if (sep == string::npos)
		throw ConfigParser::InvalidValue("missing ':' between ip and port");
	string ip = (*m_tok_it).substr(0, sep);
	listen.ip = ip_to_ui32(ip);
	string port = (*m_tok_it).substr(sep + 1);
	listen.port = port_to_ui16(port);
	consume();
	consume(";");
	return (listen);
}

upload_t ConfigParser::parse_upload()
{
	upload_t upload;
	consume();
	string url = prepare_path_slicing(*m_tok_it);
	upload.first = _Response::split_path(url);
	consume();
	upload.second = prepare_path_slicing(*m_tok_it);
	consume();
	consume(";");
	return (upload);
}

redirection_t ConfigParser::parse_redirection()
{
	redirection_t redirection;
	consume();
	unsigned long status = value_to_ul(*m_tok_it, 301, 302, "redirect status");
	switch (status) {
		case 301:
			redirection.first = moved_perm;
			break;
		case 302:
			redirection.first = moved_temp;
			break;
		default:
			throw ConfigParser::InvalidValue("redirection status not handled");
	}
	consume();
	redirection.second = *m_tok_it;
	consume();
	consume(";");
	return (redirection);
}

location_t ConfigParser::inherit_loc_from_server(const server_t& server)
{
	location_t location;
	location.autoindex = server.autoindex;
	location.error_page = server.error_page;
	location.redirection = server.redirection;
	location.root = server.root;
	return (location);
}

void ConfigParser::parse_exact_match(location_t& location)
{
	if (*m_tok_it == "=") {
		location.exact_match = true;
		consume();
	} else {
		location.exact_match = false;
	}
}

bool ConfigParser::parse_cgi()
{
	consume();
	bool val = extract_boolean();
	consume();
	consume(";");
	return (val);
}

string ConfigParser::parse_index()
{
	consume();
	if ((*m_tok_it).find("\\") != string::npos)
		throw ConfigParser::InvalidValue("index path cannot contain '\\'");
	string index = *m_tok_it;
	consume();
	consume(";");
	return (index);
}

std::vector<method_t> ConfigParser::parse_limit_except()
{
	consume();
	std::vector<method_t> except;
	int i = 0;
	while (*m_tok_it != ";") {
		if (i > 3)
			throw ConfigParser::InvalidValue("too many methods");
		if (*m_tok_it == "GET")
			except.push_back(get);
		else if (*m_tok_it == "POST")
			except.push_back(post);
		else if (*m_tok_it == "DELETE")
			except.push_back(del);
		else
			throw ConfigParser::InvalidValue("invalid method");
		consume();
		++i;
	}
	consume(";");
	return (except);
}

string ConfigParser::parse_default_type()
{
	consume();
	string type = *m_tok_it;
	consume();
	consume(";");
	return (type);
}

void ConfigParser::parse_type(std::map<string, string>& types)
{
	string type = *m_tok_it;
	consume();
	while (m_tok_it != m_tokens.end()) {
		if (*m_tok_it == ";") {
			consume();
			return;
		}
		types[*m_tok_it] = type;
		consume();
	}
}

std::map<string, string> ConfigParser::parse_types()
{
	consume();
	consume("{");
	std::map<string, string> types;
	while (m_tok_it != m_tokens.end()) {
		if (*m_tok_it == "}") {
			consume();
			return (types);
		}
		parse_type(types);
	}
	throw ConfigParser::UnexpectedEof();
}

void ConfigParser::parse_location(location_t& location)
{
	consume();
	parse_exact_match(location);
	string path = prepare_path_slicing(*m_tok_it);
	location.path = _Response::split_path(path);
	consume();
	consume("{");
	while (m_tok_it != m_tokens.end()) {
		if (*m_tok_it == "autoindex") {
			location.autoindex = parse_autoindex();
		} else if (*m_tok_it == "cgi") {
			location.cgi = parse_cgi();
		} else if (*m_tok_it == "error_page") {
			parse_error_page(location.error_page);
		} else if (*m_tok_it == "index") {
			location.index = parse_index();
		} else if (*m_tok_it == "limit_except") {
			location.limit_except = parse_limit_except();
		} else if (*m_tok_it == "return") {
			location.redirection = parse_redirection();
		} else if (*m_tok_it == "root") {
			location.root = parse_root();
		} else if (*m_tok_it == "}") {
			consume();
			// If no limit_except set we allow all methods.
			if (location.limit_except.size() == 0) {
				location.limit_except.push_back(get);
				location.limit_except.push_back(post);
				location.limit_except.push_back(del);
			}
			return;
		} else {
			throw ConfigParser::UnexpectedToken(*m_tok_it);
		}
	}
	throw ConfigParser::UnexpectedEof();
}

void ConfigParser::parse_locations(server_t& server)
{
	// Enter into server block.
	// Delete tokens while parsing as the remaining expected content is location blocks.
	consume();
	consume("{");
	std::vector<location_t>::size_type i = 0;
	while (m_tok_it != m_tokens.end()) {
		if (*m_tok_it == "location") {
			server.locations.push_back(inherit_loc_from_server(server));
			parse_location(server.locations.at(i));
			++i;
		} else if (*m_tok_it == "}") {
			consume();
			return;
		} else {
			throw ConfigParser::UnexpectedToken(*m_tok_it);
		}
	}
	throw ConfigParser::UnexpectedEof();
}

server_t ConfigParser::parse_server()
{
	advance();
	advance("{");
	server_t server = inherit_from_http();
	while (m_tok_it != m_tokens.end()) {
		if (*m_tok_it == "autoindex") {
			server.autoindex = parse_autoindex();
		} else if (*m_tok_it == "error_page") {
			parse_error_page(server.error_page);
		} else if (*m_tok_it == "listen") {
			server.listen.push_back(parse_listen());
		} else if (*m_tok_it == "location") {
			skip_block();
		} else if (*m_tok_it == "max_body_size") {
			server.max_body_size = parse_max_body_size();
		} else if (*m_tok_it == "return") {
			server.redirection = parse_redirection();
		} else if (*m_tok_it == "root") {
			server.root = parse_root();
		} else if (*m_tok_it == "upload") {
			server.upload = parse_upload();
		} else if (*m_tok_it == "}") {
			++m_tok_it;
			return (server);
		} else {
			throw ConfigParser::UnexpectedToken(*m_tok_it);
		}
	}
	throw ConfigParser::UnexpectedEof();
}

void ConfigParser::parse_http()
{
	advance();
	advance("{");
	std::vector<server_t>::size_type num_server = 0;
	while (m_tok_it != m_tokens.end()) {
		switch (m_depth) {
			case 1:
				if (*m_tok_it == "autoindex") {
					m_config.http.autoindex = parse_autoindex();
				} else if (*m_tok_it == "error_page") {
					parse_error_page(m_config.http.error_page);
				} else if (*m_tok_it == "keepalive_timeout") {
					m_config.http.keepalive_timeout = parse_keepalive_timeout();
				} else if (*m_tok_it == "max_body_size") {
					m_config.http.max_body_size = parse_max_body_size();
				} else if (*m_tok_it == "root") {
					m_config.http.root = parse_root();
				} else if (*m_tok_it == "default_type") {
					m_config.http.default_type = parse_default_type();
				} else if (*m_tok_it == "server") {
					skip_block();
				} else if (*m_tok_it == "types") {
					skip_block();
				} else if (*m_tok_it == "}") {
					++m_tok_it;
					return;
				} else {
					throw ConfigParser::UnexpectedToken(*m_tok_it);
				}
				break;
			case 2:
				if (*m_tok_it == "server") {
					m_config.http.server.push_back(parse_server());
				// ERASE BLOCK AFTER PARSING
				} else if (*m_tok_it == "types") {
					m_config.http.types = parse_types();
				} else if (*m_tok_it == "}") {
					++m_tok_it;
					return;
				} else {
					throw ConfigParser::UnexpectedToken(*m_tok_it);
				}
				break;
			case 3:
				if (*m_tok_it == "server") {
					parse_locations(m_config.http.server[num_server]);
					++num_server;
				} else if (*m_tok_it == "}") {
					++m_tok_it;
					return;
				} else {
					throw ConfigParser::UnexpectedToken(*m_tok_it);
				}
				break;
			default:
				if (m_tok_it == m_tokens.end())
					throw ConfigParser::UnexpectedEof();
				else
					throw ConfigParser::UnexpectedToken(*m_tok_it);
		}
	}
	throw ConfigParser::UnexpectedEof();
}

// the name of the config file is given as an argument to the program
config_t ConfigParser::parse()
{
	while (m_depth <= 3) {
		while (m_tok_it != m_tokens.end()) {
			if (*m_tok_it == "http") {
				parse_http();
			// events blocks are removed as their as parsed
			// we don't need to track depth here.
			} else if (*m_tok_it == "events") {
				parse_events();
			} else {
				throw ConfigParser::UnexpectedToken(*m_tok_it);
			}
		}
		++m_depth;
		m_tok_it = m_tokens.begin();
	}
	return (m_config);
}

ConfigParser::ConfigParser(const std::list<std::string>& tokens, string conf_path)
	: m_tokens(tokens)
	, m_tok_it(m_tokens.begin())
	, m_depth(1)
	, m_conf_path(conf_path)
{};

ConfigParser::UnexpectedToken::UnexpectedToken(string msg)
	: std::runtime_error(
		("config: unexpected token '" + msg + "'").c_str())
{};
ConfigParser::UnexpectedEof::UnexpectedEof()
	: std::runtime_error("config: unexpected end of file")
{};
ConfigParser::InvalidValue::InvalidValue(string msg)
	: std::runtime_error(("config: " + msg).c_str())
{};
