#!/usr/bin/python
f = open('mapping.map', 'w')
l = []
for i in range(8):
    for j in range(8):
        for k in range(8):
            l.append("%d %d %d 0" % (i, j, k))
import random
random.shuffle(l)
for s in l:
    f.write(s + '\n')
f.close()
