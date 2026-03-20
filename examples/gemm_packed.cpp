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

void pack_b_panel(const std::vector<float>& B,
                  std::vector<float>& Bp,
                  int K,
                  int N,
                  int kk,
                  int bk,
                  int jj,
                  int bn) {
    for (int k = 0; k < bk; ++k) {
        for (int j = 0; j < bn; ++j) {
            Bp[k * bn + j] = B[(kk + k) * N + (jj + j)];
        }
    }
}

void gemm_packed(const std::vector<float>& A,
                 const std::vector<float>& B,
                 std::vector<float>& C,
                 int M,
                 int N,
                 int K,
                 int BK,
                 int BN) {
    std::fill(C.begin(), C.end(), 0.0f);
    std::vector<float> Bp(BK * BN);

    for (int kk = 0; kk < K; kk += BK) {
        int bk = std::min(BK, K - kk);
        for (int jj = 0; jj < N; jj += BN) {
            int bn = std::min(BN, N - jj);
            pack_b_panel(B, Bp, K, N, kk, bk, jj, bn);

            for (int i = 0; i < M; ++i) {
                for (int k = 0; k < bk; ++k) {
                    float a = A[i * K + (kk + k)];
                    const float* bp = &Bp[k * bn];
                    float* cptr = &C[i * N + jj];
                    for (int j = 0; j < bn; ++j) {
                        cptr[j] += a * bp[j];
                    }
                }
            }
        }
    }
}

double benchmark_baseline(const std::vector<float>& A,
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

double benchmark_packed(const std::vector<float>& A,
                        const std::vector<float>& B,
                        std::vector<float>& C,
                        int M,
                        int N,
                        int K,
                        int BK,
                        int BN,
                        int repeat) {
    double best_ms = 1e100;
    for (int r = 0; r < repeat; ++r) {
        auto start = std::chrono::high_resolution_clock::now();
        gemm_packed(A, B, C, M, N, K, BK, BN);
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
    int BK = 64;
    int BN = 64;

    if (argc >= 4) {
        M = std::atoi(argv[1]);
        N = std::atoi(argv[2]);
        K = std::atoi(argv[3]);
    }
    if (argc >= 5) {
        repeat = std::atoi(argv[4]);
    }
    if (argc >= 7) {
        BK = std::atoi(argv[5]);
        BN = std::atoi(argv[6]);
    }

    std::vector<float> A(M * K);
    std::vector<float> B(K * N);
    std::vector<float> C(M * N);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    for (auto& x : A) x = dist(rng);
    for (auto& x : B) x = dist(rng);

    double ms_baseline = benchmark_baseline(A, B, C, M, N, K, repeat);
    double gflops_baseline = gflops(M, N, K, ms_baseline);

    double ms_packed = benchmark_packed(A, B, C, M, N, K, BK, BN, repeat);
    double gflops_packed = gflops(M, N, K, ms_packed);

    std::cout << "Packed GEMM benchmark\n";
    std::cout << "M=" << M << ", N=" << N << ", K=" << K
              << ", repeat=" << repeat
              << ", BK=" << BK << ", BN=" << BN << "\n\n";

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "baseline : " << ms_baseline << " ms, " << gflops_baseline << " GFLOPS\n";
    std::cout << "packed   : " << ms_packed << " ms, " << gflops_packed << " GFLOPS\n";

    return 0;
}
