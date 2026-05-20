// =============================================================================
// test_sorting.cpp - Unit Testing untuk Parallel Sorting System
//
// Mengikuti panduan Minggu 3 KPT:
//   1. Smoke Test       - program bisa jalan tanpa crash
//   2. Correctness Test - output paralel == output sequential
//   3. Edge Case Test   - input kosong, N=1, N=2, data sama semua
//   4. Concurrency Test - non-determinism check (jalankan 2x, hasil sama)
//   5. Integration Test - pipeline DatasetGenerator → Sorter → validasi
//
// Cara run:
//   g++ -O1 -std=c++17 -fopenmp src/test_sorting.cpp src/merge_sort.cpp
//       src/bitonic_sort.cpp src/dataset_generator.cpp -o test_sorting
//   ./test_sorting
// =============================================================================

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <string>
#include <climits>

#include "merge_sort.h"
#include "bitonic_sort.h"
#include "dataset_generator.h"

// =============================================================================
// Helper: warna output terminal
// =============================================================================
#define GREEN  "\033[32m"
#define RED    "\033[31m"
#define YELLOW "\033[33m"
#define RESET  "\033[0m"

int passed = 0;
int failed = 0;

void reportPass(const std::string& testName) {
    std::cout << GREEN << "  [PASS] " << RESET << testName << "\n";
    passed++;
}

void reportFail(const std::string& testName, const std::string& reason) {
    std::cout << RED << "  [FAIL] " << RESET << testName << " — " << reason << "\n";
    failed++;
}

// =============================================================================
// Helper: cek apakah array sudah terurut ascending (abaikan INT_MAX padding)
// =============================================================================
bool isSorted(const std::vector<int>& v) {
    for (size_t i = 1; i < v.size(); ++i) {
        if (v[i-1] > v[i]) return false;
    }
    return true;
}

// =============================================================================
// Helper: pad ke power of two untuk Bitonic Sort
// =============================================================================
std::vector<int> padPow2(const std::vector<int>& data) {
    int n = (int)data.size();
    if (n == 0) return {};
    int target = 1;
    while (target < n) target *= 2;
    std::vector<int> padded = data;
    while ((int)padded.size() < target) padded.push_back(INT_MAX);
    return padded;
}

// =============================================================================
// ── SMOKE TESTS ──────────────────────────────────────────────────────────────
// Pastikan program bisa dijalankan tanpa crash dengan input minimal
// =============================================================================
void runSmokeTests() {
    std::cout << "\n[1] SMOKE TESTS\n";

    // Smoke 1: MergeSortSeq tidak crash dengan input kecil
    try {
        std::vector<int> data = {5, 3, 1, 4, 2};
        MergeSortSeq sorter;
        sorter.sort(data);
        reportPass("MergeSortSeq: tidak crash dengan 5 elemen");
    } catch (...) {
        reportFail("MergeSortSeq: tidak crash dengan 5 elemen", "exception tak terduga");
    }

    // Smoke 2: MergeSortPar tidak crash dengan input kecil
    try {
        std::vector<int> data = {5, 3, 1, 4, 2};
        MergeSortPar sorter(2, 2);
        sorter.sort(data);
        reportPass("MergeSortPar: tidak crash dengan 5 elemen, 2 thread");
    } catch (...) {
        reportFail("MergeSortPar: tidak crash dengan 5 elemen, 2 thread", "exception tak terduga");
    }

    // Smoke 3: BitonicSortSeq tidak crash dengan 8 elemen (2^3)
    try {
        std::vector<int> data = {8, 7, 6, 5, 4, 3, 2, 1};
        BitonicSortSeq sorter;
        sorter.sort(data);
        reportPass("BitonicSortSeq: tidak crash dengan 8 elemen (2^3)");
    } catch (...) {
        reportFail("BitonicSortSeq: tidak crash dengan 8 elemen", "exception tak terduga");
    }

    // Smoke 4: BitonicSortPar tidak crash
    try {
        std::vector<int> data = {8, 7, 6, 5, 4, 3, 2, 1};
        BitonicSortPar sorter(2, "static", 2);
        sorter.sort(data);
        reportPass("BitonicSortPar: tidak crash dengan 8 elemen, 2 thread");
    } catch (...) {
        reportFail("BitonicSortPar: tidak crash dengan 8 elemen", "exception tak terduga");
    }

    // Smoke 5: DatasetGenerator tidak crash
    try {
        DatasetGenerator gen(42);
        auto data = gen.generate(100, "random");
        reportPass("DatasetGenerator: tidak crash generate 100 elemen random");
    } catch (...) {
        reportFail("DatasetGenerator: tidak crash", "exception tak terduga");
    }
}

// =============================================================================
// ── CORRECTNESS TESTS ────────────────────────────────────────────────────────
// Output paralel == output sequential untuk input yang sama
// Ini adalah tes terpenting untuk sistem paralel (PDF hal. 8)
// =============================================================================
void runCorrectnessTests() {
    std::cout << "\n[2] CORRECTNESS TESTS (Paralel == Sequential)\n";

    DatasetGenerator gen(42);

    // Test setiap kombinasi: algoritma × distribusi dataset
    const std::vector<std::string> dsTypes = {
        "random", "nearly_sorted", "reverse", "duplicates"
    };

    for (const auto& dsType : dsTypes) {
        // ── Merge Sort ──
        {
            auto data = gen.generate(10000, dsType);

            // Referensi: sequential
            std::vector<int> seqData = data;
            MergeSortSeq seqSorter;
            seqSorter.sort(seqData);

            // Paralel dengan berbagai konfigurasi thread
            for (int nThreads : {2, 4}) {
                std::vector<int> parData = data;
                MergeSortPar parSorter(nThreads, 512);
                parSorter.sort(parData);

                if (parData == seqData) {
                    reportPass("MergeSortPar T=" + std::to_string(nThreads) +
                               " correctness: " + dsType);
                } else {
                    reportFail("MergeSortPar T=" + std::to_string(nThreads) +
                               " correctness: " + dsType,
                               "output berbeda dari sequential");
                }
            }
        }

        // ── Bitonic Sort ──
        {
            auto raw = gen.generate(1024, dsType); // 2^10
            std::vector<int> padded = padPow2(raw);

            // Referensi: sequential
            std::vector<int> seqData = padded;
            BitonicSortSeq seqSorter;
            seqSorter.sort(seqData);

            // Paralel dengan berbagai schedule
            for (const auto& sched : {"static", "dynamic", "guided"}) {
                std::vector<int> parData = padded;
                BitonicSortPar parSorter(2, sched, 64);
                parSorter.sort(parData);

                if (parData == seqData) {
                    reportPass(std::string("BitonicSortPar sched=") + sched +
                               " correctness: " + dsType);
                } else {
                    reportFail(std::string("BitonicSortPar sched=") + sched +
                               " correctness: " + dsType,
                               "output berbeda dari sequential");
                }
            }
        }
    }
}

// =============================================================================
// ── EDGE CASE TESTS ───────────────────────────────────────────────────────────
// Input kosong, N=1, N=2, semua elemen sama, sudah terurut, reverse
// =============================================================================
void runEdgeCaseTests() {
    std::cout << "\n[3] EDGE CASE TESTS\n";

    // Edge 1: Input kosong
    {
        std::vector<int> data = {};
        MergeSortSeq sorter;
        sorter.sort(data);
        if (data.empty()) {
            reportPass("MergeSortSeq: input kosong → tetap kosong");
        } else {
            reportFail("MergeSortSeq: input kosong", "harusnya tetap kosong");
        }
    }

    // Edge 2: Input kosong (paralel)
    {
        std::vector<int> data = {};
        MergeSortPar sorter(4, 512);
        sorter.sort(data);
        if (data.empty()) {
            reportPass("MergeSortPar: input kosong → tetap kosong");
        } else {
            reportFail("MergeSortPar: input kosong", "harusnya tetap kosong");
        }
    }

    // Edge 3: N=1 (satu elemen)
    {
        std::vector<int> data = {42};
        MergeSortSeq sorter;
        sorter.sort(data);
        if (data == std::vector<int>{42}) {
            reportPass("MergeSortSeq: N=1 → tidak berubah");
        } else {
            reportFail("MergeSortSeq: N=1", "harusnya {42}");
        }
    }

    // Edge 4: N=1 (paralel)
    {
        std::vector<int> data = {42};
        MergeSortPar sorter(4, 512);
        sorter.sort(data);
        if (data == std::vector<int>{42}) {
            reportPass("MergeSortPar: N=1 → tidak berubah");
        } else {
            reportFail("MergeSortPar: N=1", "harusnya {42}");
        }
    }

    // Edge 5: N=2
    {
        std::vector<int> data = {99, 1};
        MergeSortPar sorter(4, 512);
        sorter.sort(data);
        if (data == std::vector<int>{1, 99}) {
            reportPass("MergeSortPar: N=2 → terurut benar");
        } else {
            reportFail("MergeSortPar: N=2", "harusnya {1, 99}");
        }
    }

    // Edge 6: Semua elemen sama
    {
        std::vector<int> data(100, 7);
        MergeSortPar sorter(4, 32);
        sorter.sort(data);
        bool ok = true;
        for (int x : data) if (x != 7) { ok = false; break; }
        if (ok && (int)data.size() == 100) {
            reportPass("MergeSortPar: semua elemen sama (100 × 7)");
        } else {
            reportFail("MergeSortPar: semua elemen sama", "output tidak benar");
        }
    }

    // Edge 7: Sudah terurut ascending
    {
        std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8};
        MergeSortPar sorter(2, 2);
        sorter.sort(data);
        if (isSorted(data)) {
            reportPass("MergeSortPar: input sudah terurut → tetap terurut");
        } else {
            reportFail("MergeSortPar: input sudah terurut", "output tidak terurut");
        }
    }

    // Edge 8: Sudah terurut descending (reverse)
    {
        std::vector<int> data = {8, 7, 6, 5, 4, 3, 2, 1};
        MergeSortPar sorter(2, 2);
        sorter.sort(data);
        if (data == std::vector<int>{1,2,3,4,5,6,7,8}) {
            reportPass("MergeSortPar: input reverse → terurut benar");
        } else {
            reportFail("MergeSortPar: input reverse", "output tidak benar");
        }
    }

    // Edge 9: Bitonic Sort N=1 (pad ke 1)
    {
        std::vector<int> data = {99};
        // Bitonic perlu 2^k, N=1 = 2^0, valid
        BitonicSortSeq sorter;
        sorter.sort(data);
        if (data == std::vector<int>{99}) {
            reportPass("BitonicSortSeq: N=1 → tidak berubah");
        } else {
            reportFail("BitonicSortSeq: N=1", "harusnya {99}");
        }
    }

    // Edge 10: Bitonic Sort semua elemen sama
    {
        std::vector<int> data(8, 5); // 2^3
        BitonicSortPar sorter(2, "static", 2);
        sorter.sort(data);
        bool ok = true;
        for (int x : data) if (x != 5) { ok = false; break; }
        if (ok) {
            reportPass("BitonicSortPar: semua elemen sama (8 × 5)");
        } else {
            reportFail("BitonicSortPar: semua elemen sama", "output tidak benar");
        }
    }
}

// =============================================================================
// ── CONCURRENCY TESTS ────────────────────────────────────────────────────────
// Non-determinism check: jalankan 2x dengan input sama → hasil harus sama
// =============================================================================
void runConcurrencyTests() {
    std::cout << "\n[4] CONCURRENCY TESTS (Non-Determinism Check)\n";

    DatasetGenerator gen(99);

    // Test 1: MergeSortPar deterministik
    {
        auto original = gen.generate(50000, "random");

        std::vector<int> run1 = original;
        MergeSortPar sorter1(4, 512);
        sorter1.sort(run1);

        std::vector<int> run2 = original;
        MergeSortPar sorter2(4, 512);
        sorter2.sort(run2);

        if (run1 == run2) {
            reportPass("MergeSortPar: deterministik (2 run identik, 50K elemen)");
        } else {
            reportFail("MergeSortPar: deterministik", "non-deterministic! run1 != run2");
        }
    }

    // Test 2: BitonicSortPar deterministik
    {
        auto raw = gen.generate(4096, "random"); // 2^12
        auto original = padPow2(raw);

        std::vector<int> run1 = original;
        BitonicSortPar sorter1(4, "guided", 128);
        sorter1.sort(run1);

        std::vector<int> run2 = original;
        BitonicSortPar sorter2(4, "guided", 128);
        sorter2.sort(run2);

        if (run1 == run2) {
            reportPass("BitonicSortPar: deterministik (2 run identik, 4096 elemen)");
        } else {
            reportFail("BitonicSortPar: deterministik", "non-deterministic! run1 != run2");
        }
    }

    // Test 3: MergeSortPar dengan thread berbeda hasilkan output sama
    {
        auto original = gen.generate(10000, "reverse");

        std::vector<int> run2t = original;
        MergeSortPar sorter2(2, 512);
        sorter2.sort(run2t);

        std::vector<int> run4t = original;
        MergeSortPar sorter4(4, 512);
        sorter4.sort(run4t);

        std::vector<int> run8t = original;
        MergeSortPar sorter8(8, 512);
        sorter8.sort(run8t);

        if (run2t == run4t && run4t == run8t) {
            reportPass("MergeSortPar: hasil sama untuk 2/4/8 thread (10K reverse)");
        } else {
            reportFail("MergeSortPar: hasil sama untuk 2/4/8 thread",
                       "output berbeda antar konfigurasi thread");
        }
    }
}

// =============================================================================
// ── INTEGRATION TESTS ────────────────────────────────────────────────────────
// Pipeline: DatasetGenerator → Sorter → validasi output benar
// =============================================================================
void runIntegrationTests() {
    std::cout << "\n[5] INTEGRATION TESTS (Pipeline Lengkap)\n";

    DatasetGenerator gen(123);

    // Integration 1: Pipeline Merge Sort lengkap
    {
        auto data = gen.generate(100000, "random");
        std::vector<int> parData = data;

        MergeSortPar sorter(4, 1024);
        sorter.sort(parData);

        // Validasi: terurut
        bool sorted = isSorted(parData);
        // Validasi: ukuran tidak berubah
        bool sizeOk = (parData.size() == data.size());

        if (sorted && sizeOk) {
            reportPass("Integration: DatasetGenerator → MergeSortPar → terurut benar (100K)");
        } else {
            reportFail("Integration: DatasetGenerator → MergeSortPar",
                       sorted ? "ukuran berubah" : "tidak terurut");
        }
    }

    // Integration 2: Pipeline Bitonic Sort lengkap
    {
        auto raw = gen.generate(65536, "nearly_sorted"); // 2^16
        auto padded = padPow2(raw);

        BitonicSortPar sorter(4, "guided", 512);
        sorter.sort(padded);

        bool sorted = isSorted(padded);
        if (sorted) {
            reportPass("Integration: DatasetGenerator → BitonicSortPar → terurut benar (65536)");
        } else {
            reportFail("Integration: DatasetGenerator → BitonicSortPar", "tidak terurut");
        }
    }

    // Integration 3: padToPow2 dan elemen asli tetap urut setelah remove padding
    {
        auto raw = gen.generate(1000, "random");
        int originalSize = (int)raw.size();
        auto padded = gen.padToPow2(raw, 1024); // 2^10

        BitonicSortPar sorter(4, "static", 64);
        sorter.sort(padded);

        gen.removePadding(padded, originalSize);

        bool sorted = isSorted(padded);
        bool sizeOk = ((int)padded.size() == originalSize);

        if (sorted && sizeOk) {
            reportPass("Integration: padToPow2 → sort → removePadding → 1000 elemen benar");
        } else {
            reportFail("Integration: padToPow2 → sort → removePadding",
                       sizeOk ? "tidak terurut setelah remove padding" : "ukuran salah");
        }
    }

    // Integration 4: semua tipe dataset bisa diproses tanpa error
    {
        bool allOk = true;
        for (const auto& dsType : {"random","nearly_sorted","reverse","duplicates"}) {
            try {
                auto data = gen.generate(5000, dsType);
                MergeSortPar sorter(4, 512);
                sorter.sort(data);
                if (!isSorted(data)) { allOk = false; break; }
            } catch (...) {
                allOk = false; break;
            }
        }
        if (allOk) {
            reportPass("Integration: semua 4 tipe dataset diproses benar oleh MergeSortPar");
        } else {
            reportFail("Integration: semua tipe dataset", "ada tipe yang gagal");
        }
    }

    // Integration 5: MergeSortPar vs std::sort sebagai ground truth
    {
        auto data = gen.generate(10000, "random");
        std::vector<int> stdSorted = data;
        std::sort(stdSorted.begin(), stdSorted.end());

        std::vector<int> parData = data;
        MergeSortPar sorter(4, 512);
        sorter.sort(parData);

        if (parData == stdSorted) {
            reportPass("Integration: MergeSortPar == std::sort (ground truth, 10K)");
        } else {
            reportFail("Integration: MergeSortPar == std::sort", "output berbeda dari std::sort");
        }
    }
}

// =============================================================================
// MAIN
// =============================================================================
int main() {
    std::cout << "================================================\n";
    std::cout << " UNIT TESTING — Parallel Sorting System\n";
    std::cout << " KPT Minggu 3 | Correctness First\n";
    std::cout << "================================================\n";

    runSmokeTests();
    runCorrectnessTests();
    runEdgeCaseTests();
    runConcurrencyTests();
    runIntegrationTests();

    std::cout << "\n================================================\n";
    std::cout << " HASIL: " << passed << " PASS | " << failed << " FAIL\n";
    std::cout << "================================================\n";

    return failed > 0 ? 1 : 0;
}


test: implementasi 43 unit test dan coverage report 100 persen dari 36 fungsi
