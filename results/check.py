#!/usr/bin/python

import sys
from matrix import *

m1 = load_matrix(sys.argv[1])
m2 = load_matrix(sys.argv[2])
if multiply(m1, m2) == load_matrix(sys.argv[3]):
    print "Ok!"
else:
    print "Fail!"
