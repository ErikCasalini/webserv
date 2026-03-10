#include "lib_test.h"
#include <iostream>

void test(void (*func)(), const std::string& test_name)
{
	func();
	std::cout <<  test_name  << GREEN " [OK]" RESET "\n";
}

void test_str(void (*func)(char*), char* arg, const std::string& test_name)
{
	func(arg);
	std::cout <<  test_name  << GREEN " [OK]" RESET "\n";
}
