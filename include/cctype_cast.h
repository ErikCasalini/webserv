#ifndef CCTYPE_CAST_H
# define CCTYPE_CAST_H

// Wrappers to do the required cast to unsigned char
// to avoid undefined behavior with negative values
// as this functions treat their argument as an int.
int to_lowercase(char c);
int is_graph(char c);
int is_space(char c);

#endif
