// =============================================================================
// bitonic_sort.cpp - Implementasi Bitonic Sort Sequential & Parallel
//
// Bitonic Sort adalah sorting network berbasis compare-and-swap.
// Membutuhkan ukuran input tepat 2^k (power of two).
// Complexity: O(n log^2 n) operasi, dapat diparalelkan setiap stage.
// =============================================================================

#include "bitonic_sort.h"
#include <omp.h>
#include <algorithm>
#include <stdexcept>
#include <cmath>

// =============================================================================
// Helper: cek apakah n adalah power of two
// =============================================================================
static bool isPowerOfTwo(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

// =============================================================================
// BitonicSortSeq - Sequential Implementation
// =============================================================================

void BitonicSortSeq::sort(std::vector<int>& data) {
    int n = (int)data.size();
    if (n <= 1) return;
    if (!isPowerOfTwo(n)) {
        throw std::invalid_argument("BitonicSort: ukuran harus power of two. Ukuran: " + std::to_string(n));
    }
    bitonicSort(data, 0, n, true);
}

void BitonicSortSeq::bitonicSort(std::vector<int>& arr, int lo, int cnt, bool ascending) {
    if (cnt <= 1) return;
    int k = cnt / 2;
    bitonicSort(arr, lo, k, true);
    bitonicSort(arr, lo + k, k, false);
    bitonicMerge(arr, lo, cnt, ascending);
}

void BitonicSortSeq::bitonicMerge(std::vector<int>& arr, int lo, int cnt, bool ascending) {
    if (cnt <= 1) return;
    int k = cnt / 2;
    for (int i = lo; i < lo + k; ++i) {
        compareAndSwap(arr, i, i + k, ascending);
    }
    bitonicMerge(arr, lo, k, ascending);
    bitonicMerge(arr, lo + k, k, ascending);
}

void BitonicSortSeq::compareAndSwap(std::vector<int>& arr, int i, int j, bool ascending) {
    if ((arr[i] > arr[j]) == ascending) {
        std::swap(arr[i], arr[j]);
    }
}

// =============================================================================
// BitonicSortPar - Parallel Implementation menggunakan OpenMP Parallel For
//
// PRINSIP 01 - Dekomposisi Tugas:
//   Domain decomposition: setiap iterasi inner loop (compare-and-swap)
//   dibagi ke thread secara merata. Setiap thread mengerjakan segmen
//   indeks yang tidak saling bergantung dalam satu stage.
//
// PRINSIP 02 - Granularitas (BARU):
//   Cutoff threshold diterapkan pada ukuran stage aktif (j * 2).
//   Stage dengan elemen aktif < cutoff_ dieksekusi sekuensial untuk
//   menghindari overhead pembuatan thread pada beban kerja ringan.
//   Sweet spot: cutoff=512 menyeimbangkan overhead vs paralelisme.
//
// PRINSIP 03 - Load Balancing:
//   Tiga policy scheduling tersedia:
//     * static  : Blok tetap per thread, overhead rendah, cocok data random
//     * dynamic : Alokasi dinamis (chunk=1 per default), cocok beban tidak
//                 merata (nearly_sorted, duplicates)
//     * guided  : Chunk mulai besar lalu mengecil adaptif, kompromi
//                 static vs dynamic (default)
//   Implicit barrier di akhir setiap parallel for memastikan sinkronisasi
//   antar stage sebelum melanjutkan ke stage berikutnya.
//
// PRINSIP 04 - Minimasi Komunikasi (BARU):
//   __builtin_prefetch disisipkan sebelum blok paralel untuk preload
//   data ke cache (locality principle), sehingga mengurangi cache miss
//   saat thread mulai mengakses elemen yang akan di-swap.
//
// =============================================================================

BitonicSortPar::BitonicSortPar(int numThreads, const std::string& schedule, int cutoff)
    : numThreads_(numThreads), schedule_(schedule), cutoff_(cutoff) {}

void BitonicSortPar::sort(std::vector<int>& data) {
    int n = (int)data.size();
    if (n <= 1) return;
    if (!isPowerOfTwo(n)) {
        throw std::invalid_argument("BitonicSort: ukuran harus power of two.");
    }

    omp_set_num_threads(numThreads_);

    // Algoritma iteratif Bitonic Sort
    // k = ukuran sequence yang diproses (2, 4, 8, ..., n)
    for (int k = 2; k <= n; k *= 2) {
        // j = jarak antara elemen yang dibandingkan dalam stage ini
        for (int j = k / 2; j > 0; j /= 2) {

            // -----------------------------------------------------------------
            // PRINSIP 02 - Granularitas: Cutoff
            // Ukuran stage aktif = j * 2 (jumlah elemen yang berinteraksi).
            // Jika di bawah cutoff, tidak ada manfaat membuat thread baru
            // karena overhead lebih besar dari pekerjaannya (fine-grain).
            // -----------------------------------------------------------------
            int stageSize = j * 2;
            if (stageSize < cutoff_) {
                // Sequential fallback untuk stage kecil
                for (int i = 0; i < n; ++i) {
                    int l = i ^ j;
                    if (l > i) {
                        bool ascending = ((i & k) == 0);
                        compareAndSwap(data, i, l, ascending);
                    }
                }
                continue; // lanjut ke j berikutnya
            }

            // -----------------------------------------------------------------
            // PRINSIP 04 - Minimasi Komunikasi: Prefetch
            // Preload elemen tengah array ke cache sebelum thread-thread
            // mulai mengakses data, sehingga mengurangi cache miss latency.
            // -----------------------------------------------------------------
            __builtin_prefetch(&data[n / 2], 0, 1);
            __builtin_prefetch(&data[n / 4], 0, 1);
            __builtin_prefetch(&data[3 * n / 4], 0, 1);

            // -----------------------------------------------------------------
            // PRINSIP 03 - Load Balancing: Schedule Policy
            // -----------------------------------------------------------------
            if (schedule_ == "static") {
                // Static: blok tetap per thread
                // Optimal untuk data random karena beban tiap iterasi merata
                #pragma omp parallel for schedule(static) shared(data)
                for (int i = 0; i < n; ++i) {
                    int l = i ^ j;
                    if (l > i) {
                        bool ascending = ((i & k) == 0);
                        compareAndSwap(data, i, l, ascending);
                    }
                }
            }
            else if (schedule_ == "dynamic") {
                // Dynamic: alokasi dinamis (chunk=1 per default)
                // Optimal untuk nearly_sorted dan duplicates (beban tidak merata)
                #pragma omp parallel for schedule(dynamic) shared(data)
                for (int i = 0; i < n; ++i) {
                    int l = i ^ j;
                    if (l > i) {
                        bool ascending = ((i & k) == 0);
                        compareAndSwap(data, i, l, ascending);
                    }
                }
            }
            else { // guided (default)
                // Guided: chunk mulai besar lalu mengecil secara adaptif
                // Sweet spot antara static dan dynamic
                #pragma omp parallel for schedule(guided) shared(data)
                for (int i = 0; i < n; ++i) {
                    int l = i ^ j;
                    if (l > i) {
                        bool ascending = ((i & k) == 0);
                        compareAndSwap(data, i, l, ascending);
                    }
                }
            }
            // Implicit barrier: semua thread selesai sebelum stage berikutnya
        }
    }
}

void BitonicSortPar::compareAndSwap(std::vector<int>& arr, int i, int j, bool ascending) {
    if ((arr[i] > arr[j]) == ascending) {
        std::swap(arr[i], arr[j]);
    }
}
