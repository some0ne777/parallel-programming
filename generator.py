import numpy as np

n = int(input("Введите размер матрицы: "))

A = np.random.randint(0, 10, (n, n))
B = np.random.randint(0, 10, (n, n))

with open(f"1_{n}.txt", 'w') as f:
    f.write(f"{n}\n")
    for row in A:
        f.write(' '.join(map(str, row)) + '\n')

with open(f"2_{n}.txt", 'w') as f:
    f.write(f"{n}\n")
    for row in B:
        f.write(' '.join(map(str, row)) + '\n')

print(f"файлы 1_{n}.txt и 2_{n}.txt")