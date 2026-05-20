// =============================================================================
// benchmark.cpp - Implementasi BenchmarkEngine
// =============================================================================

#include "benchmark.h"
#include "merge_sort.h"
#include "bitonic_sort.h"
#include "fault_tolerance.h"
#include <chrono>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <stdexcept>

using Clock = std::chrono::high_resolution_clock;
using Ms    = std::chrono::duration<double, std::milli>;

BenchmarkEngine::BenchmarkEngine(const std::string& csvPath)
    : csvPath_(csvPath) {}

// -----------------------------------------------------------------------------
// initCSV() - Buat file CSV dengan header
// -----------------------------------------------------------------------------
void BenchmarkEngine::initCSV() {
    csvFile_.open(csvPath_, std::ios::out | std::ios::trunc);
    if (!csvFile_.is_open()) {
        throw std::runtime_error("Tidak bisa membuka file CSV: " + csvPath_);
    }
    csvFile_ << "algorithm,dataset_type,dataset_size,execution_time,"
             << "thread_count,speedup,efficiency,schedule_type,"
             << "cutoff_threshold,validation_status\n";
    csvFile_.flush();
}

// -----------------------------------------------------------------------------
// filterOutliers() - Hitung rata-rata setelah buang outlier ±2σ
// -----------------------------------------------------------------------------
double BenchmarkEngine::filterOutliers(std::vector<double>& times) {
    if (times.empty()) return 0.0;
    if (times.size() == 1) return times[0];

    double mean = std::accumulate(times.begin(), times.end(), 0.0) / times.size();

    double variance = 0.0;
    for (double t : times) variance += (t - mean) * (t - mean);
    variance /= times.size();
    double stddev = std::sqrt(variance);

    // Filter outlier di luar rentang [mean - 2σ, mean + 2σ]
    std::vector<double> filtered;
    for (double t : times) {
        if (std::abs(t - mean) <= 2.0 * stddev) {
            filtered.push_back(t);
        }
    }
    if (filtered.empty()) return mean; // fallback jika semua difilter

    return std::accumulate(filtered.begin(), filtered.end(), 0.0) / filtered.size();
}

// -----------------------------------------------------------------------------
// validateSorted() - Cek apakah array sudah terurut ascending
// -----------------------------------------------------------------------------
bool BenchmarkEngine::validateSorted(const std::vector<int>& data) {
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i] < data[i - 1]) return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
// measureSequential() - Ukur waktu sequential sort
// -----------------------------------------------------------------------------
double BenchmarkEngine::measureSequential(
    std::function<void()> sortFn,
    std::vector<int>& data,
    int repetitions,
    const std::string& /*algorithm*/,
    const std::string& /*datasetType*/)
{
    // Simpan data asli
    std::vector<int> original = data;
    std::vector<double> times;
    times.reserve(repetitions);

    RetryPolicy policy{2, 50, true}; // max 2 retry, backoff 50ms

    for (int r = 0; r < repetitions; ++r) {
        data = original;

        auto start = Clock::now();
        bool ok = withRetry([&]() { sortFn(); }, policy, "measureSequential");
        auto end = Clock::now();

        if (!ok) {
            std::cerr << "[WARNING] measureSequential: semua retry gagal, skip repetisi " << r << "\n";
            continue;
        }

        times.push_back(Ms(end - start).count());

        if (r == 0 && !validateSorted(data)) {
            std::cerr << "[WARNING] Validasi GAGAL untuk sequential sort!\n";
        }
    }

    return filterOutliers(times);
}

// -----------------------------------------------------------------------------
// measureParallel() - Ukur waktu parallel merge sort
// -----------------------------------------------------------------------------
double BenchmarkEngine::measureParallel(
    std::function<void(int, int)> sortFn,
    std::vector<int>& data,
    int repetitions,
    int numThreads,
    int cutoff,
    const std::string& /*algorithm*/,
    const std::string& /*datasetType*/)
{
    std::vector<int> original = data;
    std::vector<double> times;
    times.reserve(repetitions);

    RetryPolicy policy{2, 50, true};

    for (int r = 0; r < repetitions; ++r) {
        data = original;

        auto start = Clock::now();
        bool ok = withRetry([&]() { sortFn(numThreads, cutoff); }, policy, "measureParallel");
        auto end = Clock::now();

        if (!ok) {
            std::cerr << "[WARNING] measureParallel: semua retry gagal, skip repetisi " << r << "\n";
            continue;
        }

        times.push_back(Ms(end - start).count());

        if (r == 0 && !validateSorted(data)) {
            std::cerr << "[WARNING] Validasi GAGAL untuk parallel merge sort!\n";
        }
    }

    return filterOutliers(times);
}

// -----------------------------------------------------------------------------
// benchmarkSequential() - Baseline merge sort (tanpa CSV write)
// -----------------------------------------------------------------------------
double BenchmarkEngine::benchmarkSequential(std::vector<int>& data, int repetitions) {
    std::vector<int> original = data;
    std::vector<double> times;
    times.reserve(repetitions);

    for (int r = 0; r < repetitions; ++r) {
        data = original;
        MergeSortSeq sorter;

        auto start = Clock::now();
        sorter.sort(data);
        auto end = Clock::now();

        times.push_back(Ms(end - start).count());
    }

    return filterOutliers(times);
}

// -----------------------------------------------------------------------------
// benchmarkBitonicSeq() - Baseline bitonic sort (tanpa CSV write)
// -----------------------------------------------------------------------------
double BenchmarkEngine::benchmarkBitonicSeq(std::vector<int>& data, int repetitions) {
    std::vector<int> original = data;
    std::vector<double> times;
    times.reserve(repetitions);

    for (int r = 0; r < repetitions; ++r) {
        data = original;
        BitonicSortSeq sorter;

        auto start = Clock::now();
        sorter.sort(data);
        auto end = Clock::now();

        times.push_back(Ms(end - start).count());
    }

    return filterOutliers(times);
}

// -----------------------------------------------------------------------------
// measureBitonicParallel() - Ukur waktu parallel bitonic sort
// -----------------------------------------------------------------------------
double BenchmarkEngine::measureBitonicParallel(
    std::vector<int>& data,
    int repetitions,
    int numThreads,
    const std::string& schedule,
    int cutoff,
    const std::string& /*algorithm*/,
    const std::string& /*datasetType*/)
{
    std::vector<int> original = data;
    std::vector<double> times;
    times.reserve(repetitions);

    for (int r = 0; r < repetitions; ++r) {
        data = original;
        BitonicSortPar sorter(numThreads, schedule, cutoff);

        auto start = Clock::now();
        sorter.sort(data);
        auto end = Clock::now();

        times.push_back(Ms(end - start).count());

        if (r == 0 && !validateSorted(data)) {
            std::cerr << "[WARNING] Validasi GAGAL untuk parallel bitonic sort!\n";
        }
    }

    return filterOutliers(times);
}

// -----------------------------------------------------------------------------
// writeRow() - Tulis satu baris hasil benchmark ke CSV
// -----------------------------------------------------------------------------
void BenchmarkEngine::writeRow(
    const std::string& algorithm,
    const std::string& datasetType,
    int datasetSize,
    double executionTime,
    int threadCount,
    double speedup,
    double efficiency,
    const std::string& scheduleType,
    int cutoffThreshold,
    const std::string& validationStatus)
{
    if (!csvFile_.is_open()) {
        csvFile_.open(csvPath_, std::ios::app);
    }

    csvFile_ << std::fixed << std::setprecision(4)
             << algorithm       << ","
             << datasetType     << ","
             << datasetSize     << ","
             << executionTime   << ","
             << threadCount     << ","
             << speedup         << ","
             << efficiency      << ","
             << scheduleType    << ","
             << cutoffThreshold << ","
             << validationStatus << "\n";

    csvFile_.flush(); // flush langsung agar realtime_plot bisa membaca
}
