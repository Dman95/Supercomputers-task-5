import matplotlib.pyplot as plt
import numpy as np
from struct import pack, unpack

def load_vector_fp(f):
    result = []
    length = unpack('<q', f.read(8))[0]
    if 'calls' not in dir(load_vector_fp):
        load_vector_fp.calls = 1
    print(length, load_vector_fp.calls)
    load_vector_fp.calls += 1
    for i in range(length):
        result.append(unpack('<d', f.read(8))[0])
    return result
                
def load_matrix(filename):
    result = []
    f = open(filename, 'rb')
    row_count = unpack('<q', f.read(8))[0]
    col_count = unpack('<q', f.read(8))[0]
    print(row_count, col_count);
    for i in range(row_count):
        row = load_vector_fp(f)
        result.append(row)
    f.close()
    return result

A = load_matrix('result')

plt.imshow(A, origin='lower')
plt.grid(True)
plt.show()
