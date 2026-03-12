#!/usr/bin/env python3
import os
import sys

w = sys.stdout.write

w("Status: 200 OK\r\n")
w("Content-Type: text/plain\r\n\r\n")

w("THIS IS PYTHON SCRIPT IN HTML/:\n\n")
for k, v in os.environ.items():
    w(f"{k}={v}\r\n")
