#include "matrix.h"

#include <chrono>
#include <cmath>
#include <random>
#include <thread>

#include <omp.h>

double& matrixAt(
    Matrix& matrix,
    int row,
    int col,
    int n)
{
    return matrix[static_cast<size_t>(row) * n + col];
}

const double& matrixAt(
    const Matrix& matrix,
    int row,
    int col,
    int n)
{
    return matrix[static_cast<size_t>(row) * n + col];
}

Matrix generateMatrix(
    int n,
    unsigned int seed)
{
    Matrix matrix(static_cast<size_t>(n) * n);

    std::mt19937 generator(seed);
    std::uniform_real_distribution<double> distribution(1.0, 10.0);

    for (double& value : matrix)
        value = distribution(generator);

    return matrix;
}

Matrix multiplyReference(
    const Matrix& A,
    const Matrix& B,
    int n)
{
    Matrix C(static_cast<size_t>(n) * n, 0.0);

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            double sum = 0.0;

            for (int k = 0; k < n; ++k)
            {
                sum +=
                    matrixAt(A, i, k, n) *
                    matrixAt(B, k, j, n);
            }

            matrixAt(C, i, j, n) = sum;
        }
    }

    return C;
}

Matrix multiplyParallel(
    const Matrix& A,
    const Matrix& B,
    int n,
    int numThreads,
    std::vector<int>& threadForRow,
    ProgressState* progress,
    int visualizationDelayMs,
    ProgressCallback callback)
{
    Matrix C(static_cast<size_t>(n) * n, 0.0);

    threadForRow.assign(n, -1);

    if (progress != nullptr)
    {
        progress->initializeRows(n);
        progress->finished.store(false);

        for (auto& value : progress->completedRows)
            value.store(0);

        for (auto& value : progress->activeRows)
            value.store(-1);
    }

    omp_set_num_threads(numThreads);

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; ++i)
    {
        const int threadId = omp_get_thread_num();

        threadForRow[i] = threadId;

        if (progress != nullptr)
            progress->activeRows[threadId].store(i);

        // Cache A(i,k) and walk B contiguously.
        for (int k = 0; k < n; ++k)
        {
            const double Aik = matrixAt(A, i, k, n);

            for (int j = 0; j < n; ++j)
            {
                matrixAt(C, i, j, n) +=
                    Aik * matrixAt(B, k, j, n);
            }
        }

        if (progress != nullptr)
        {
            // All writes to this row happen before this atomic flag.
            progress->completedRow[i].store(true);

            progress->completedRows[threadId].fetch_add(1);

            if (visualizationDelayMs > 0)
            {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(visualizationDelayMs)
                );
            }

            progress->activeRows[threadId].store(-1);

            if (callback)
                callback(C);
        }
    }

    if (progress != nullptr)
        progress->finished.store(true);

    return C;
}

bool verifyResult(
    const Matrix& reference,
    const Matrix& parallel)
{
    if (reference.size() != parallel.size())
        return false;

    constexpr double tolerance = 1e-9;

    for (size_t i = 0; i < reference.size(); ++i)
    {
        if (std::abs(reference[i] - parallel[i]) > tolerance)
            return false;
    }

    return true;
}
