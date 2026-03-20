#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

void gemm_ikj(const std::vector<float>& A,
              const std::vector<float>& B,
              std::vector<float>& C,
              int M,
              int N,
              int K) {
    std::fill(C.begin(), C.end(), 0.0f);
    for (int i = 0; i < M; ++i) {
        for (int k = 0; k < K; ++k) {
            float a = A[i * K + k];
            for (int j = 0; j < N; ++j) {
                C[i * N + j] += a * B[k * N + j];
            }
        }
    }
}

void gemm_blocked(const std::vector<float>& A,
                  const std::vector<float>& B,
                  std::vector<float>& C,
                  int M,
                  int N,
                  int K,
                  int BM,
                  int BN,
                  int BK) {
    std::fill(C.begin(), C.end(), 0.0f);

    for (int ii = 0; ii < M; ii += BM) {
        for (int kk = 0; kk < K; kk += BK) {
            for (int jj = 0; jj < N; jj += BN) {
                int i_end = std::min(ii + BM, M);
                int k_end = std::min(kk + BK, K);
                int j_end = std::min(jj + BN, N);

                for (int i = ii; i < i_end; ++i) {
                    for (int k = kk; k < k_end; ++k) {
                        float a = A[i * K + k];
                        for (int j = jj; j < j_end; ++j) {
                            C[i * N + j] += a * B[k * N + j];
                        }
                    }
                }
            }
        }
    }
}

double benchmark_ikj(const std::vector<float>& A,
                     const std::vector<float>& B,
                     std::vector<float>& C,
                     int M,
                     int N,
                     int K,
                     int repeat) {
    double best_ms = 1e100;
    for (int r = 0; r < repeat; ++r) {
        auto start = std::chrono::high_resolution_clock::now();
        gemm_ikj(A, B, C, M, N, K);
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        best_ms = std::min(best_ms, ms);
    }
    return best_ms;
}

double benchmark_blocked(const std::vector<float>& A,
                         const std::vector<float>& B,
                         std::vector<float>& C,
                         int M,
                         int N,
                         int K,
                         int BM,
                         int BN,
                         int BK,
                         int repeat) {
    double best_ms = 1e100;
    for (int r = 0; r < repeat; ++r) {
        auto start = std::chrono::high_resolution_clock::now();
        gemm_blocked(A, B, C, M, N, K, BM, BN, BK);
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        best_ms = std::min(best_ms, ms);
    }
    return best_ms;
}

double gflops(int M, int N, int K, double ms) {
    double flops = 2.0 * M * N * K;
    return flops / 1e9 / (ms / 1e3);
}

int main(int argc, char** argv) {
    int M = 512;
    int N = 512;
    int K = 512;
    int repeat = 3;
    int BM = 64;
    int BN = 64;
    int BK = 64;

    if (argc >= 4) {
        M = std::atoi(argv[1]);
        N = std::atoi(argv[2]);
        K = std::atoi(argv[3]);
    }
    if (argc >= 5) {
        repeat = std::atoi(argv[4]);
    }
    if (argc >= 8) {
        BM = std::atoi(argv[5]);
        BN = std::atoi(argv[6]);
        BK = std::atoi(argv[7]);
    }

    std::vector<float> A(M * K);
    std::vector<float> B(K * N);
    std::vector<float> C(M * N);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    for (auto& x : A) x = dist(rng);
    for (auto& x : B) x = dist(rng);

    double ms_ikj = benchmark_ikj(A, B, C, M, N, K, repeat);
    double gflops_ikj = gflops(M, N, K, ms_ikj);

    double ms_blocked = benchmark_blocked(A, B, C, M, N, K, BM, BN, BK, repeat);
    double gflops_blocked = gflops(M, N, K, ms_blocked);

    std::cout << "Blocked GEMM benchmark\n";
    std::cout << "M=" << M << ", N=" << N << ", K=" << K
              << ", repeat=" << repeat
              << ", BM=" << BM << ", BN=" << BN << ", BK=" << BK << "\n\n";

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "ikj baseline: " << ms_ikj << " ms, " << gflops_ikj << " GFLOPS\n";
    std::cout << "blocked     : " << ms_blocked << " ms, " << gflops_blocked << " GFLOPS\n";

    return 0;
}
