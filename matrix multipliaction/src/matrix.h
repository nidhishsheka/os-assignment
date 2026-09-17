#ifndef MATRIX_H
#define MATRIX_H

#include <atomic>
#include <functional>
#include <vector>

using Matrix = std::vector<double>;

struct ProgressState
{
    std::vector<std::atomic<int>> completedRows;
    std::vector<std::atomic<int>> activeRows;
    std::vector<std::atomic<bool>> completedRow;
    std::atomic<bool> finished;

    explicit ProgressState(int threadCount)
        : completedRows(threadCount),
          activeRows(threadCount),
          completedRow(),
          finished(false)
    {
        for (auto& value : completedRows)
            value.store(0);

        for (auto& value : activeRows)
            value.store(-1);
    }

    void initializeRows(int n)
    {
        // std::atomic<bool> is neither copyable nor movable, so the vector
        // can't be resized/assigned in place. Move-assigning a freshly
        // constructed vector only transfers the underlying buffer (no
        // per-element move required), which is valid even for atomic types.
        completedRow = std::vector<std::atomic<bool>>(n);

        for (auto& value : completedRow)
            value.store(false);
    }
};

using ProgressCallback = std::function<void(const Matrix&)>;

Matrix generateMatrix(int n, unsigned int seed);

Matrix multiplyReference(
    const Matrix& A,
    const Matrix& B,
    int n
);

Matrix multiplyParallel(
    const Matrix& A,
    const Matrix& B,
    int n,
    int numThreads,
    std::vector<int>& threadForRow,
    ProgressState* progress = nullptr,
    int visualizationDelayMs = 0,
    ProgressCallback callback = nullptr
);

bool verifyResult(
    const Matrix& reference,
    const Matrix& parallel
);

double& matrixAt(
    Matrix& matrix,
    int row,
    int col,
    int n
);

const double& matrixAt(
    const Matrix& matrix,
    int row,
    int col,
    int n
);

#endif
