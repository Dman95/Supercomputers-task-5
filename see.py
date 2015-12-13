#!/usr/bin/python

import os, time, sys

CURSOR_UP_ONE = '\x1b[1A'
ERASE_LINE = '\x1b[2K'

try:
    while True:
        f = os.popen('cat %s' % sys.argv[1])
        s = f.read()
        s = '\n'.join(s.split('\n')[-10:])
        print s,
        f.close()
        time.sleep(1)
        for i in range(s.count('\n')):
            print CURSOR_UP_ONE + ERASE_LINE, 
        print '\r',
except KeyboardInterrupt:
    pass

