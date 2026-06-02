# FORMULIR EVALUASI KONTRIBUSI — GHEA VICTORIA NEISCA
**Peran:** Ketua Kelompok
**Proyek:** Analisis Speedup Sorting Paralel: Merge Sort & Bitonic Sort (OpenMP)
---

## 1. Kontribusi Teknis

**Modul yang dikerjakan:**
```
- src/main.cpp — entry point, konfigurasi semua parameter benchmark
- src/dataset_generator.cpp — implementasi 4 distribusi dataset
- src/dataset_generator.h — header DatasetGenerator
- Integrasi pipeline: DatasetGenerator → Sorter → BenchmarkEngine → CSV
```

**Baris kode yang ditulis (estimasi):**
```
~300 baris
```

**Jumlah commit di GitHub:**
```
3 commit bermakna:
- feat: inisialisasi project parallel sorting dan konfigurasi benchmark utama
- feat: implementasi DatasetGenerator dengan 4 distribusi random nearly_sorted reverse duplicates
- chore: setup direktori hasil dan konfigurasi parameter thread cutoff schedule
```

**Unit test yang ditulis:**
```
Integration Test — memastikan pipeline DatasetGenerator → Sorter → validasi berjalan benar:
- Integration Test #1: DatasetGenerator → MergeSortPar → terurut benar (100K)
- Integration Test #4: semua 4 tipe dataset diproses benar oleh MergeSortPar
```

---

## 2. Kontribusi Non-Teknis

- [x] Koordinasi tim / meeting
- [x] Penulisan laporan / dokumentasi
- [x] Review kode rekan
- [x] Presentasi / demo
- [ ] Debugging dan testing

---

## 3. Penilaian Diri (1-10)

| Aspek                        | Nilai |
|------------------------------|-------|
| Kualitas kode yang ditulis   | 8     |
| Keaktifan komunikasi tim     | 9     |
| Ketepatan waktu deliverable  | 9     |
| Pemahaman sistem keseluruhan | 8     |

---

## 4. Penilaian Rekan

### Meylin (Sistem Arsitek)
- Kontribusi: [x] Tinggi [ ] Sedang [ ] Rendah
- Alasan:
```
Meylin merancang arsitektur pipeline sistem secara keseluruhan dan mengimplementasikan
BenchmarkEngine dengan filter outlier dan fault tolerance yang menjadi komponen kritis
dalam menghasilkan data benchmark yang akurat dan andal.
```

### Elga (Core Developer)
- Kontribusi: [x] Tinggi [ ] Sedang [ ] Rendah
- Alasan:
```
Elga mengimplementasikan seluruh algoritma inti yaitu MergeSortSeq, MergeSortPar,
BitonicSortSeq, dan BitonicSortPar lengkap dengan cutoff threshold, prefetch, dan
schedule policy yang merupakan komponen terpenting dari proyek ini.
```

### Helen (QA & Dokumentasi)
- Kontribusi: [x] Tinggi [ ] Sedang [ ] Rendah
- Alasan:
```
Helen membuat 43 unit test dengan coverage 100% yang memastikan kebenaran seluruh
program sebelum benchmark dijalankan, serta mendokumentasikan proyek secara lengkap
melalui README, laporan, dan coverage report.
```

---

## 5. Refleksi

**Hal terbaik yang saya pelajari:**
```
Saya mempelajari cara merancang dan mengintegrasikan sistem paralel secara menyeluruh, mulai dari awal hingga akhir. Saya memahami bahwa pipeline yang baik mencakup tahapan generate data, sorting, benchmark, hingga visualisasi. Selain itu, saya juga belajar bahwa tidak semua algoritma menghasilkan performa yang lebih baik ketika diparalelkan.
```

**Hal yang ingin saya tingkatkan:**
```
Ingin lebih memahami detail implementasi algoritma paralel seperti omp task dan
cutoff threshold agar bisa berkontribusi lebih di bagian core developer selain
hanya di konfigurasi dan integrasi sistem.
```

**Kendala terbesar dan solusinya:**
```
Kendala: memastikan semua parameter benchmark (thread, cutoff, schedule, dataset)
terkonfigurasi dengan benar dan konsisten dengan yang tertulis di laporan SDD.
Solusi: membuat konstanta terpusat di main.cpp sehingga semua parameter mudah
diubah dan tidak ada yang hardcode di tempat lain.
```

