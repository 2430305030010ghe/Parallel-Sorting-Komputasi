#pragma once
// =============================================================================
// bitonic_sort.h - Header Bitonic Sort (Sequential & Parallel)
// =============================================================================

#include <vector>
#include <string>

// =============================================================================
// BitonicSortSeq - Bitonic Sort Sequential (Baseline)
// PENTING: Hanya bekerja untuk ukuran data 2^k
// =============================================================================
class BitonicSortSeq {
public:
    void sort(std::vector<int>& data);

private:
    void bitonicSort(std::vector<int>& arr, int lo, int cnt, bool ascending);
    void bitonicMerge(std::vector<int>& arr, int lo, int cnt, bool ascending);
    void compareAndSwap(std::vector<int>& arr, int i, int j, bool ascending);
};

// =============================================================================
// BitonicSortPar - Bitonic Sort Parallel menggunakan OpenMP Parallel For
//
// Prinsip desain yang dipenuhi:
//   01 Dekomposisi   : Domain decomposition - array dibagi ke tiap thread
//   02 Granularitas  : cutoff_ threshold - stage kecil (< cutoff elemen)
//                      fallback ke sequential untuk hindari overhead paralel
//   03 Load Balancing: schedule configurable (static/dynamic/guided)
//   04 Min. Komunikasi: __builtin_prefetch untuk locality principle
//
// Dataset harus di-pad ke 2^k dengan INT_MAX sebelum sort()
// =============================================================================
class BitonicSortPar {
public:
    explicit BitonicSortPar(int numThreads = 4,
                            const std::string& schedule = "guided",
                            int cutoff = 512);
    void sort(std::vector<int>& data);

private:
    int numThreads_;
    std::string schedule_;
    int cutoff_;    // Prinsip 02: stage dengan elemen < cutoff → sequential

    void compareAndSwap(std::vector<int>& arr, int i, int j, bool ascending);
};
