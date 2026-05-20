# Progress Log — KPT Minggu 3
## Format: [NAMA] — [TANGGAL]

---

## Progress Log — [ISI NAMA] — [ISI TANGGAL]

### Dikerjakan Hari Ini
- Implementasi fault_tolerance.h: withRetry() dengan exponential backoff
- Implementasi test_sorting.cpp: 5 kategori test (Smoke/Correctness/EdgeCase/Concurrency/Integration)
- Update benchmark.cpp: integrasi retry logic di measureSequential dan measureParallel
- Tambah cutoff threshold di BitonicSortPar (Prinsip 02 Granularitas)
- Update README.md dengan struktur lengkap sesuai panduan Minggu 3

### Kendala / Blockers
- BitonicSortPar sebelumnya tidak punya cutoff → diperbaiki agar sesuai 4 prinsip desain
- dynamic schedule sebelumnya pakai chunk=64 tanpa justifikasi → diubah ke default (chunk=1)

### Solusi
- Tambah parameter cutoff_ di BitonicSortPar constructor dan header
- Implementasi sequential fallback di loop stage jika stageSize < cutoff_

### Rencana Besok (Minggu 4)
- Integrasi penuh semua modul (end-to-end test)
- Jalankan benchmark lengkap dan generate grafik
- Analisis hasil: speedup vs thread count, efisiensi per dataset type
- Analisis Amdahl's Law dari data CSV
- Optimasi jika ada bottleneck yang ditemukan dari profiling

### Commit Hari Ini
- feat: add fault_tolerance.h with withRetry and CheckpointManager
- feat: add comprehensive unit tests (test_sorting.cpp) — 5 categories
- fix: add cutoff threshold to BitonicSortPar (Prinsip 02)
- fix: dynamic schedule chunk=default (was 64, inconsistent with docs)
- docs: update README.md with complete structure per W3 guidelines
- refactor: integrate retry logic into BenchmarkEngine measure functions

---
## Template untuk Anggota Lain

```
## Progress Log — [NAMA] — [TANGGAL]

### Dikerjakan Hari Ini
- 

### Kendala / Blockers
- 

### Solusi
- 

### Rencana Besok
- 

### Commit Hari Ini
- 
```
