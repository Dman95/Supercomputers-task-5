#!/usr/bin/python

import sys
from generator import *

m = load_matrix(sys.argv[1])
v = load_vector(sys.argv[2])
if multiply(m, v) == load_vector(sys.argv[3]):
    print "Ok!"
else:
    print "Fail!"
