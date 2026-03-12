#!/usr/bin/env bash

BOLD="\033[1m"
RESET="\033[0m"

function test() {
	echo -e "${BOLD}TEST: ${1}${RESET}"
}

for i in ../test_data/config/wrong_configs/*; do
	test ${i}
	valgrind --quiet ./../../webserv ${i}
	echo
done
