#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "matrix.h"
#include <vector>

struct BenchmarkResult
{
    int threadCount;
    double averageTimeMs;
    double speedup;
};

double measureParallel(
    const Matrix& A,
    const Matrix& B,
    int n,
    int threadCount,
    int runs
);

std::vector<BenchmarkResult> runBenchmark(
    const Matrix& A,
    const Matrix& B,
    int n,
    int runs
);

void printBenchmarkResults(
    const std::vector<BenchmarkResult>& results
);

#endif
