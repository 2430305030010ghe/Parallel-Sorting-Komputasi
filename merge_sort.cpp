// =============================================================================
// merge_sort.cpp - Implementasi Merge Sort Sequential & Parallel
// =============================================================================

#include "merge_sort.h"
#include <omp.h>
#include <algorithm>
#include <vector>

// =============================================================================
// MergeSortSeq - Sequential Implementation
// =============================================================================

void MergeSortSeq::sort(std::vector<int>& data) {
    if (data.size() <= 1) return;
    mergeSort(data, 0, (int)data.size() - 1);
}

void MergeSortSeq::mergeSort(std::vector<int>& arr, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSort(arr, left, mid);
    mergeSort(arr, mid + 1, right);
    merge(arr, left, mid, right);
}

void MergeSortSeq::merge(std::vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<int> L(arr.begin() + left, arr.begin() + left + n1);
    std::vector<int> R(arr.begin() + mid + 1, arr.begin() + mid + 1 + n2);

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) arr[k++] = L[i++];
        else               arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

// =============================================================================
// MergeSortPar - Parallel Implementation menggunakan OpenMP Task
//
// PRINSIP 01 - Dekomposisi Tugas:
//   Task decomposition + Rekursif decomposition.
//   Setiap sub-rekursi (kiri dan kanan) di-spawn sebagai omp task independen.
//   Ini adalah divide & conquer murni: setiap task tidak bergantung satu sama
//   lain sampai fase merge.
//
// PRINSIP 02 - Granularitas:
//   cutoff_ threshold menentukan batas fine-grain vs coarse-grain.
//   Subarray di bawah cutoff diproses sekuensial (coarse-grain) untuk
//   menghindari overhead task yang lebih besar dari pekerjaannya.
//   Sweet spot: cutoff {512, 1024, 2048} diuji untuk temukan nilai optimal.
//
// PRINSIP 03 - Load Balancing:
//   Klausa untied memungkinkan task berpindah thread saat runtime,
//   sehingga OpenMP scheduler melakukan load balancing dinamis secara
//   otomatis. Thread yang selesai lebih cepat dapat mengambil task lain
//   (work stealing behavior).
//
// PRINSIP 04 - Minimasi Komunikasi:
//   __builtin_prefetch disisipkan sebelum task spawn untuk preload data
//   ke cache (locality principle), menyembunyikan memory latency di balik
//   scheduling overhead sehingga data tersedia saat thread mulai eksekusi.
//
// =============================================================================

MergeSortPar::MergeSortPar(int numThreads, int cutoff)
    : numThreads_(numThreads), cutoff_(cutoff) {}

void MergeSortPar::sort(std::vector<int>& data) {
    if (data.size() <= 1) return;

    omp_set_num_threads(numThreads_);

    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            mergeSortPar(data, 0, (int)data.size() - 1);
        }
    }
}

void MergeSortPar::mergeSortPar(std::vector<int>& arr, int left, int right) {
    if (left >= right) return;

    int size = right - left + 1;

    // -----------------------------------------------------------------
    // PRINSIP 02 - Granularitas: Cutoff threshold
    // Subarray kecil diproses sekuensial untuk hindari overhead task.
    // Overhead pembuatan task OpenMP tidak sebanding untuk data kecil.
    // -----------------------------------------------------------------
    if (size <= cutoff_) {
        MergeSortSeq seqSorter;
        std::vector<int> sub(arr.begin() + left, arr.begin() + right + 1);
        seqSorter.sort(sub);
        std::copy(sub.begin(), sub.end(), arr.begin() + left);
        return;
    }

    int mid = left + (right - left) / 2;

    // -----------------------------------------------------------------
    // PRINSIP 04 - Minimasi Komunikasi: Prefetch (locality principle)
    // Preload elemen titik tengah ke cache sebelum task di-spawn,
    // sehingga data sudah tersedia di L1/L2 cache saat thread mulai.
    // -----------------------------------------------------------------
    __builtin_prefetch(&arr[mid], 0, 1);
    __builtin_prefetch(&arr[left + size / 4], 0, 1);
    __builtin_prefetch(&arr[right - size / 4], 0, 1);

    // -----------------------------------------------------------------
    // PRINSIP 01 - Dekomposisi: Task decomposition + Rekursif
    // PRINSIP 03 - Load Balancing: untied = work stealing behavior
    // untied: task dapat dipindah ke thread lain saat runtime,
    // memungkinkan OpenMP scheduler melakukan load balancing dinamis.
    // -----------------------------------------------------------------
    #pragma omp task untied shared(arr)
    {
        mergeSortPar(arr, left, mid);
    }

    #pragma omp task untied shared(arr)
    {
        mergeSortPar(arr, mid + 1, right);
    }

    // Tunggu kedua child tasks selesai sebelum merge
    #pragma omp taskwait

    // Merge phase: sequential (Amdahl's bottleneck - tidak dapat diparalelkan
    // karena bergantung pada hasil kedua sub-rekursi sebelumnya)
    merge(arr, left, mid, right);
}

void MergeSortPar::merge(std::vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    // -----------------------------------------------------------------
    // PRINSIP 04 - Minimasi Komunikasi: Prefetch buffer merge
    // Preload awal buffer L dan R ke cache untuk kurangi miss saat merge.
    // -----------------------------------------------------------------
    std::vector<int> L(arr.begin() + left, arr.begin() + left + n1);
    std::vector<int> R(arr.begin() + mid + 1, arr.begin() + mid + 1 + n2);

    __builtin_prefetch(&L[0], 0, 1);
    __builtin_prefetch(&R[0], 0, 1);

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) arr[k++] = L[i++];
        else               arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

