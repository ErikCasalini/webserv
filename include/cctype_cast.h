#ifndef CCTYPE_CAST_H
# define CCTYPE_CAST_H

# include <string>

// Wrappers to do the required cast to unsigned char
// to avoid undefined behavior with negative values
// as this functions treat their argument as an int.
int to_lower(char c);
int is_graph(char c);
int is_space(char c);
int is_digit(char c);

// Extensions for http parsing
int is_token(char c);
void strltrim(std::string& str, int (*func)(char));
void strrtrim(std::string& str, int (*func)(char));
void strtrim(std::string& str, int (*func)(char) = is_space);

#endif
