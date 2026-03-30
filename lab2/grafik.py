import matplotlib.pyplot as plt
import numpy as np

sizes = []
threads = []
read_times = []
mult_times = []
write_times = []
total_times = []

with open('stats.txt', 'r') as f:
    lines = f.readlines()
    for line in lines[2:]:
        if line.strip():
            parts = line.split()
            if len(parts) >= 6:
                sizes.append(int(parts[0]))
                threads.append(int(parts[1]))
                read_times.append(int(parts[2]))
                mult_times.append(int(parts[3]))
                write_times.append(int(parts[4]))
                total_times.append(int(parts[5]))

plt.figure(figsize=(12, 8))
for t in sorted(set(threads)):
    t_mult = [mult_times[i] for i in range(len(threads)) if threads[i] == t]
    t_sizes = [sizes[i] for i in range(len(threads)) if threads[i] == t]
    plt.plot(t_sizes, t_mult, 'o-', label=f'{t} потоков', linewidth=2, markersize=8)

plt.xlabel('Размер матрицы (n)', fontsize=12)
plt.ylabel('Время умножения (мс)', fontsize=12)
plt.title('Зависимость времени умножения от размера матрицы', fontsize=14)
plt.legend()
plt.grid(True, alpha=0.3)
plt.savefig('mult_time_vs_size.png', dpi=300)
plt.show()

plt.figure(figsize=(12, 8))
for size in sorted(set(sizes)):
    size_mult = {}
    for i in range(len(threads)):
        if sizes[i] == size:
            size_mult[threads[i]] = mult_times[i]
    
    if 1 in size_mult:
        base_time = size_mult[1]
        t_values = sorted([t for t in size_mult.keys() if t != 1])
        speedups = [base_time / size_mult[t] for t in t_values]
        plt.plot(t_values, speedups, 'o-', label=f'n={size}', linewidth=2, markersize=8)

ideal = [1, 2, 4, 8]
plt.plot(ideal, ideal, 'k--', label='Идеальное ускорение', alpha=0.5)

plt.xlabel('Количество потоков', fontsize=12)
plt.ylabel('Ускорение', fontsize=12)
plt.title('Ускорение при параллельном умножении матриц', fontsize=14)
plt.legend()
plt.grid(True, alpha=0.3)
plt.savefig('speedup.png', dpi=300)
plt.show()

plt.figure(figsize=(12, 8))
for size in sorted(set(sizes)):
    size_mult = {}
    for i in range(len(threads)):
        if sizes[i] == size:
            size_mult[threads[i]] = mult_times[i]
    
    if 1 in size_mult:
        base_time = size_mult[1]
        t_values = sorted([t for t in size_mult.keys() if t != 1])
        efficiency = [base_time / (size_mult[t] * t) for t in t_values]
        plt.plot(t_values, efficiency, 'o-', label=f'n={size}', linewidth=2, markersize=8)

plt.xlabel('Количество потоков', fontsize=12)
plt.ylabel('Эффективность', fontsize=12)
plt.title('Эффективность параллелизации', fontsize=14)
plt.legend()
plt.grid(True, alpha=0.3)
plt.savefig('efficiency.png', dpi=300)
plt.show()

print("Графики сохранены:")
print("- mult_time_vs_size.png")
print("- speedup.png")
print("- efficiency.png")