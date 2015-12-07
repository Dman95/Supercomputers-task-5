#!/usr/bin/python

import os, time, sys

while True:
    f = os.popen('llq -W')
    s = f.read()
    print s,
    f.close()
    time.sleep(1)
    for i in range(s.count('\n')):
        sys.stdout.write('\033[F') #Cursor up one line
