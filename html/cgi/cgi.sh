#!/usr/bin/env bash

#sleep 15

# Read entire stdin
input="$(cat)"

# Build body
body="<!DOCTYPE html>
<html>
<body>
<pre>
$input
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
# printf "Content-Length: %d
# " "$length"
# printf "Connection: close
# "
# printf "
# "

# Emit body
printf "%s" "$body"

exit 0
