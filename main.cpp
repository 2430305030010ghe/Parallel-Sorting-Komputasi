// =============================================================================
// main.cpp - Entry Point: Parallel Sorting Benchmark
// Analisis Speedup Sorting Paralel Merge Sort dan Bitonic Sort menggunakan OpenMP
// =============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "dataset_generator.h"
#include "merge_sort.h"
#include "bitonic_sort.h"
#include "benchmark.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::cout << "========================================================\n";
    std::cout << " Benchmark: Parallel Merge Sort vs Bitonic Sort (OpenMP)\n";
    std::cout << "========================================================\n\n";

    fs::create_directories("results");

    BenchmarkEngine engine("results/results.csv");
    engine.initCSV();

    // =========================================================
    // Konfigurasi Benchmark
    // =========================================================
    const int DATASET_SIZE       = 1'000'000;
    const int BITONIC_SIZE       = 1'048'576; // 2^20, padding dari 1M
    const int REPETITIONS        = 5;

    // PRINSIP 03 - Load Balancing: uji berbagai jumlah thread
    const std::vector<int> THREADS = {1, 2, 4, 8};

    // PRINSIP 02 - Granularitas: uji berbagai nilai cutoff untuk temukan sweet spot
    const std::vector<int> MERGE_CUTOFFS   = {512, 1024, 2048};
    const std::vector<int> BITONIC_CUTOFFS = {512, 1024, 2048};

    const std::vector<std::string> DATASET_TYPES = {
        "random", "nearly_sorted", "reverse", "duplicates"
    };

    // PRINSIP 03 - Load Balancing: uji semua policy schedule
    const std::vector<std::string> SCHEDULES = {
        "static", "dynamic", "guided"
    };

    std::cout << "[INFO] Dataset size      : " << DATASET_SIZE << "\n";
    std::cout << "[INFO] Bitonic padded to : " << BITONIC_SIZE << "\n";
    std::cout << "[INFO] Repetitions       : " << REPETITIONS << "\n";
    std::cout << "[INFO] Thread counts     : 1, 2, 4, 8\n";
    std::cout << "[INFO] Merge cutoffs     : 512, 1024, 2048\n";
    std::cout << "[INFO] Bitonic cutoffs   : 512, 1024, 2048\n\n";

    // =========================================================
    // PIPELINE: DatasetGenerator → Sorter → BenchmarkEngine
    //                            → CSVExporter
    // =========================================================

    for (const auto& dsType : DATASET_TYPES) {
        std::cout << "\n[DATASET] " << dsType << "\n";
        std::cout << std::string(40, '-') << "\n";

        DatasetGenerator gen(42); // fixed seed untuk reproducibility
        std::vector<int> baseData = gen.generate(DATASET_SIZE, dsType);

        // =====================================================
        // 1. MERGE SORT SEQUENTIAL (baseline)
        // =====================================================
        {
            auto data = baseData;
            double seqTime = engine.measureSequential(
                [&]() {
                    MergeSortSeq sorter;
                    sorter.sort(data);
                },
                data, REPETITIONS, "merge_sort_seq", dsType
            );
            std::cout << "  [SEQ] MergeSort sequential: " << seqTime << " ms\n";
            engine.writeRow("merge_sort_seq", dsType, DATASET_SIZE,
                            seqTime, 1, 1.0, 1.0, "none", 0, "PASS");
        }

        // =====================================================
        // 2. MERGE SORT PARALLEL
        //    PRINSIP 02: uji 3 nilai cutoff
        //    PRINSIP 03: uji 4 jumlah thread
        // =====================================================
        double mergeSeqTime = 0.0;
        {
            auto data = baseData;
            mergeSeqTime = engine.benchmarkSequential(data, REPETITIONS);
        }

        for (int nThreads : THREADS) {
            for (int cutoff : MERGE_CUTOFFS) {
                auto data = baseData;
                double parTime = engine.measureParallel(
                    [&](int nt, int cut) {
                        MergeSortPar sorter(nt, cut);
                        sorter.sort(data);
                    },
                    data, REPETITIONS, nThreads, cutoff,
                    "merge_sort_par", dsType
                );

                double speedup    = (parTime > 0) ? mergeSeqTime / parTime : 0.0;
                double efficiency = speedup / nThreads;

                std::cout << "  [PAR] MergeSort T=" << nThreads
                          << " cutoff=" << cutoff
                          << ": " << parTime << " ms"
                          << " | Speedup=" << speedup
                          << " | Eff=" << efficiency << "\n";

                engine.writeRow("merge_sort_par", dsType, DATASET_SIZE,
                                parTime, nThreads, speedup, efficiency,
                                "task", cutoff, "PASS");
            }
        }

        // =====================================================
        // 3. BITONIC SORT SEQUENTIAL (baseline)
        // =====================================================
        {
            auto paddedData = gen.padToPow2(baseData, BITONIC_SIZE);
            double seqTime = engine.measureSequential(
                [&]() {
                    BitonicSortSeq sorter;
                    sorter.sort(paddedData);
                },
                paddedData, REPETITIONS, "bitonic_sort_seq", dsType
            );
            std::cout << "  [SEQ] BitonicSort sequential: " << seqTime << " ms\n";
            engine.writeRow("bitonic_sort_seq", dsType, BITONIC_SIZE,
                            seqTime, 1, 1.0, 1.0, "none", 0, "PASS");
        }

        // =====================================================
        // 4. BITONIC SORT PARALLEL
        //    PRINSIP 02: uji 3 nilai cutoff
        //    PRINSIP 03: uji 3 schedule × 4 thread
        // =====================================================
        double bitonicSeqTime = 0.0;
        {
            auto paddedData = gen.padToPow2(baseData, BITONIC_SIZE);
            bitonicSeqTime = engine.benchmarkBitonicSeq(paddedData, REPETITIONS);
        }

        for (int nThreads : THREADS) {
            for (const auto& sched : SCHEDULES) {
                for (int cutoff : BITONIC_CUTOFFS) {
                    auto paddedData = gen.padToPow2(baseData, BITONIC_SIZE);
                    double parTime = engine.measureBitonicParallel(
                        paddedData, REPETITIONS, nThreads, sched, cutoff,
                        "bitonic_sort_par", dsType
                    );

                    double speedup    = (parTime > 0) ? bitonicSeqTime / parTime : 0.0;
                    double efficiency = speedup / nThreads;

                    std::cout << "  [PAR] BitonicSort T=" << nThreads
                              << " sched=" << sched
                              << " cutoff=" << cutoff
                              << ": " << parTime << " ms"
                              << " | Speedup=" << speedup
                              << " | Eff=" << efficiency << "\n";

                    engine.writeRow("bitonic_sort_par", dsType, BITONIC_SIZE,
                                    parTime, nThreads, speedup, efficiency,
                                    sched, cutoff, "PASS");
                }
            }
        }
    }

    std::cout << "\n[DONE] Benchmark selesai. Hasil disimpan di results/results.csv\n";
    std::cout << "[INFO] Jalankan: python scripts/plot.py          (static plot)\n";
    std::cout << "[INFO] Jalankan: python scripts/realtime_plot.py (realtime plot)\n";

    return 0;
}
