#!/usr/bin/python

import sys
from_count = int(sys.argv[1])
to = int(sys.argv[2])
A = sys.argv[3] #512
B = sys.argv[4] #0.01
filetemplate = 'out%s_%s' % (A, B) + '_%d.txt'

i = from_count
while i <= to:
    try:
        f = open(filetemplate % (i), 'r')
        time = max([float(s.split()[1]) for s in f.read().split('\n') if s != '' and s.startswith('Time')])
        f.close()
        f = open(filetemplate % (i), 'r')
        n_iter = max([float(s.split()[1]) for s in f.read().split('\n') if s != '' and s.startswith('N')])
        f.close()
    except:
        time = 0
        n_iter = 0
    print('%d %f %d' % (i, time, n_iter))
    i *= 2
