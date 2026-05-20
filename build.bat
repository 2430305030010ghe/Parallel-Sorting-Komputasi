@echo off
REM ============================================================
REM build.bat - Build Script untuk Windows (MSYS2 + MinGW-w64)
REM Parallel Sorting Benchmark: Merge Sort vs Bitonic Sort
REM ============================================================

echo ============================================================
echo  Building Parallel Sorting Benchmark
echo  Compiler: g++ (MinGW-w64)
echo  Standard: C++17
echo  Flags:    -O3 -fopenmp
echo ============================================================
echo.

REM Buat folder results jika belum ada
if not exist "results" (
    mkdir results
    echo [INFO] Folder results/ dibuat
)

REM Compile semua source files
echo [BUILD] Compiling...
g++ -O3 -std=c++17 -fopenmp ^
    src/main.cpp ^
    src/dataset_generator.cpp ^
    src/merge_sort.cpp ^
    src/bitonic_sort.cpp ^
    src/benchmark.cpp ^
    -o benchmark.exe

REM Cek hasil kompilasi
if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] Build berhasil! File: benchmark.exe
    echo.
    echo Cara menjalankan:
    echo   benchmark.exe              ^(jalankan benchmark^)
    echo   python scripts/plot.py     ^(static plot setelah benchmark^)
    echo   python scripts/realtime_plot.py  ^(realtime monitor^)
    echo.
) else (
    echo.
    echo [ERROR] Build GAGAL! Error code: %ERRORLEVEL%
    echo.
    echo Pastikan:
    echo   1. MSYS2 + MinGW-w64 sudah diinstall
    echo   2. g++ ada di PATH ^(coba: g++ --version^)
    echo   3. OpenMP tersedia ^(sudah termasuk di MinGW-w64^)
    echo.
)

pause
