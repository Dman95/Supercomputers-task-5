import random
from struct import pack, unpack

def generate_vector(n):
    l = []
    for i in range(n):
        r = random.randint(0, 100)
        l.append(r)
    return l
    
def generate_matrix(n):
    m = []
    for i in range(n):
        print i
        m.append(generate_vector(n))
    return m
    
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
    
def multiply(m, v):
    n = len(v)
    result = [dot(m[i], v) for i in range(n)]
    return result
