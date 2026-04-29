import numpy as np
import sys

if len(sys.argv) > 1:
    n = int(sys.argv[1])
else:
    n = int(input("Введите размер матрицы: "))

print(f"Генерация матриц размером {n}x{n}...")

A = np.random.randint(0, 10, (n, n))
B = np.random.randint(0, 10, (n, n))

with open(f"1_{n}.txt", 'w') as f:
    for row in A:
        f.write(' '.join(map(str, row)) + '\n')

with open(f"2_{n}.txt", 'w') as f:
    for row in B:
        f.write(' '.join(map(str, row)) + '\n')

print(f"Созданы файлы: 1_{n}.txt и 2_{n}.txt")