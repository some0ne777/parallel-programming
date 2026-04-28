import matplotlib.pyplot as plt

plt.rcParams['font.family'] = 'DejaVu Sans'
plt.rcParams['axes.unicode_minus'] = False

sizes = []
processes = []
mult_times = []

with open('stats.txt', 'r') as f:
    lines = f.readlines()
    for line in lines[1:]:
        if line.strip():
            parts = line.split()
            if len(parts) >= 6:
                sizes.append(int(parts[0]))
                processes.append(int(parts[1]))
                mult_times.append(int(parts[3])) 

plt.figure(figsize=(10, 6))

for p in sorted(set(processes)):
    p_sizes = []
    p_times = []
    for i in range(len(processes)):
        if processes[i] == p:
            p_sizes.append(sizes[i])
            p_times.append(mult_times[i])
    
    sorted_pairs = sorted(zip(p_sizes, p_times))
    p_sizes, p_times = zip(*sorted_pairs)
    
    plt.plot(p_sizes, p_times, marker='o', label=f'{p} процессов')

plt.xlabel('Размер матрицы (N x N)', fontsize=12)
plt.ylabel('Время умножения (мс)', fontsize=12)
plt.title('Зависимость времени умножения матриц от размера', fontsize=14)

plt.grid(True, alpha=0.3)
plt.legend()
plt.tight_layout()
plt.savefig('mpi_mult_time_vs_size.png', dpi=300)
plt.show()

print("График сохранен: mpi_mult_time_vs_size.png")