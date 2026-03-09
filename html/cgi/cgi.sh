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
printf "HTTP/1.0 200 OK
"
printf "Content-Type: text/html
"
printf "Content-Length: %d
" "$length"
printf "Connection: close
"
printf "
"

# Emit body
printf "%s" "$body"

exit 0
