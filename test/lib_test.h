#ifndef LIB_TEST_H
# define LIB_TEST_H

# include <cassert>
# include <string>

# define RESET "\033[0m"
# define GREEN "\033[32m"
# define RED "\033[31m"

void test(void (*func)(), const std::string& test_name);

#endif
