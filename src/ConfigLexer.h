#ifndef CONFIG_LEXER_H
# define CONFIG_LEXER_H

# include <string>
# include <list>

class ConfigLexer {
public:
	ConfigLexer(const std::string& filename);

	std::list<std::string> lex();
private:
	ConfigLexer();
	std::string m_filename;
};

namespace _config_lexer {
	std::string remove_comments_nl(std::istream& config);
	std::list<std::string> tokenize_config(const std::string& config);
	void expand_includes(std::list<std::string>& config, std::string filename);
}

namespace config_files {
	std::string extract_path_part(std::string filepath);
	std::string resolve_include_path(std::string incpath, std::string filepath);
}

#endif
