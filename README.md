# Parallel Sorting: Merge Sort vs Bitonic Sort (OpenMP)

Proyek benchmark performa algoritma sorting paralel menggunakan OpenMP.  
Membandingkan **Merge Sort** dan **Bitonic Sort** dalam versi sekuensial dan paralel  
pada berbagai ukuran data, jumlah thread, dan distribusi dataset.

**Mata Kuliah:** Komputasi Paralel dan Terdistribusi  
**Semester:** Genap 2024/2025

---

## Prerequisites

- OS: Windows 10/11, Linux, atau macOS
- Compiler: GCC 9+ dengan dukungan OpenMP (`g++ --version`)
- Python 3.8+ untuk visualisasi grafik
- Pustaka Python: `matplotlib`, `pandas`, `numpy`

Cek apakah OpenMP tersedia:
```bash
echo '#include <omp.h>' | g++ -fopenmp -x c++ - -o /dev/null && echo "OpenMP OK"
```

---

## Instalasi

```bash
# 1. Clone atau ekstrak project
cd project_parallel_sorting

# 2. Install Python dependencies
pip install matplotlib pandas numpy
```

---

## Cara Menjalankan

### Sequential vs Parallel Benchmark

```bash
# Langkah 1: Compile semua modul
g++ -O3 -std=c++17 -fopenmp \
    src/main.cpp \
    src/dataset_generator.cpp \
    src/merge_sort.cpp \
    src/bitonic_sort.cpp \
    src/benchmark.cpp \
    -o benchmark.exe

# Langkah 2: Jalankan benchmark (output ke results/results.csv)
.\benchmark.exe        # Windows
./benchmark.exe        # Linux/macOS
```

### Unit Testing

```bash
# Compile test runner
g++ -O1 -std=c++17 -fopenmp \
    src/test_sorting.cpp \
    src/dataset_generator.cpp \
    src/merge_sort.cpp \
    src/bitonic_sort.cpp \
    -o test_sorting.exe

# Jalankan semua test
.\test_sorting.exe      # Windows
./test_sorting.exe      # Linux/macOS
```

Test yang dijalankan:
1. **Smoke Test** — program tidak crash dengan input minimal
2. **Correctness Test** — output paralel == output sequential
3. **Edge Case Test** — input kosong, N=1, N=2, semua elemen sama
4. **Concurrency Test** — non-determinism check (2 run → hasil identik)
5. **Integration Test** — pipeline DatasetGenerator → Sorter → validasi

### Visualisasi Grafik

```bash
# Grafik statis (setelah benchmark selesai)
python scripts/plot.py

# Grafik realtime (jalankan bersamaan dengan benchmark)
python scripts/realtime_plot.py
```

---

## Struktur Direktori

```
project_parallel_sorting/
│
├── src/
│   ├── main.cpp                # Entry point & konfigurasi benchmark
│   ├── merge_sort.h/.cpp       # MergeSortSeq + MergeSortPar (omp task)
│   ├── bitonic_sort.h/.cpp     # BitonicSortSeq + BitonicSortPar (omp parallel for)
│   ├── dataset_generator.h/.cpp# DatasetGenerator: random/nearly_sorted/reverse/duplicates
│   ├── benchmark.h/.cpp        # BenchmarkEngine: ukur waktu, filter outlier, CSV
│   ├── fault_tolerance.h       # withRetry() + CheckpointManager
│   └── test_sorting.cpp        # Unit test (Smoke/Correctness/EdgeCase/Concurrency/Integration)
│
├── scripts/
│   ├── plot.py                 # Generate grafik speedup & efisiensi
│   └── realtime_plot.py        # Grafik realtime sambil benchmark jalan
│
├── results/
│   └── results.csv             # Output benchmark (auto-generated)
│
├── build.bat                   # Script build Windows
└── README.md                   # Dokumentasi ini
```

---

## Konfigurasi Benchmark (src/main.cpp)

| Parameter | Nilai | Keterangan |
|-----------|-------|------------|
| `DATASET_SIZE` | 1.000.000 | Jumlah elemen data |
| `BITONIC_SIZE` | 1.048.576 | Padding ke 2²⁰ untuk Bitonic Sort |
| `REPETITIONS` | 5 | Repetisi per konfigurasi, outlier ±2σ dibuang |
| `THREADS` | {1, 2, 4, 8} | Variasi jumlah thread yang diuji |
| `MERGE_CUTOFFS` | {512, 1024, 2048} | Cutoff threshold Merge Sort |
| `BITONIC_CUTOFFS` | {512, 1024, 2048} | Cutoff threshold Bitonic Sort |
| `SCHEDULES` | {static, dynamic, guided} | Schedule policy Bitonic Sort |

---

## Format Output CSV

```
algorithm,dataset_type,dataset_size,execution_time,thread_count,
speedup,efficiency,schedule_type,cutoff_threshold,validation_status
```

---

## Prinsip Desain yang Diimplementasi

| Prinsip | Implementasi |
|---------|-------------|
| 01 Dekomposisi | Task decomposition (Merge) + Domain decomposition (Bitonic) |
| 02 Granularitas | cutoff_ threshold di kedua algoritma — sweet spot 512–2048 |
| 03 Load Balancing | untied task (work stealing) + 3 schedule policy |
| 04 Min. Komunikasi | __builtin_prefetch di titik strategis array |

---

## Keterbatasan yang Diketahui

- **Amdahl's Law**: fase merge bersifat sequential → speedup maks ~3x dengan 8 thread
- **Barrier Overhead**: Bitonic Sort punya ~190 barrier untuk n=2²⁰
- **Fault Handling**: retry logic ada, tapi retry tidak relevan untuk OpenMP shared memory (crash 1 thread = crash seluruh proses)
