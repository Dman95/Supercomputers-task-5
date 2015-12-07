#!/usr/bin/python

import os, time, sys

CURSOR_UP_ONE = '\x1b[1A'
ERASE_LINE = '\x1b[2K'

while True:
    f = os.popen('llq -W')
    s = f.read()
    print s,
    f.close()
    time.sleep(1)
    for i in range(s.count('\n')):
        print CURSOR_UP_ONE + ERASE_LINE, 
    print '\r',

