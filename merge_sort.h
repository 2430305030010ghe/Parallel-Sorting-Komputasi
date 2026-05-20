#pragma once
// =============================================================================
// merge_sort.h - Header Merge Sort (Sequential & Parallel)
// =============================================================================

#include <vector>

// =============================================================================
// MergeSortSeq - Merge Sort Sequential (Baseline)
// =============================================================================
class MergeSortSeq {
public:
    void sort(std::vector<int>& data);

private:
    void mergeSort(std::vector<int>& arr, int left, int right);
    void merge(std::vector<int>& arr, int left, int mid, int right);
};

// =============================================================================
// MergeSortPar - Merge Sort Parallel menggunakan OpenMP Task
// =============================================================================
class MergeSortPar {
public:
    explicit MergeSortPar(int numThreads = 4, int cutoff = 1024);
    void sort(std::vector<int>& data);

private:
    int numThreads_;
    int cutoff_;        // Threshold untuk fallback ke sequential

    void mergeSortPar(std::vector<int>& arr, int left, int right);
    void merge(std::vector<int>& arr, int left, int mid, int right);
};
