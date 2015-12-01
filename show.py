#!/usr/bin/python

import sys
from_count = int(sys.argv[1])
to = int(sys.argv[2])

i = from_count
while i <= to:
    f = open('out%d.txt' % (i), 'r')
    print('%d %f' % (i, float(f.read().split()[1])))
    f.close()
    i *= 2
