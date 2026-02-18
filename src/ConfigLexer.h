#ifndef CONFIG_LEXER_H
# define CONFIG_LEXER_H

# include <string>
# include <fstream>
# include <list>

class ConfigLexer {
public:
	ConfigLexer(const std::string& filename);

	std::list<std::string> lex();
private:
	ConfigLexer();
	std::string m_filename;
};

std::list<std::string> lex(std::ifstream& config);

namespace _config_lexer {
	std::string remove_comments_nl(std::istream& config);
	void expand_includes(std::list<std::string>& config);
	std::list<std::string> tokenize_config(const std::string& config);
}

#endif
