#pragma once
// =============================================================================
// dataset_generator.h - Header DatasetGenerator
// =============================================================================

#include <vector>
#include <string>
#include <random>
#include <stdexcept>

class DatasetGenerator {
public:
    explicit DatasetGenerator(uint32_t seed = 42);

    // Generate dataset sesuai tipe
    std::vector<int> generate(int size, const std::string& type);

    // Pad array ke ukuran target menggunakan INT_MAX sebagai sentinel
    std::vector<int> padToPow2(const std::vector<int>& data, int targetSize);

    // Hapus padding (elemen INT_MAX) setelah sorting
    void removePadding(std::vector<int>& data, int originalSize);

private:
    std::mt19937 rng_;

    std::vector<int> generateRandom(int size);
    std::vector<int> generateNearlySorted(int size);
    std::vector<int> generateReverse(int size);
    std::vector<int> generateDuplicates(int size);
};
