%%writefile grafik_cuda.py
import matplotlib.pyplot as plt
import pandas as pd

data = []
with open('cuda_stats.txt', 'r') as f:
    lines = f.readlines()

for line in lines[2:]:
    if line.strip() and '---' not in line:
        parts = line.split()
        if len(parts) >= 5:
            size = int(parts[0])
            block = int(parts[1])
            mult_time = int(parts[3])
            if mult_time > 0:
                data.append([size, block, mult_time])

df = pd.DataFrame(data, columns=['Размер', 'Блок', 'Умножение(мс)'])

if len(df) > 0:
    plt.figure(figsize=(10, 6))
    
    for block in sorted(df['Блок'].unique()):
        data = df[df['Блок'] == block]
        plt.plot(data['Размер'], data['Умножение(мс)'], 'o-', linewidth=2, markersize=8, 
                label=f'Блок {block}x{block}')
    
    plt.xlabel('Размер матрицы')
    plt.ylabel('Время умножения (мс)')
    plt.title('Умножение матриц на GPU')
    plt.legend()
    plt.grid(True)
    plt.savefig('cuda_graph.png', dpi=150)
    plt.show()
    print(" График сохранён")
else:
    print("Нет данных для графика")