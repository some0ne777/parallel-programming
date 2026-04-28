@echo off
chcp 65001 > nul
echo ========================================
echo ПОЛНЫЙ ЦИКЛ ЭКСПЕРИМЕНТОВ MPI
echo ========================================
echo.

echo 1. Очистка старых файлов...
if exist stats.txt del stats.txt
if exist result.txt del result.txt
for %%s in (200 400 800 1200 1600 2000) do (
    if exist 1_%%s.txt del 1_%%s.txt
    if exist 2_%%s.txt del 2_%%s.txt
)
echo.

echo 2. Генерация матриц...
python generator.py 200
python generator.py 400
python generator.py 800
python generator.py 1200
python generator.py 1600
python generator.py 2000
echo.

echo 3. Проверка наличия исполняемого файла...
if not exist lab1.exe (
    echo ОШИБКА: lab1.exe не найден!
    echo Сначала скомпилируйте программу.
    pause
    exit /b 1
)
echo.

echo Размер   Процессов  Чтение(мс)  Умножение(мс)  Запись(мс)  Всего(мс) > stats.txt
echo ---------------------------------------------------------------- >> stats.txt

set SIZES=200 400 800 1200 1600 2000
set PROCS=1 2 4 8

for %%s in (%SIZES%) do (
    echo ========== РАЗМЕР %%s x %%s ==========
    for %%p in (%PROCS%) do (
        echo Запуск с %%p процессами...
        mpiexec -n %%p lab1.exe %%p %%s
        echo.
    )
    echo.
)

echo ========================================
echo 5. РЕЗУЛЬТАТЫ ЭКСПЕРИМЕНТОВ:
echo ========================================
echo.
type stats.txt
echo.

echo 6. Проверка корректности умножения...
for %%s in (200 400 800 1200 1600 2000) do (
    echo.
    echo Проверка для размера %%s...
    mpiexec -n 1 lab1.exe 1 %%s
    python verify_results.py %%s 1_%%s.txt 2_%%s.txt result.txt
)
echo.

echo 7. Построение графиков...
python grafik.py
echo.

echo 8. Созданные файлы:
echo    - stats.txt (результаты экспериментов)
echo    - result.txt (последний результат умножения)
echo    - mpi_mult_time_vs_size.png (график времени)
echo    - mpi_speedup.png (график ускорения)
echo    - mpi_efficiency.png (график эффективности)
echo.

echo ========================================
echo ЭКСПЕРИМЕНТЫ ЗАВЕРШЕНЫ!
echo ========================================
pause