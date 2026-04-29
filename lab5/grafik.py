import matplotlib.pyplot as plt

plt.rcParams['font.family'] = 'DejaVu Sans'
plt.rcParams['axes.unicode_minus'] = False

folders = [
    ('1proces', 1),
    ('2proces', 2),
    ('4proces', 4),
    ('8proces', 8),
]

def read_stats_from_folder(folder):
    sizes = []
    read_times = []
    mult_times = []
    
    try:
        with open(f'{folder}/stats.txt', 'r', encoding='utf-8') as f:
            lines = f.readlines()
            for line in lines[1:]:
                if line.strip():
                    parts = line.split()
                    if len(parts) >= 4:
                        sizes.append(int(parts[0]))
                        read_times.append(int(parts[2]))
                        mult_times.append(int(parts[3]))
    except FileNotFoundError:
        print(f"Файл {folder}/stats.txt не найден!")
        return None, None, None
    
    return sizes, read_times, mult_times

for folder, proc_count in folders:
    sizes, read_times, mult_times = read_stats_from_folder(folder)
    
    if sizes is None:
        continue
    
    plt.figure(figsize=(10, 6))
    
    plt.plot(sizes, mult_times, marker='o', color='red', 
             linewidth=2, markersize=8, label='Время умножения')
    plt.plot(sizes, read_times, marker='s', color='blue', 
             linewidth=2, markersize=8, label='Время чтения')
    
    plt.xlabel('Размер матрицы (N x N)', fontsize=12)
    plt.ylabel('Время (мс)', fontsize=12)
    plt.title(f'Зависимость времени от размера матрицы\n{proc_count} процессов', fontsize=14)
    plt.grid(True, alpha=0.3)
    plt.legend()
    plt.tight_layout()
    
    plt.savefig(f'mpi_{proc_count}proc.png', dpi=300)
    plt.show()