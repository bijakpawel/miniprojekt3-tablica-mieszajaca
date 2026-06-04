#include "hash_tables.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;

struct SummaryRow {
    std::string variant;
    std::string operation;
    std::size_t size;
    double total_average_ns;
    double per_operation_average_ns;
};

template <typename TableFactory>
SummaryRow benchmark_operation(const std::string& variant, const std::string& operation, std::size_t size, int repeats, TableFactory factory, const std::vector<int>& keys)
{
    std::vector<double> measurements;
    measurements.reserve(repeats);

    for (int repeat = 0; repeat < repeats; ++repeat) {
        auto table = factory();

        const auto insert_start = Clock::now();
        for (std::size_t index = 0; index < keys.size(); ++index) {
            table.insert(keys[index], static_cast<int>(index));
        }
        const auto insert_end = Clock::now();

        if (table.size() != keys.size()) {
            throw std::runtime_error("Unexpected table size after insert phase for " + variant);
        }

        const auto remove_start = Clock::now();
        for (auto it = keys.rbegin(); it != keys.rend(); ++it) {
            if (!table.remove(*it)) {
                throw std::runtime_error("Removal failed for " + variant);
            }
        }
        const auto remove_end = Clock::now();

        if (table.size() != 0) {
            throw std::runtime_error("Unexpected table size after remove phase for " + variant);
        }

        const auto insert_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(insert_end - insert_start).count();
        const auto remove_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(remove_end - remove_start).count();

        if (operation == "insert") {
            measurements.push_back(static_cast<double>(insert_ns));
        } else {
            measurements.push_back(static_cast<double>(remove_ns));
        }
    }

    const double sum = std::accumulate(measurements.begin(), measurements.end(), 0.0);
    const double average = sum / static_cast<double>(measurements.size());
    return SummaryRow{variant, operation, size, average, average / static_cast<double>(size)};
}

template <typename TableFactory>
std::vector<SummaryRow> benchmark_table(const std::string& variant, std::size_t size, int repeats, TableFactory factory)
{
    std::vector<int> keys(size);
    std::iota(keys.begin(), keys.end(), 1);

    std::mt19937 generator(static_cast<std::mt19937::result_type>(1337 + size));
    std::shuffle(keys.begin(), keys.end(), generator);

    std::vector<SummaryRow> rows;
    rows.push_back(benchmark_operation(variant, "insert", size, repeats, factory, keys));
    rows.push_back(benchmark_operation(variant, "remove", size, repeats, factory, keys));
    return rows;
}

template <typename Table>
void smoke_test(const std::string& name)
{
    Table table;
    if (!table.insert(42, 1)) {
        throw std::runtime_error(name + ": first insert should report a new key");
    }
    if (table.insert(42, 2)) {
        throw std::runtime_error(name + ": updating an existing key should not report a new key");
    }
    if (!table.contains(42)) {
        throw std::runtime_error(name + ": contains() failed after insert");
    }
    if (!table.remove(42)) {
        throw std::runtime_error(name + ": remove() failed");
    }
    if (table.contains(42) || table.size() != 0) {
        throw std::runtime_error(name + ": table should be empty after removal");
    }
}

void write_csv(const std::string& path, const std::vector<SummaryRow>& rows)
{
    std::ofstream out(path, std::ios::trunc);
    if (!out) {
        throw std::runtime_error("Cannot open output file: " + path);
    }

    out << "variant,operation,size,total_average_ns,per_operation_average_ns\n";
    out << std::fixed << std::setprecision(2);
    for (const auto& row : rows) {
        out << row.variant << ',' << row.operation << ',' << row.size << ',' << row.total_average_ns << ',' << row.per_operation_average_ns << '\n';
    }
}

int main(int argc, char** argv)
{
    try {
        const std::string output_path = argc > 1 ? argv[1] : "results/benchmark_summary.csv";

        smoke_test<ChainingHashTable>("ChainingHashTable");
        smoke_test<LinearProbingHashTable>("LinearProbingHashTable");
        smoke_test<AvlBucketHashTable>("AvlBucketHashTable");

        const std::vector<std::size_t> sizes = {512, 2048, 8192, 16384};
        const int repeats = 7;

        std::vector<SummaryRow> rows;
        for (std::size_t size : sizes) {
            const auto chaining_rows = benchmark_table("chaining", size, repeats, []() { return ChainingHashTable{}; });
            const auto probing_rows = benchmark_table("linear_probing", size, repeats, []() { return LinearProbingHashTable{}; });
            const auto avl_rows = benchmark_table("avl_buckets", size, repeats, []() { return AvlBucketHashTable{}; });

            rows.insert(rows.end(), chaining_rows.begin(), chaining_rows.end());
            rows.insert(rows.end(), probing_rows.begin(), probing_rows.end());
            rows.insert(rows.end(), avl_rows.begin(), avl_rows.end());
        }

        write_csv(output_path, rows);

        std::cout << "Benchmark finished. Results written to: " << output_path << '\n';
        std::cout << "Open docs/chart_viewer.html and load the CSV to render charts." << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
