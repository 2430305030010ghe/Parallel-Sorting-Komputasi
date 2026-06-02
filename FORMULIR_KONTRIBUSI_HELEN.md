# FORMULIR EVALUASI KONTRIBUSI — HELEN ANGELINA PUTRI
**Peran:** QA & Dokumentasi
**Proyek:** Analisis Speedup Sorting Paralel: Merge Sort & Bitonic Sort (OpenMP)

---

## 1. Kontribusi Teknis

**Modul yang dikerjakan:**
```
- src/test_sorting.cpp — 43 unit test (Smoke/Correctness/EdgeCase/Concurrency/Integration)
- src/coverage_report.cpp — coverage report 100% dari 36 fungsi inti
- README.md — dokumentasi lengkap instalasi dan cara menjalankan
- PROGRESS_LOG.md — daily log per anggota
- Laporan progress Minggu 3 dan laporan analisis performa Minggu 4
```

**Baris kode yang ditulis (estimasi):**
```
~500 baris
```

**Jumlah commit di GitHub:**
```
3 commit bermakna:
- test: implementasi 43 unit test Smoke Correctness EdgeCase Concurrency Integration
- test: tambah coverage_report.cpp hasil 100 persen dari 36 fungsi
- docs: tambah README.md laporan progress dan coverage report 100 persen
```

**Unit test yang ditulis:**
```
Semua 43 unit test:
- Smoke Test (5 test): program tidak crash dengan input minimal
- Correctness Test (20 test): output paralel == sequential untuk 4 dataset
- Edge Case Test (10 test): input kosong, N=1, N=2, semua elemen sama, reverse
- Concurrency Test (3 test): non-determinism check, hasil sama 2/4/8 thread
- Integration Test (5 test): pipeline lengkap termasuk vs std::sort sebagai ground truth
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
| Ketepatan waktu deliverable | 9 |
| Pemahaman sistem keseluruhan | 8 |

---

## 4. Penilaian Rekan

### Ghea (Ketua Kelompok)
- Kontribusi: [x] Tinggi [ ] Sedang [ ] Rendah
- Alasan:
```
Ghea menjalankan peran ketua dengan baik — memastikan semua komponen terintegrasi
dan deadline terpenuhi. Implementasi DatasetGenerator dengan seed yang fixed
memudahkan saya dalam membuat unit test yang deterministik dan reproducible.
```

### Meylin (Sistem Arsitek)
- Kontribusi: [x] Tinggi [ ] Sedang [ ] Rendah
- Alasan:
```
Meylin merancang arsitektur yang bersih dan mudah diuji. BenchmarkEngine dengan
validateSorted yang dibuat Meylin sangat membantu dalam memverifikasi kebenaran
output sorting di setiap konfigurasi benchmark.
```

### Elga (Core Developer)
- Kontribusi: [x] Tinggi [ ] Sedang [ ] Rendah
- Alasan:
```
Elga mengimplementasikan semua algoritma dengan kualitas tinggi sehingga semua
43 unit test yang saya buat berhasil PASS. Kode yang bersih dan terstruktur
memudahkan penulisan test case untuk setiap edge case dan skenario concurrency.
```

---

## 5. Refleksi

**Hal terbaik yang saya pelajari:**
```
Saya mempelajari pentingnya pengujian dalam sistem paralel, khususnya correctness
test yang membuktikan output paralel identik dengan sequential. Memahami bahwa
sistem paralel rentan terhadap race condition dan non-determinism sehingga
pengujian yang komprehensif sangat krusial sebelum benchmark dijalankan.
```

**Hal yang ingin saya tingkatkan:**
```
Ingin mempelajari tools coverage yang lebih canggih seperti gcov atau lcov untuk
menghasilkan laporan coverage yang lebih detail dan akurat, serta menulis test
yang lebih otomatis menggunakan framework seperti Google Test atau Catch2.
```

**Kendala terbesar dan solusinya:**
```
Kendala: membuat test untuk sistem paralel lebih sulit dari sequential karena
hasil bisa berbeda antar run jika ada race condition yang tidak terdeteksi.
Solusi: menggunakan seed RNG yang fixed (42) dan menjalankan setiap test
beberapa kali untuk memastikan hasilnya konsisten dan deterministik.
```