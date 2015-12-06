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
    f = open(filetemplate % (i, n), 'r')
    print('%d %f' % (i, float(f.read().split()[1])))
    f.close()
    i *= 2
