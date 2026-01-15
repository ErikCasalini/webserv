#! /bin/sh

SRC="src/"
TEST="test/"

compile_test() {
	c++ -std=c++98 -g3 -Wall -Werror -Wextra "$@" "${TEST}lib_test.cpp"
}

run_test() {
	compile_test "$@" && ./a.out && rm a.out
}

run_test "${SRC}request_parser.cpp" "${TEST}test_request_parser.cpp"
