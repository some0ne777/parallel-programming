@echo off
chcp 65001 > nul
echo ========================================
echo ЭКСПЕРИМЕНТЫ
echo ========================================
echo.

REM
if exist stats.txt del stats.txt

REM
echo Генерация матриц...
python generator.py 200
python generator.py 400
python generator.py 800
python generator.py 1200
python generator.py 1600
python generator.py 2000
echo.

REM
echo ========== РАЗМЕР 200x200 ==========
lab1.exe 1 200
lab1.exe 2 200
lab1.exe 4 200
lab1.exe 8 200
echo.

REM
echo ========== РАЗМЕР 400x400 ==========
lab1.exe 1 400
lab1.exe 2 400
lab1.exe 4 400
lab1.exe 8 400
echo.

REM
echo ========== РАЗМЕР 800x800 ==========
lab1.exe 1 800
lab1.exe 2 800
lab1.exe 4 800
lab1.exe 8 800
echo.

REM
echo ========== РАЗМЕР 1200x1200 ==========
lab1.exe 1 1200
lab1.exe 2 1200
lab1.exe 4 1200
lab1.exe 8 1200
echo.

REM
echo ========== РАЗМЕР 1600x1600 ==========
lab1.exe 1 1600
lab1.exe 2 1600
lab1.exe 4 1600
lab1.exe 8 1600
echo.

REM
echo ========== РАЗМЕР 2000x2000 ==========
lab1.exe 1 2000
lab1.exe 2 2000
lab1.exe 4 2000
lab1.exe 8 2000
echo.

echo.
echo Результаты в файле stats.txt:
echo.
type stats.txt
echo.
pause