#include "benchmark.h"

#include <chrono>
#include <iomanip>
#include <iostream>

double measureParallel(
    const Matrix& A,
    const Matrix& B,
    int n,
    int threadCount,
    int runs)
{
    double totalTime = 0.0;

    {
        std::vector<int> threadForRow;

        Matrix result =
            multiplyParallel(
                A,
                B,
                n,
                threadCount,
                threadForRow
            );

        volatile double check = result[0];
        (void)check;
    }

    for (int run = 0; run < runs; ++run)
    {
        std::vector<int> threadForRow;

        auto start = std::chrono::high_resolution_clock::now();

        Matrix result =
            multiplyParallel(
                A,
                B,
                n,
                threadCount,
                threadForRow
            );

        auto end = std::chrono::high_resolution_clock::now();

        volatile double check = result[0];
        (void)check;

        double elapsed =
            std::chrono::duration<double, std::milli>(
                end - start
            ).count();

        totalTime += elapsed;
    }

    return totalTime / runs;
}

std::vector<BenchmarkResult> runBenchmark(
    const Matrix& A,
    const Matrix& B,
    int n,
    int runs)
{
    const std::vector<int> threadCounts = {1, 2, 4, 8};

    std::vector<BenchmarkResult> results;

    double baselineTime = 0.0;

    for (int threadCount : threadCounts)
    {
        double averageTime =
            measureParallel(
                A,
                B,
                n,
                threadCount,
                runs
            );

        if (threadCount == 1)
            baselineTime = averageTime;

        double speedup = baselineTime / averageTime;

        results.push_back(
            {
                threadCount,
                averageTime,
                speedup
            }
        );
    }

    return results;
}

void printBenchmarkResults(
    const std::vector<BenchmarkResult>& results)
{
    std::cout
        << "\n========================================\n"
        << "             BENCHMARK\n"
        << "========================================\n\n";

    std::cout
        << std::left
        << std::setw(12) << "Threads"
        << std::setw(18) << "Average (ms)"
        << std::setw(15) << "Speedup"
        << '\n';

    std::cout
        << "----------------------------------------\n";

    for (const auto& result : results)
    {
        std::cout
            << std::left
            << std::setw(12) << result.threadCount
            << std::setw(18)
            << std::fixed
            << std::setprecision(3)
            << result.averageTimeMs
            << std::setw(15)
            << std::setprecision(2)
            << result.speedup
            << "x\n";
    }

    std::cout << "\nBenchmark completed successfully.\n";
}
