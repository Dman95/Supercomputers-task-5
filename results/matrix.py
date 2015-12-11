import random
from struct import pack, unpack

def save_vector(l, filename):
    f = open(filename, 'wb') 
    save_vector_fp(l, filename);
    f.close()

def save_vector_fp(l, filename):
    f.write(pack('q', len(l)))
    for i in l:
        f.write(pack('d', i))
            
def load_vector(filename):
    f = open(filename, 'rb')
    result = load_vector_fp(f)
    return result

def load_vector_fp(f):
    result = []
    length = unpack('q', f.read(8))[0]
    for i in range(length):
        result.append(unpack('d', f.read(8))[0])
    return result

def save_matrix(m, filename):
    f = open(filename, 'wb')
    f.write(pack('q', len(m)))
    f.write(pack('q', len(m[0])))
    for l in m:
        save_vector_fp(l, f)
    f.close()
                
def load_matrix(filename):
    result = []
    f = open(filename, 'rb')
    row_count = unpack('q', f.read(8))[0]
    col_count = unpack('q', f.read(8))[0]
    for i in range(row_count):
        row = load_vector_fp(f)
        result.append(row)
    f.close()
    return result

def dot(v1, v2):
    return sum(x * y for x, y in zip(v1, v2))
   
def multiply(m1, m2):
    result = [[0 for j in range(len(m2[0]))] for i in range(len(m1))]
    m2 = list(zip(*m2))
    for i in range(len(m1)):
        for j in range(len(m2)):
            result[i][j] = dot(m1[i], m2[j])
    return result 

def matrix_cmp(m1, m2):
    eps = 0.000001
    for i in range(len(m1)):
        for j in range(len(m1[0])):
            if abs(m1[i][j] - m2[i][j]) > eps:
                return False
    return True
