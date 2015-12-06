#!/usr/bin/python

import sys
from_count = int(sys.argv[1])
to = int(sys.argv[2])
n = int(sys.argv[3])
sparse = len(sys.argv) > 4
if sparse:
    filetemplate = 'out%d_%d_sparse.txt'
else:
    filetemplate = 'out%d_%d.txt'

i = from_count
while i <= to:
    try:
        f = open(filetemplate % (i, n), 'r')
        time = float(f.read().split()[1])
        f.close()
    except:
        time = 0
    print('%d %f' % (i, time))
    i *= 2
