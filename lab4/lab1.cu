%%writefile lab1.cu
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <string>
#include <cuda_runtime.h>

using namespace std;
using namespace chrono;

#define CUDA_CHECK(call)                                                        \
    do {                                                                        \
        cudaError_t err = (call);                                               \
        if (err != cudaSuccess) {                                               \
            cerr << "CUDA error at " << __FILE__ << ":" << __LINE__            \
                 << " — " << cudaGetErrorString(err) << "\n";                  \
            exit(EXIT_FAILURE);                                                 \
        }                                                                       \
    } while (0)

__global__ void matMulNaive(const double* __restrict__ A,
                             const double* __restrict__ B,
                             double*       __restrict__ C,
                             int N)
{
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    if (row >= N || col >= N) return;

    double sum = 0.0;
    for (int k = 0; k < N; ++k)
        sum += A[row * N + k] * B[k * N + col];
    C[row * N + col] = sum;
}

template <int TILE>
__global__ void matMulTiled(const double* __restrict__ A,
                             const double* __restrict__ B,
                             double*       __restrict__ C,
                             int N)
{
    __shared__ double sA[TILE][TILE];
    __shared__ double sB[TILE][TILE];

    int row = blockIdx.y * TILE + threadIdx.y;
    int col = blockIdx.x * TILE + threadIdx.x;
    double sum = 0.0;

    for (int t = 0; t < (N + TILE - 1) / TILE; ++t) {
        int aCol = t * TILE + threadIdx.x;
        int bRow = t * TILE + threadIdx.y;

        sA[threadIdx.y][threadIdx.x] = (row < N && aCol < N) ? A[row * N + aCol] : 0.0;
        sB[threadIdx.y][threadIdx.x] = (bRow < N && col < N) ? B[bRow * N + col] : 0.0;
        __syncthreads();

        for (int k = 0; k < TILE; ++k)
            sum += sA[threadIdx.y][k] * sB[k][threadIdx.x];
        __syncthreads();
    }

    if (row < N && col < N)
        C[row * N + col] = sum;
}

static vector<double> readMatrix(const string& path, int N)
{
    ifstream f(path);
    if (!f.is_open()) {
        cerr << "Cannot open " << path << "\n";
        exit(1);
    }
    vector<double> m(N * N);
    for (int i = 0; i < N * N; ++i) f >> m[i];
    return m;
}

static void writeMatrix(const string& path, const vector<double>& C, int N)
{
    ofstream f(path);
    if (!f.is_open()) {
        cerr << "Cannot create " << path << "\n";
        exit(1);
    }
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            f << C[i * N + j];
            if (j < N - 1) f << " ";
        }
        f << "\n";
    }
}

static void printDeviceInfo()
{
    int dev = 0;
    cudaDeviceProp p{};
    CUDA_CHECK(cudaGetDeviceProperties(&p, dev));
    cout << "GPU: " << p.name
         << " | SMs: " << p.multiProcessorCount
         << " | GMEM: " << p.totalGlobalMem / (1 << 20) << " MB\n";
}

static double runExperiment(const vector<double>& hA,
                             const vector<double>& hB,
                             vector<double>&       hC,
                             int N, int blockSize, int kernelType,
                             bool saveResult,
                             const string& resultPath)
{
    size_t bytes = (size_t)N * N * sizeof(double);

    double *dA, *dB, *dC;
    CUDA_CHECK(cudaMalloc(&dA, bytes));
    CUDA_CHECK(cudaMalloc(&dB, bytes));
    CUDA_CHECK(cudaMalloc(&dC, bytes));

    CUDA_CHECK(cudaMemcpy(dA, hA.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(dB, hB.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemset(dC, 0, bytes));

    dim3 threads(blockSize, blockSize);
    dim3 blocks((N + blockSize - 1) / blockSize,
                (N + blockSize - 1) / blockSize);

    if (kernelType == 0)
        matMulNaive<<<blocks, threads>>>(dA, dB, dC, N);
    else {
        if (blockSize == 4)  matMulTiled< 4><<<blocks, threads>>>(dA, dB, dC, N);
        else if (blockSize == 8)  matMulTiled< 8><<<blocks, threads>>>(dA, dB, dC, N);
        else if (blockSize == 16) matMulTiled<16><<<blocks, threads>>>(dA, dB, dC, N);
        else                 matMulTiled<32><<<blocks, threads>>>(dA, dB, dC, N);
    }
    CUDA_CHECK(cudaDeviceSynchronize());

    cudaEvent_t start, stop;
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));
    CUDA_CHECK(cudaEventRecord(start));

    if (kernelType == 0)
        matMulNaive<<<blocks, threads>>>(dA, dB, dC, N);
    else {
        if (blockSize == 4)  matMulTiled< 4><<<blocks, threads>>>(dA, dB, dC, N);
        else if (blockSize == 8)  matMulTiled< 8><<<blocks, threads>>>(dA, dB, dC, N);
        else if (blockSize == 16) matMulTiled<16><<<blocks, threads>>>(dA, dB, dC, N);
        else                 matMulTiled<32><<<blocks, threads>>>(dA, dB, dC, N);
    }

    CUDA_CHECK(cudaEventRecord(stop));
    CUDA_CHECK(cudaDeviceSynchronize());

    float ms = 0.f;
    CUDA_CHECK(cudaEventElapsedTime(&ms, start, stop));
    CUDA_CHECK(cudaEventDestroy(start));
    CUDA_CHECK(cudaEventDestroy(stop));

    hC.resize(N * N);
    CUDA_CHECK(cudaMemcpy(hC.data(), dC, bytes, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(dA));
    CUDA_CHECK(cudaFree(dB));
    CUDA_CHECK(cudaFree(dC));

    if (saveResult)
        writeMatrix(resultPath, hC, N);

    return static_cast<double>(ms);
}

int main(int argc, char* argv[])
{
    int N          = (argc > 1) ? atoi(argv[1]) : 256;
    int blockSize  = (argc > 2) ? atoi(argv[2]) : 16;
    int kernelType = (argc > 3) ? atoi(argv[3]) : 1;
    bool saveRes   = (argc > 4) ? (atoi(argv[4]) != 0) : false;

    if (blockSize != 4 && blockSize != 8 && blockSize != 16 && blockSize != 32) blockSize = 16;

    printDeviceInfo();

    string fileA    = "1_" + to_string(N) + ".txt";
    string fileB    = "2_" + to_string(N) + ".txt";
    string fileRes  = "cuda_result.txt";
    string statsFile = "cuda_stats.txt";

    auto t0 = high_resolution_clock::now();
    auto hA = readMatrix(fileA, N);
    auto hB = readMatrix(fileB, N);
    long long readTime = duration_cast<milliseconds>(
        high_resolution_clock::now() - t0).count();

    vector<double> hC;
    double multTime = runExperiment(hA, hB, hC, N, blockSize, kernelType,
                                    saveRes, fileRes);

    cout << "========================================\n";
    cout << "Размер:     " << N << "x" << N << "\n";
    cout << "Блок:       " << blockSize << "x" << blockSize << "\n";
    cout << "Чтение:     " << readTime      << " мс\n";
    cout << "Умножение:  " << (long long)multTime << " мс\n";
    cout << "Всего:      " << readTime + (long long)multTime << " мс\n";
    cout << "========================================\n";

    ofstream stats(statsFile, ios::app);
    if (stats.is_open())
        stats << left
              << setw(8)  << N
              << setw(10) << blockSize
              << setw(14) << readTime
              << setw(16) << (long long)multTime
              << setw(12) << readTime + (long long)multTime << "\n";

    return 0;
}