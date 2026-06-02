## 1. Kontribusi Teknis

**Modul yang dikerjakan:**
```
- src/merge_sort.cpp — MergeSortSeq (baseline) + MergeSortPar (omp task untied)
- src/merge_sort.h — header MergeSort
- src/bitonic_sort.cpp — BitonicSortSeq (baseline) + BitonicSortPar (omp parallel for)
- src/bitonic_sort.h — header BitonicSort
- Implementasi 4 prinsip desain: cutoff, prefetch, untied task, schedule policy
```

**Baris kode yang ditulis (estimasi):**
```
~600 baris
```

**Jumlah commit di GitHub:**
```
3 commit bermakna:
- feat: implementasi MergeSortSeq dan MergeSortPar dengan omp task untied
- feat: implementasi BitonicSortSeq dan BitonicSortPar dengan omp parallel for
- feat: tambah cutoff threshold prefetch dan 3 schedule policy sesuai 4 prinsip desain
```

**Unit test yang ditulis:**
```
Correctness Test — memastikan output paralel == output sequential:
- MergeSortPar T=2 dan T=4 untuk semua 4 dataset (8 test)
- BitonicSortPar schedule static/dynamic/guided untuk semua 4 dataset (12 test)
Concurrency Test:
- MergeSortPar deterministik 2 run identik 50K elemen
- BitonicSortPar deterministik 2 run identik 4096 elemen
- MergeSortPar hasil sama untuk 2/4/8 thread
```

---

## 2. Kontribusi Non-Teknis

- [x] Koordinasi tim / meeting
- [ ] Penulisan laporan / dokumentasi
- [x] Review kode rekan
- [x] Presentasi / demo
- [x] Debugging dan testing

---

## 3. Penilaian Diri (1-10)

| Aspek | Nilai |
|-------|-------|
| Kualitas kode yang ditulis | 9 |
| Keaktifan komunikasi tim | 8 |
| Ketepatan waktu deliverable | 8 |
| Pemahaman sistem keseluruhan | 9 |

---

## 4. Penilaian Rekan

### Ghea (Ketua Kelompok)
- Kontribusi: [x] Tinggi [ ] Sedang [ ] Rendah
- Alasan:
```
Ghea memimpin koordinasi tim dengan efektif dan memastikan integrasi semua modul
berjalan lancar. DatasetGenerator yang diimplementasikan Ghea dengan 4 distribusi
data menjadi komponen penting untuk menguji algoritma dalam berbagai kondisi nyata.
```

### Meylin (Sistem Arsitek)
- Kontribusi: [x] Tinggi [ ] Sedang [ ] Rendah
- Alasan:
```
Meylin merancang arsitektur pipeline yang solid dan mengimplementasikan BenchmarkEngine
dengan filter outlier yang menghasilkan data benchmark akurat. Fault tolerance yang
dibuat Meylin juga meningkatkan ketahanan sistem secara keseluruhan.
```

### Helen (QA & Dokumentasi)
- Kontribusi: [x] Tinggi [ ] Sedang [ ] Rendah
- Alasan:
```
Helen membuat pengujian yang sangat komprehensif dengan 43 unit test dan coverage
100% yang memvalidasi kebenaran semua algoritma yang saya implementasikan. Tanpa
pengujian tersebut akan sulit membuktikan kebenaran output paralel ke dosen.
```

---

## 5. Refleksi

**Hal terbaik yang saya pelajari:**
```
Saya mempelajari secara mendalam bagaimana mengimplementasikan paralelisme dengan
OpenMP menggunakan omp task untuk Merge Sort dan omp parallel for untuk Bitonic Sort.
Memahami perbedaan mendasar antara task decomposition dan domain decomposition serta
kapan masing-masing lebih efektif digunakan.
```

**Hal yang ingin saya tingkatkan:**
```
Ingin mengimplementasikan parallel merge untuk menghilangkan bottleneck fase merge
di Merge Sort sehingga speedup bisa melebihi batas Amdahl's Law saat ini. Juga
ingin mengeksplorasi implementasi di GPU menggunakan CUDA untuk Bitonic Sort.
```

**Kendala terbesar dan solusinya:**
```
Kendala: Bitonic Sort paralel ternyata lebih lambat dari sequential karena implicit
barrier yang terlalu sering. Awalnya mengira ada bug di kode karena tidak sesuai
ekspektasi bahwa paralel pasti lebih cepat.
Solusi: Setelah dianalisis ternyata bukan bug tetapi memang sifat algoritma Bitonic
Sort. Jumlah barrier 45 kali dengan overhead masing-masing lebih besar dari manfaat
paralelisme pada data 1 juta elemen.
```

===