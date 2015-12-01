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
    f.write(pack('q', len(l)))
    for i in l:
         f.write(pack('q', i))
    f.close()
            
def load_vector(filename):
    result = []
    f = open(filename, 'rb')
    length = unpack('q', f.read(8))[0]
    for i in range(length):
        result.append(unpack('q', f.read(8))[0])
    f.close()
    return result 
           
def save_matrix(m, filename):
    f = open(filename, 'wb')
    f.write(pack('q', len(m)))
    for l in m:
        for i in l:
            f.write(pack('q', i))
    f.close()
                
def load_matrix(filename):
    result = []
    f = open(filename, 'rb')
    length = unpack('q', f.read(8))[0]
    for i in range(length):
        row = []
        for j in range(length):
            row.append(unpack('q', f.read(8))[0])
        result.append(row)
    f.close()
    return result

def dot(v1, v2):
    return sum(x * y for x, y in zip(v1, v2))
    
def multiply(m, v):
    n = len(v)
    result = [dot(m[i], v) for i in range(n)]
    return result
