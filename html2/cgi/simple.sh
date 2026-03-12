#!/usr/bin/env bash

# Build body
body="<!DOCTYPE html>
<html>
<body>
<pre>
THIS IS SIMPLE TEXT HTML2/
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
