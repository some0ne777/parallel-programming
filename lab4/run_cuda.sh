%%writefile run_cuda.sh
#!/bin/bash

echo "========================================"
echo "ЭКСПЕРИМЕНТЫ CUDA"
echo "========================================"

rm -f cuda_stats.txt cuda_result.txt
rm -f 1_*.txt 2_*.txt

echo "1. Генерация матриц..."
for s in 200 400 800 1200 1600 2000; do
    python generator.py $s
done

echo "2. Компиляция..."
nvcc -O3 --use_fast_math -o lab2 lab2.cu -Wno-deprecated-gpu-targets

echo "Размер   Блок       Чтение(мс)  Умножение(мс)  Всего(мс)" > cuda_stats.txt
echo "----------------------------------------------------------------------" >> cuda_stats.txt

echo "3. Запуск экспериментов..."
for s in 200 400 800 1200 1600 2000; do
    ./lab2 $s 4 1 0        # Блок 4x4
    ./lab2 $s 8 1 0        # Блок 8x8
    ./lab2 $s 16 1 0       # Блок 16x16
    ./lab2 $s 32 1 0       # Блок 32x32
done

echo "4. Построение графиков..."
python grafik_cuda.py
