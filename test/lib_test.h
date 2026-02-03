#ifndef LIB_TEST_H
# define LIB_TEST_H

# include <cassert>
# include <string>

# define RESET "\033[0m"
# define GREEN "\033[32m"
# define RED "\033[31m"

void test(void (*func)(), const std::string& test_name);

// Permit to call test() without giving redundant args:
// e.g. test(funcname, "funcname") => TEST(funcname)
# define TEST(F) test(F, #F)

#endif
