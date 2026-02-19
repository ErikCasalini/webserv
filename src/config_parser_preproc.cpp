#include "config_parser.h"
#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <algorithm>

using std::string;

namespace _preproc {
	static size_t consume_spaces(const string& str, size_t pos)
	{
		while (str[pos] == ' ' || str[pos] == '\t')
			++pos;
		return (pos);
	}

	static size_t consume_spaces_nl(const string& str, size_t pos)
	{
		while (str[pos] == ' ' || str[pos] == '\t' || str[pos] == '\n')
			++pos;
		return (pos);
	}

	// static string file_to_string(const string& filename)
	// {
	// 	std::ifstream file(filename.c_str());
	// 	if (!file.good())
	// 		throw std::runtime_error("file_to_string: error trying to open file");
	// 	std::stringstream content;
	// 	content << file.rdbuf();
	// 	return (content.str());
	// }
	//
	static void append_file_to_sstream(const string& filename, std::stringstream& sstream)
	{
		std::ifstream file(filename.c_str());
		if (!file.good())
			throw std::runtime_error("file_to_string: error trying to open file");
		sstream << file.rdbuf();
	}

	// Remove comments and replace each newlines by a space
	string remove_comments_nl(std::istream& config)
	{
		string pre;
		for (string line; std::getline(config, line); ) {
			size_t pos = 0;
			pos = consume_spaces(line, pos);
			if (pos == line.length() || line[pos] == '#')
				continue;
			// line.erase(0, pos);
			string::size_type f = line.find_first_of("#");
			if (f == string::npos)
				pre.append(line);
			else
				pre.append(line.substr(0, f));
			pre.append(" ");
		}
		return (pre);
	}

	std::list<string> tokenize_config(const string& config)
	{
		std::list<string> tokens;
		string::size_type start_pos = 0;
		string::size_type end_pos = 0;
		while (start_pos < config.length()) {
			end_pos = config.find_first_of("{}; \t\n", start_pos);
			if (end_pos == string::npos) {
				tokens.push_back(config.substr(start_pos));
				return (tokens);
			}
			if (start_pos != end_pos) {
				tokens.push_back(config.substr(start_pos, end_pos - start_pos));
				start_pos = end_pos;
			}
			if (config.at(start_pos) != ' '
					&& config.at(start_pos) != '\t'
					&& config.at(start_pos) != '\n' ) {
				tokens.push_back(config.substr(start_pos, 1));
				++start_pos;
			}
			start_pos = consume_spaces_nl(config, start_pos);
		}
		return (tokens);
	}

	void expand_includes(std::list<string>& config)
	{
		std::list<string>::iterator it;
		while ((it = std::find(config.begin(), config.end(), "include"))
				!= config.end()) {
			std::list<string>::iterator path_it = it;
			++path_it;
			if (path_it == config.end())
				throw std::runtime_error("config: unexpected end of file");
			string include_path = *path_it;
			// Load the include file content.
			std::stringstream s_content;
			try {
				append_file_to_sstream(include_path, s_content);
			} catch (const std::exception& e) {
				throw std::runtime_error("config: error trying to open include file");
			}

			string content = remove_comments_nl(s_content);
			std::list<string> tokens = tokenize_config(content);
			config.insert(it, tokens.begin(), tokens.end());

			// Remove the two include nodes
			config.erase(it);
			config.erase(path_it);
		}
	}
}

// Remove comments, expand includes and tokenize
std::list<string> preprocess(std::ifstream& config)
{
	string clean = _preproc::remove_comments_nl(config);
	if (clean.length() == 0)
		throw std::runtime_error("config: empty config file");
	std::list<string> pre = _preproc::tokenize_config(clean);
	_preproc::expand_includes(pre);
	return (pre);
}
