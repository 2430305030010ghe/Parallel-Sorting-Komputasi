# Analisis Speedup Sorting Paralel: Merge Sort & Bitonic Sort (OpenMP)

Proyek final mata kuliah Komputasi Paralel dan Terdistribusi yang menganalisis
performa algoritma sorting paralel menggunakan OpenMP. Membandingkan Merge Sort
dan Bitonic Sort dalam versi sequential dan paralel pada berbagai ukuran data,
jumlah thread, dan distribusi dataset.

---

## Tim

| Nama                | NIM           | Peran            |
|---------------------|---------------|------------------|
| Ghea Victoria Neisca| 2430305030010 | Ketua Kelompok   |
| Meylin Sazkia Putri | 2430305030004 | Sistem Arsitek   |
| Elga Edenia Hafitz  | 2430305030014 | Core Developer   |
| Helen Angelina Putri| 2430305030034 | QA & Dokumentasi |

**Mata Kuliah:** Komputasi Paralel dan Terdistribusi
**Semester:** 4
**Universitas:** Universitas Palangka Raya

---

## Hasil Utama

| Metrik                | Nilai                         |
|-----------------------|-------------------------------|
| Speedup tertinggi     | 3.782x (Merge Sort, 8 thread) |
| Serial fraction (f)   | 18.4% - 21.5%                 |
| Speedup maks teoritis | ~5x (Amdahl's Law)            |
| Unit test             | 43 PASS / 0 FAIL              |
| Coverage              | 100% (36 fungsi)              |
| Total kombinasi uji   | 200 konfigurasi               |

---

## Prerequisites

- OS: Windows 10/11
- Compiler: GCC 9+ dengan dukungan OpenMP
- Python 3.8+ untuk visualisasi grafik
- RAM minimum: 4 GB
- Core minimum: 4 core

Cek versi compiler:
```bash
g++ --version
```

Cek OpenMP tersedia:
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

### Langkah 1 — Compile Benchmark

```bash
g++ -O3 -std=c++17 -fopenmp src/main.cpp src/dataset_generator.cpp src/merge_sort.cpp src/bitonic_sort.cpp src/benchmark.cpp -o benchmark.exe
```

### Langkah 2 — Compile Unit Test

```bash
g++ -O1 -std=c++17 -fopenmp src/test_sorting.cpp src/dataset_generator.cpp src/merge_sort.cpp src/bitonic_sort.cpp -o test_sorting.exe
```

### Langkah 3 — Jalankan Unit Test (wajib sebelum benchmark)

```bash
.\test_sorting.exe      # Windows
./test_sorting.exe      # Linux/macOS
```

Output yang diharapkan:
```
HASIL: 43 PASS | 0 FAIL
```

### Langkah 4 — Jalankan Benchmark Sequential vs Parallel

```bash
.\benchmark.exe         # Windows
./benchmark.exe         # Linux/macOS
```

Program akan menguji 200 kombinasi secara otomatis:
- 4 tipe dataset × 4 jumlah thread × 3 cutoff × 3 schedule
- Masing-masing diulang 5 kali
- Hasil tersimpan otomatis di `results/results.csv`
- Estimasi waktu: 5-30 menit tergantung spesifikasi hardware

### Langkah 5 — Generate Grafik

```bash
python scripts/plot.py
```

Menghasilkan 5 grafik di folder `results/`:
- `plot_speedup.png` — Speedup vs jumlah thread + kurva Amdahl
- `plot_efficiency.png` — Efisiensi paralel per dataset
- `plot_execution_time.png` — Execution time sequential vs paralel
- `plot_dataset_comparison.png` — Perbandingan semua algoritma
- `plot_time_vs_dataset.png` — Waktu vs karakteristik dataset

### (Opsional) Realtime Monitor

Buka 2 terminal. Terminal 1 jalankan benchmark, Terminal 2 jalankan:

```bash
python scripts/realtime_plot.py
```

### Coverage Report

```bash
g++ -std=c++17 src/coverage_report.cpp -o coverage_report.exe
.\coverage_report.exe
```

---

## Reproducibility Check

Ikuti langkah ini untuk memverifikasi program berjalan dari awal:

```
1. Clone atau ekstrak ke folder baru (bukan folder lama)
2. Install dependencies: pip install matplotlib pandas numpy
3. Compile benchmark (Langkah 1 di atas)
4. Compile unit test (Langkah 2 di atas)
5. Jalankan unit test → harus 43 PASS 0 FAIL
6. Jalankan benchmark → tunggu selesai
7. Generate grafik → 5 file PNG muncul di results/
```

Seluruh proses dapat diselesaikan dalam kurang dari 15 menit
(tidak termasuk waktu benchmark yang 5-30 menit).

**Catatan:** Seed RNG sudah di-fixate (seed=42) sehingga dataset
yang dihasilkan selalu identik di setiap run — output deterministik.

---

## Struktur Direktori

```
project_parallel_sorting/
│
├── src/
│   ├── main.cpp                # Entry point & konfigurasi benchmark
│   ├── merge_sort.h/.cpp       # MergeSortSeq + MergeSortPar (omp task)
│   ├── bitonic_sort.h/.cpp     # BitonicSortSeq + BitonicSortPar (omp parallel for)
│   ├── dataset_generator.h/.cpp# DatasetGenerator: 4 distribusi dataset
│   ├── benchmark.h/.cpp        # BenchmarkEngine: ukur waktu, filter outlier, CSV
│   ├── fault_tolerance.h       # withRetry() + CheckpointManager
│   ├── test_sorting.cpp        # 43 unit test (5 kategori)
│   └── coverage_report.cpp     # Coverage report 36 fungsi
│
├── scripts/
│   ├── plot.py                 # Generate 5 grafik performa
│   └── realtime_plot.py        # Dashboard realtime saat benchmark jalan
│
├── results/
│   ├── results.csv             # Raw data 200 kombinasi benchmark
│   ├── plot_speedup.png        # Grafik speedup + kurva Amdahl
│   ├── plot_efficiency.png     # Grafik efisiensi paralel
│   ├── plot_execution_time.png # Grafik execution time
│   ├── plot_dataset_comparison.png # Perbandingan per dataset
│   └── plot_time_vs_dataset.png    # Waktu vs karakteristik dataset
│
├── docs/                       # Laporan akhir dan slide presentasi
├── build.bat                   # Script build Windows
├── PROGRESS_LOG.md             # Daily log per anggota
└── README.md                   # Dokumentasi ini
```

---

## Konfigurasi Benchmark

| Parameter | Nilai | Keterangan |
|-----------|-------|------------|
| `DATASET_SIZE` | 1.000.000 | Jumlah elemen Merge Sort |
| `BITONIC_SIZE` | 1.048.576 | Padding ke 2²⁰ untuk Bitonic Sort |
| `REPETITIONS` | 5 | Repetisi per konfigurasi, outlier ±2σ dibuang |
| `THREADS` | {1, 2, 4, 8} | Variasi jumlah thread |
| `MERGE_CUTOFFS` | {512, 1024, 2048} | Cutoff Merge Sort |
| `BITONIC_CUTOFFS` | {512, 1024, 2048} | Cutoff Bitonic Sort |
| `SCHEDULES` | {static, dynamic, guided} | Schedule policy Bitonic Sort |
| `SEED` | 42 | Fixed seed untuk reproducibility |

---

## Format Output CSV

```
algorithm, dataset_type, dataset_size, execution_time, thread_count,
speedup, efficiency, schedule_type, cutoff_threshold, validation_status
```

---

## Prinsip Desain yang Diimplementasi

| Prinsip | Implementasi di Kode |
|---------|---------------------|
| 01 Dekomposisi | `#pragma omp task untied` (Merge) + `#pragma omp parallel for` (Bitonic) |
| 02 Granularitas | `cutoff_` threshold di kedua algoritma — sweet spot 512-2048 |
| 03 Load Balancing | `untied` task work stealing + 3 schedule policy (static/dynamic/guided) |
| 04 Min. Komunikasi | `__builtin_prefetch` di titik strategis untuk locality principle |

---

## Anti-Pattern yang Dihindari

| Anti-Pattern | Status | Cara Menghindari |
|-------------|--------|-----------------|
| God Node | ✅ Dihindari | `omp single nowait` — semua thread jadi worker |
| Over-communication | ✅ Dihindari | `taskwait` lokal + cutoff hindari sync stage kecil |
| No Baseline | ✅ Dihindari | MergeSortSeq & BitonicSortSeq sebagai baseline eksplisit |
| Hard-coded Parallelism | ✅ Dihindari | THREADS, CUTOFFS, SCHEDULES dikonfigurasi di main.cpp |
| Skip Fault Handling | ⚠️ Sebagian | validateSorted() + withRetry(), retry terbatas shared memory |

---

## Temuan Utama

- **Merge Sort paralel berhasil:** speedup 3.19-3.78x dengan 8 thread
- **Bitonic Sort paralel tidak efisien:** barrier overhead > manfaat paralel
- **Schedule dynamic fatal:** 6-10x lebih lambat dari static untuk Bitonic Sort
- **Superlinear speedup terdeteksi:** dataset Duplicates T=2, efisiensi 104.5% (cache effect)
- **Amdahl's Law terbukti akurat:** selisih prediksi vs empiris hanya 0.04x

---

## Keterbatasan

- Fase merge Merge Sort bersifat sequential → speedup maks teoritis ~5x
- Bitonic Sort paralel lebih lambat dari sequential karena 45 implicit barrier
- Fault handling terbatas untuk shared memory OpenMP
- Ukuran data hanya 1 variasi (1 juta elemen)

