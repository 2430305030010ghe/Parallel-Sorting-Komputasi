// =============================================================================
// dataset_generator.cpp - Implementasi DatasetGenerator
// =============================================================================

#include "dataset_generator.h"
#include <algorithm>
#include <climits>
#include <stdexcept>
#include <iostream>

DatasetGenerator::DatasetGenerator(uint32_t seed) : rng_(seed) {}

// -----------------------------------------------------------------------------
// generate() - Dispatcher berdasarkan tipe dataset
// -----------------------------------------------------------------------------
std::vector<int> DatasetGenerator::generate(int size, const std::string& type) {
    if (type == "random")         return generateRandom(size);
    if (type == "nearly_sorted")  return generateNearlySorted(size);
    if (type == "reverse")        return generateReverse(size);
    if (type == "duplicates")     return generateDuplicates(size);
    throw std::invalid_argument("Unknown dataset type: " + type);
}

// -----------------------------------------------------------------------------
// generateRandom() - Data acak sepenuhnya
// -----------------------------------------------------------------------------
std::vector<int> DatasetGenerator::generateRandom(int size) {
    std::uniform_int_distribution<int> dist(1, size * 10);
    std::vector<int> data(size);
    for (auto& v : data) v = dist(rng_);
    return data;
}

// -----------------------------------------------------------------------------
// generateNearlySorted() - 90-95% terurut, sisanya acak
// -----------------------------------------------------------------------------
std::vector<int> DatasetGenerator::generateNearlySorted(int size) {
    std::vector<int> data(size);
    for (int i = 0; i < size; ++i) data[i] = i + 1;

    // Shuffle 5-10% elemen secara acak
    int swapCount = size / 20; // 5% unsorted
    std::uniform_int_distribution<int> dist(0, size - 1);
    for (int i = 0; i < swapCount; ++i) {
        int a = dist(rng_);
        int b = dist(rng_);
        std::swap(data[a], data[b]);
    }
    return data;
}

// -----------------------------------------------------------------------------
// generateReverse() - Data terurut terbalik (worst case)
// -----------------------------------------------------------------------------
std::vector<int> DatasetGenerator::generateReverse(int size) {
    std::vector<int> data(size);
    for (int i = 0; i < size; ++i) data[i] = size - i;
    return data;
}

// -----------------------------------------------------------------------------
// generateDuplicates() - Hanya 10-20 nilai unik
// -----------------------------------------------------------------------------
std::vector<int> DatasetGenerator::generateDuplicates(int size) {
    const int uniqueValues = 15; // 15 nilai unik
    std::uniform_int_distribution<int> dist(1, uniqueValues);
    std::vector<int> data(size);
    for (auto& v : data) v = dist(rng_);
    return data;
}

// -----------------------------------------------------------------------------
// padToPow2() - Tambahkan INT_MAX sebagai padding ke ukuran target
// -----------------------------------------------------------------------------
std::vector<int> DatasetGenerator::padToPow2(const std::vector<int>& data, int targetSize) {
    if ((int)data.size() > targetSize) {
        throw std::invalid_argument("Data lebih besar dari targetSize");
    }
    std::vector<int> padded = data;
    padded.resize(targetSize, INT_MAX); // INT_MAX sebagai sentinel padding
    return padded;
}

// -----------------------------------------------------------------------------
// removePadding() - Truncate ke originalSize setelah sorting
// -----------------------------------------------------------------------------
void DatasetGenerator::removePadding(std::vector<int>& data, int originalSize) {
    if ((int)data.size() > originalSize) {
        data.resize(originalSize);
    }
}
