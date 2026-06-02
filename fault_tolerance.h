#pragma once
// =============================================================================
// fault_tolerance.h - Fault Tolerance untuk Parallel Sorting System
//
// Mengikuti panduan KPT Minggu 3 (hal. 7):
//   [R] Retry Logic        - exponential backoff saat task gagal
//   [C] Checkpointing      - simpan state tiap N iterasi ke disk
//   [D] Task Replication   - (tidak diimplementasi: shared memory OpenMP
//                            tidak butuh replikasi — crash 1 thread = crash
//                            seluruh proses, bukan partial failure)
//
// Catatan desain:
//   Retry dan checkpoint relevan di sini untuk melindungi dari:
//   - Exception saat sorting (misal: data invalid, out of memory)
//   - Benchmark yang terlalu lama / hang
// =============================================================================

#include <vector>
#include <string>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

// =============================================================================
// RetryPolicy - konfigurasi parameter retry
// =============================================================================
struct RetryPolicy {
    int maxAttempts   = 3;      // Maksimum percobaan
    int baseDelayMs   = 100;    // Delay awal dalam milidetik
    bool exponential  = true;   // true = exponential backoff, false = fixed
};

// =============================================================================
// withRetry() - jalankan fungsi dengan retry logic + exponential backoff
//
// Mengikuti pola dari PDF KPT Minggu 3 hal. 7:
//   MAX_RETRY = 3
//   Exponential backoff: tunggu 100ms, 200ms, 400ms antar retry
//
// Args:
//   fn          - fungsi yang akan dijalankan
//   policy      - konfigurasi retry (maxAttempts, baseDelayMs)
//   taskName    - nama task untuk logging
//
// Returns:
//   true jika berhasil, false jika semua percobaan gagal
//
// Cara pakai:
//   bool ok = withRetry([&]() {
//       sorter.sort(data);
//   }, RetryPolicy{3, 100}, "MergeSortPar");
// =============================================================================
inline bool withRetry(
    std::function<void()> fn,
    const RetryPolicy& policy = RetryPolicy{},
    const std::string& taskName = "task"
) {
    for (int attempt = 1; attempt <= policy.maxAttempts; ++attempt) {
        try {
            fn();
            if (attempt > 1) {
                std::cout << "[RETRY] " << taskName
                          << " berhasil pada percobaan ke-" << attempt << "\n";
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[RETRY] " << taskName
                      << " gagal (percobaan " << attempt << "/"
                      << policy.maxAttempts << "): " << e.what() << "\n";

            if (attempt == policy.maxAttempts) {
                std::cerr << "[RETRY] " << taskName
                          << " gagal setelah " << policy.maxAttempts
                          << " percobaan. Tidak ada retry lagi.\n";
                return false;
            }

            // Exponential backoff: delay * 2^(attempt-1)
            int delayMs = policy.exponential
                ? policy.baseDelayMs * (1 << (attempt - 1))
                : policy.baseDelayMs;

            std::cerr << "[RETRY] Menunggu " << delayMs << "ms sebelum retry...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
        }
    }
    return false;
}

// =============================================================================
// CheckpointManager - simpan dan resume state benchmark
//
// Mengikuti pola dari PDF KPT Minggu 3 hal. 7:
//   Simpan ke disk tiap N iterasi
//   Resume dari checkpoint terakhir jika crash
//
// Format checkpoint file (CSV sederhana):
//   algorithm,dataset_type,thread_count,cutoff,elapsed_ms,status
// =============================================================================
class CheckpointManager {
public:
    // =========================================================================
    // Constructor
    // Args:
    //   checkpointPath - path file checkpoint (misal: "results/checkpoint.csv")
    //   interval       - simpan tiap N operasi
    // =========================================================================
    explicit CheckpointManager(const std::string& checkpointPath,
                               int interval = 10)
        : path_(checkpointPath), interval_(interval), opCount_(0) {}

    // =========================================================================
    // save() - simpan state saat ini ke file checkpoint
    //
    // Args:
    //   algorithm   - nama algoritma yang sedang berjalan
    //   datasetType - tipe dataset
    //   nThreads    - jumlah thread
    //   cutoff      - nilai cutoff
    //   elapsedMs   - waktu yang sudah berjalan
    //   status      - "IN_PROGRESS" atau "DONE"
    // =========================================================================
    void save(const std::string& algorithm,
              const std::string& datasetType,
              int nThreads,
              int cutoff,
              double elapsedMs,
              const std::string& status = "IN_PROGRESS")
    {
        opCount_++;
        // Hanya simpan setiap interval operasi untuk hindari I/O overhead
        if (opCount_ % interval_ != 0 && status != "DONE") return;

        std::ofstream f(path_, std::ios::app);
        if (!f.is_open()) {
            std::cerr << "[CHECKPOINT] Gagal buka file: " << path_ << "\n";
            return;
        }
        f << algorithm << "," << datasetType << ","
          << nThreads << "," << cutoff << ","
          << elapsedMs << "," << status << "\n";
    }

    // =========================================================================
    // exists() - cek apakah checkpoint tersedia untuk resume
    // =========================================================================
    bool exists() const {
        std::ifstream f(path_);
        return f.good();
    }

    // =========================================================================
    // getLastCompleted() - baca baris terakhir checkpoint untuk resume
    //
    // Returns:
    //   string berisi baris terakhir, atau "" jika tidak ada
    // =========================================================================
    std::string getLastCompleted() const {
        std::ifstream f(path_);
        if (!f.is_open()) return "";

        std::string line, lastLine;
        while (std::getline(f, line)) {
            if (line.find("DONE") != std::string::npos) {
                lastLine = line;
            }
        }
        return lastLine;
    }

    // =========================================================================
    // clear() - hapus file checkpoint (setelah benchmark selesai penuh)
    // =========================================================================
    void clear() {
        std::remove(path_.c_str());
        std::cout << "[CHECKPOINT] File checkpoint dihapus: " << path_ << "\n";
    }

private:
    std::string path_;
    int interval_;
    int opCount_;
};
