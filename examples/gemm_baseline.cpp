#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

void gemm_ijk(const std::vector<float>& A,
              const std::vector<float>& B,
              std::vector<float>& C,
              int M,
              int N,
              int K) {
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            float sum = 0.0f;
            for (int k = 0; k < K; ++k) {
                sum += A[i * K + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }
}

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

double benchmark_ms(void (*fn)(const std::vector<float>&,
                                const std::vector<float>&,
                                std::vector<float>&,
                                int,
                                int,
                                int),
                    const std::vector<float>& A,
                    const std::vector<float>& B,
                    std::vector<float>& C,
                    int M,
                    int N,
                    int K,
                    int repeat) {
    double best_ms = 1e100;
    for (int r = 0; r < repeat; ++r) {
        auto start = std::chrono::high_resolution_clock::now();
        fn(A, B, C, M, N, K);
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

    if (argc >= 4) {
        M = std::atoi(argv[1]);
        N = std::atoi(argv[2]);
        K = std::atoi(argv[3]);
    }
    if (argc >= 5) {
        repeat = std::atoi(argv[4]);
    }

    std::vector<float> A(M * K);
    std::vector<float> B(K * N);
    std::vector<float> C(M * N);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    for (auto& x : A) x = dist(rng);
    for (auto& x : B) x = dist(rng);

    double ms_ijk = benchmark_ms(gemm_ijk, A, B, C, M, N, K, repeat);
    double gflops_ijk = gflops(M, N, K, ms_ijk);

    double ms_ikj = benchmark_ms(gemm_ikj, A, B, C, M, N, K, repeat);
    double gflops_ikj = gflops(M, N, K, ms_ikj);

    std::cout << "GEMM baseline benchmark\n";
    std::cout << "M=" << M << ", N=" << N << ", K=" << K << ", repeat=" << repeat << "\n\n";

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "ijk: " << ms_ijk << " ms, " << gflops_ijk << " GFLOPS\n";
    std::cout << "ikj: " << ms_ikj << " ms, " << gflops_ikj << " GFLOPS\n";

    return 0;
}
