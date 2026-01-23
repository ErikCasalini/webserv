#! /bin/sh

SRC="src/"
TEST="test/"
LIB="src/lib/"

compile_test() {
	c++ -std=c++98 -g3 -Wall -Werror -Wextra "$@" "${TEST}lib_test.cpp"
}

run_test() {
	compile_test "$@" && ./a.out && rm a.out
}

run_test "${SRC}request_parser.cpp" "${LIB}cctype_cast.cpp" "${TEST}test_request_parser.cpp"
run_test "${LIB}cctype_cast.cpp" "${TEST}test_lib_cctype_cast.cpp"
