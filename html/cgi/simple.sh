#!/usr/bin/env bash

# Build body
ENVIRONMENT=$(env)

body="<!DOCTYPE html>
<html>
<body>
<pre>
ENVIRONMENT:

$ENVIRONMENT
</pre>
</body>
</html>"

#sleep 15

# Compute byte length (no trailing newline)
length=$(printf "%s" "$body" | wc -c)

# Emit full HTTP response
echo -en "Status: 200 OK\r\n"
echo -en "Content-Type: text/html\r\n"
echo -en "\r\n"

# Emit body
printf "%s" "$body"

exit 0
