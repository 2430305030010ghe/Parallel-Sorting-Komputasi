# FORMULIR EVALUASI KONTRIBUSI — MEYLIN
**Peran:** Sistem Arsitek
**Proyek:** Analisis Speedup Sorting Paralel: Merge Sort & Bitonic Sort (OpenMP)

---

## 1. Kontribusi Teknis

**Modul yang dikerjakan:**
```
- src/benchmark.cpp — BenchmarkEngine: ukur waktu, filter outlier ±2σ, export CSV
- src/benchmark.h — header BenchmarkEngine
- src/fault_tolerance.h — withRetry() exponential backoff + CheckpointManager
- Desain arsitektur pipeline sistem keseluruhan
```

**Baris kode yang ditulis (estimasi):**
```
~400 baris
```

**Jumlah commit di GitHub:**
```
3 commit bermakna:
- arch: rancang pipeline DatasetGenerator ke Sorter ke BenchmarkEngine ke CSVExporter
- feat: implementasi BenchmarkEngine dengan 5 repetisi filter outlier dan export CSV
- feat: tambah fault_tolerance.h dengan withRetry exponential backoff dan CheckpointManager
```

**Unit test yang ditulis:**
```
Integration Test — memastikan BenchmarkEngine menghasilkan output yang benar:
- Integration Test #2: DatasetGenerator → BitonicSortPar → terurut benar (65536)
- Integration Test #3: padToPow2 → sort → removePadding → 1000 elemen benar
```

---

## 2. Kontribusi Non-Teknis

- [x] Koordinasi tim / meeting
- [x] Penulisan laporan / dokumentasi
- [x] Review kode rekan
- [x] Presentasi / demo
- [x] Debugging dan testing

---

## 3. Penilaian Diri (1-10)

| Aspek | Nilai |
|-------|-------|
| Kualitas kode yang ditulis | 8 |
| Keaktifan komunikasi tim | 8 |
| Ketepatan waktu deliverable | 8 |
| Pemahaman sistem keseluruhan | 9 |

---

## 4. Penilaian Rekan

### Ghea (Ketua Kelompok)
- Kontribusi: [x] Tinggi [ ] Sedang [ ] Rendah
- Alasan:
```
Ghea mengoordinasikan tim dengan baik dan memastikan semua komponen terintegrasi
dengan benar. Implementasi DatasetGenerator dan konfigurasi benchmark di main.cpp
yang dibuat Ghea menjadi fondasi penting untuk seluruh pipeline sistem.
```

### Elga (Core Developer)
- Kontribusi: [x] Tinggi [ ] Sedang [ ] Rendah
- Alasan:
```
Elga mengimplementasikan empat algoritma sorting dengan kualitas tinggi mengikuti
empat prinsip desain paralel. Penerapan cutoff threshold di Bitonic Sort yang
awalnya tidak ada di SDD merupakan kontribusi teknis yang signifikan.
```

### Helen (QA & Dokumentasi)
- Kontribusi: [x] Tinggi [ ] Sedang [ ] Rendah
- Alasan:
```
Helen memastikan kualitas program melalui 43 unit test yang komprehensif mencakup
smoke test, correctness test, edge case, concurrency, dan integration test dengan
coverage 100% dari 36 fungsi inti.
```

---

## 5. Refleksi

**Hal terbaik yang saya pelajari:**
```
Saya mempelajari bagaimana merancang arsitektur sistem paralel yang baik mengikuti
empat prinsip desain. Memahami pentingnya fault tolerance dalam sistem paralel dan
bagaimana retry logic dengan exponential backoff dapat meningkatkan ketahanan sistem.
```

**Hal yang ingin saya tingkatkan:**
```
Ingin lebih memahami profiling tools seperti gprof dan perf untuk mengidentifikasi
bottleneck secara lebih akurat, sehingga rekomendasi optimasi dapat didasarkan pada
data profiling nyata bukan hanya analisis teoritis.
```

**Kendala terbesar dan solusinya:**
```
Kendala: memastikan BenchmarkEngine mengukur waktu dengan akurat tanpa terpengaruh
overhead inisialisasi atau faktor eksternal seperti proses background Windows.
Solusi: menggunakan 5 repetisi dengan filter outlier ±2σ dan seed RNG yang fixed
sehingga hasil reproducible dan konsisten.
```
