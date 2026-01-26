#include "../include/cctype_cast.h"
#include "lib_test.h"

void test_strtrim()
{
	// correct inputs
	std::string full = "str";
	strtrim(full);
	assert(full == "str");

	std::string lead = "    str";
	strtrim(lead);
	assert(lead == "str");

	std::string trail = "str    ";
	strtrim(trail);
	assert(trail == "str");

	std::string both = "    str    ";
	strtrim(both);
	assert(both == "str");

	std::string middle = "st r";
	strtrim(middle);
	assert(middle == "st r");

	std::string empty = "";
	strtrim(empty);
	assert(empty == "");
}

int main(void)
{
	test(test_strtrim, "test_strtrim");
	return (0);
}
