#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "src/matrix.h"
#include "src/benchmark.h"

namespace
{
std::mutex jsonMutex;

// iostream's operator<< for double goes through the locale-aware num_put
// facet, which is noticeably slower than snprintf when formatting hundreds
// of thousands of numbers per write. Building the JSON text with snprintf
// into a plain string (then writing that string in one shot) cuts the
// per-write CPU cost substantially, on top of the O(n) vs O(n^2)/O(n^3)
// fixes below.
inline void appendDouble(std::string& s, double v)
{
    char buf[32];
    int len = std::snprintf(buf, sizeof(buf), "%.4f", v);
    s.append(buf, static_cast<size_t>(len));
}

// A and B never change once generated, so serialize them to text exactly
// once instead of re-formatting n*n numbers on every single row callback
// (that redundant work was O(n^3) total and dominated the real compute).
std::string serializeMatrix(const char* name, const Matrix& M, int n)
{
    std::string out;
    out.reserve(static_cast<size_t>(n) * n * 8 + 64);
    out += "  \"";
    out += name;
    out += "\": [";
    for (int i = 0; i < n * n; ++i)
    {
        if (i) out += ",";
        if (i % n == 0) out += "\n    ";
        appendDouble(out, M[static_cast<size_t>(i)]);
    }
    out += "\n  ],\n";
    return out;
}

void writeJson(
    const std::string& matrixAJson,
    const std::string& matrixBJson,
    const Matrix* C,
    int n,
    int threads,
    const ProgressState& progress,
    double executionTimeMs,
    const std::string& verification)
{
    std::lock_guard<std::mutex> lock(jsonMutex);

    std::ofstream out("matrix_data.json", std::ios::trunc);
    if (!out) return;

    out << std::fixed << std::setprecision(4);
    out << "{\n";
    out << "  \"size\": " << n << ",\n";
    out << "  \"threads\": " << threads << ",\n";

    out << matrixAJson;
    out << matrixBJson;

    // Only expose rows whose worker has completed them.
    std::string matrixCJson;
    matrixCJson.reserve(static_cast<size_t>(n) * n * 8 + 32);
    matrixCJson += "  \"matrixC\": [";
    for (int i = 0; i < n * n; ++i)
    {
        if (i) matrixCJson += ",";
        if (i % n == 0) matrixCJson += "\n    ";

        int row = i / n;
        bool completed = progress.completedRow[row].load();

        if (C != nullptr && completed)
            appendDouble(matrixCJson, (*C)[static_cast<size_t>(i)]);
        else
            matrixCJson += "0.0000";
    }
    matrixCJson += "\n  ],\n";
    out << matrixCJson;

    out << "  \"completedRows\": [";
    for (int t = 0; t < threads; ++t)
    {
        if (t) out << ",";
        out << progress.completedRows[t].load();
    }
    out << "],\n";

    out << "  \"activeRows\": [";
    for (int t = 0; t < threads; ++t)
    {
        if (t) out << ",";
        out << progress.activeRows[t].load();
    }
    out << "],\n";

    out << "  \"threadProgress\": [\n";
    for (int t = 0; t < threads; ++t)
    {
        int start = (n * t) / threads;
        int end = (n * (t + 1)) / threads - 1;
        if (t) out << ",\n";
        out << "    {\"completed\":" << progress.completedRows[t].load()
            << ",\"total\":" << (end - start + 1)
            << ",\"start\":" << start
            << ",\"end\":" << end << "}";
    }
    out << "\n  ],\n";

    out << "  \"finished\": "
        << (progress.finished.load() ? "true" : "false") << ",\n";
    out << "  \"verification\": \"" << verification << "\",\n";
    out << "  \"executionTimeMs\": " << executionTimeMs << "\n";
    out << "}\n";
}

} // namespace

int main()
{
    const int N = 100;
    const int threads = 4;
    const int benchmarkRuns = 5;

    std::cout
        << "========================================\n"
        << "       MATRIX MULTIPLICATION\n"
        << "========================================\n\n";

    std::cout << "Matrix size : " << N << " x " << N << '\n';
    std::cout << "Threads     : " << threads << "\n\n";

    std::cout << "Generating matrices...\n";

    Matrix A = generateMatrix(N, 42);
    Matrix B = generateMatrix(N, 123);

    ProgressState progress(threads);
    progress.initializeRows(N);

    // Serialize the (unchanging) input matrices exactly once. Rebuilding
    // this text on every row callback was the main thing making the
    // program slow at larger N — it turned an O(n) callback into an
    // O(n^2) one, called up to n times (O(n^3) total, just for I/O).
    const std::string matrixAJson = serializeMatrix("matrixA", A, N);
    const std::string matrixBJson = serializeMatrix("matrixB", B, N);

    // Start with an empty result in the visualization.
    Matrix emptyResult(static_cast<size_t>(N) * N, 0.0);
    writeJson(matrixAJson, matrixBJson, &emptyResult, N, threads, progress, 0.0, "RUNNING");

    std::vector<int> threadForRow;

    std::cout << "Running parallel multiplication...\n";

    // Throttle how often the live snapshot is written to disk. Formatting
    // an n x n matrix to text is itself O(n^2), so writing it on every one
    // of the n completed rows costs O(n^3) overall — for n=500 that write
    // work alone dominates the whole run (tens of seconds), even though
    // the actual multiplication finishes in milliseconds. A wall-clock
    // throttle doesn't fix this: each write already takes about as long
    // as a short time window, so almost nothing gets skipped. Instead,
    // cap the number of writes to a fixed budget independent of n (about
    // 60 snapshots across the whole run), which bounds the visualization
    // overhead to O(60 * n^2) instead of O(n^3). The final snapshot after
    // the loop always captures the finished state regardless.
    std::atomic<int> rowsWritten{0};
    const int writeEvery = std::max(1, (N + 59) / 60); // ceil(N / 60)

    auto start = std::chrono::high_resolution_clock::now();

    Matrix parallelResult =
        multiplyParallel(
            A,
            B,
            N,
            threads,
            threadForRow,
            &progress,
            0, // no artificial per-row delay: run at full speed
            [&](const Matrix& currentResult)
            {
                // Called by workers after a row is complete.
                int count = rowsWritten.fetch_add(1) + 1;
                if (count % writeEvery != 0 && count != N)
                    return;

                writeJson(
                    matrixAJson,
                    matrixBJson,
                    &currentResult,
                    N,
                    threads,
                    progress,
                    0.0,
                    "RUNNING"
                );
            }
        );

    auto end = std::chrono::high_resolution_clock::now();

    double elapsed =
        std::chrono::duration<double, std::milli>(
            end - start
        ).count();

    std::cout << "\nProgress state:\n";

    for (int thread = 0; thread < threads; ++thread)
    {
        std::cout
            << "Thread " << thread
            << " completed "
            << progress.completedRows[thread].load()
            << " rows\n";
    }

    std::cout
        << "Finished : "
        << (progress.finished.load() ? "YES" : "NO")
        << '\n';

    Matrix referenceResult = multiplyReference(A, B, N);

    bool passed = verifyResult(referenceResult, parallelResult);

    std::cout
        << "Result verification : "
        << (passed ? "PASSED" : "FAILED")
        << '\n';

    // Final visualization snapshot.
    writeJson(
        matrixAJson,
        matrixBJson,
        &parallelResult,
        N,
        threads,
        progress,
        elapsed,
        passed ? "PASSED" : "FAILED"
    );

    if (!passed)
        return 1;

    std::cout << "\nThread distribution:\n";

    for (int thread = 0; thread < threads; ++thread)
    {
        int startRow = -1;
        int endRow = -1;

        for (int row = 0; row < N; ++row)
        {
            if (threadForRow[row] == thread)
            {
                if (startRow == -1)
                    startRow = row;

                endRow = row;
            }
        }

        std::cout
            << "Thread " << thread
            << " -> Rows "
            << startRow << " - " << endRow
            << '\n';
    }

    std::cout << "\nRunning benchmark...\n";

    auto results =
        runBenchmark(
            A,
            B,
            N,
            benchmarkRuns
        );

    printBenchmarkResults(results);

    std::cout << "\nLive visualization data: matrix_data.json\n";
    std::cout << "Open visualization/index.html through the local server.\n";

    return 0;
}
