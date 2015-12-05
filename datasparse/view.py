import random
from struct import pack, unpack
           
def load_vector(filename):
    f = open(filename, 'rb')
    result = load_vector_fp(f)
    return result

def load_vector_fp(f):
    result = []
    length = unpack('q', f.read(8))[0]
    for i in range(length):
        index = unpack('q', f.read(8))[0]
        value = unpack('d', f.read(8))[0]
        result.append((index, value))
    return result
               
def load_matrix(filename):
    result = []
    f = open(filename, 'rb')
    row_count = unpack('q', f.read(8))[0]
    col_count = unpack('q', f.read(8))[0]
    f.seek(row_count * 8, 1)
    for i in range(row_count):
        row = load_vector_fp(f)
        result.append(row)
    f.close()
    return result

