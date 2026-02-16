#! /usr/bin/env bash

echo "I'm the cgi. I have this env:"
export

echo "And here is the body I received:"
if [[ "${CONTENT_LENGTH}" -gt 0 ]]; then
	# read -r -N "${CONTENT_LENGTH}" R
	read -r R
	echo "$R"
fi
