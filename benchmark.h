#pragma once
// =============================================================================
// benchmark.h - Header BenchmarkEngine + CSVExporter
// =============================================================================

#include <vector>
#include <string>
#include <functional>
#include <fstream>

// =============================================================================
// BenchmarkEngine: mengukur waktu, menghitung speedup, export CSV
// =============================================================================
class BenchmarkEngine {
public:
    explicit BenchmarkEngine(const std::string& csvPath);

    void initCSV();

    double measureSequential(
        std::function<void()> sortFn,
        std::vector<int>& data,
        int repetitions,
        const std::string& algorithm,
        const std::string& datasetType
    );

    double measureParallel(
        std::function<void(int, int)> sortFn,
        std::vector<int>& data,
        int repetitions,
        int numThreads,
        int cutoff,
        const std::string& algorithm,
        const std::string& datasetType
    );

    double benchmarkSequential(std::vector<int>& data, int repetitions);
    double benchmarkBitonicSeq(std::vector<int>& data, int repetitions);

    // DIUPDATE: tambah parameter cutoff untuk Prinsip 02 Granularitas
    double measureBitonicParallel(
        std::vector<int>& data,
        int repetitions,
        int numThreads,
        const std::string& schedule,
        int cutoff,
        const std::string& algorithm,
        const std::string& datasetType
    );

    void writeRow(
        const std::string& algorithm,
        const std::string& datasetType,
        int datasetSize,
        double executionTime,
        int threadCount,
        double speedup,
        double efficiency,
        const std::string& scheduleType,
        int cutoffThreshold,
        const std::string& validationStatus
    );

private:
    std::string csvPath_;
    std::ofstream csvFile_;

    double filterOutliers(std::vector<double>& times);
    bool validateSorted(const std::vector<int>& data);
};
