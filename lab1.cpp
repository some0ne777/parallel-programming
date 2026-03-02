#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>

using namespace std;
using namespace chrono;

int main(int argc, char* argv[]) {
    if (argc < 5 || argc > 6) {
        cout << "Usage: matrix_mult.exe <size> <file_a> <file_b> <output> [threads]" << endl;
        return 1;
    }

    int N = atoi(argv[1]);
    string fileA = argv[2];
    string fileB = argv[3];
    string fileOut = argv[4];
    int threads = (argc == 6) ? atoi(argv[5]) : 1;

    vector<vector<double>> A(N, vector<double>(N));
    vector<vector<double>> B(N, vector<double>(N));
    vector<vector<double>> C(N, vector<double>(N, 0.0));

    ifstream fa(fileA);
    if (!fa.is_open()) {
        cerr << "Error: Cannot open file " << fileA << endl;
        return 1;
    }
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            fa >> A[i][j];
    fa.close();

    ifstream fb(fileB);
    if (!fb.is_open()) {
        cerr << "Error: Cannot open file " << fileB << endl;
        return 1;
    }
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            fb >> B[i][j];
    fb.close();

    long long mem = 3LL * N * N * sizeof(double);
    cout << "Memory: " << mem << " bytes (" << mem / (1024.0 * 1024.0) << " MB)" << endl;

    long long ops = 2LL * N * N * N;
    cout << "Operations: " << ops << " FLOP" << endl;

    auto start = high_resolution_clock::now();

    if (threads <= 1) {
        for (int i = 0; i < N; i++)
            for (int k = 0; k < N; k++)
                for (int j = 0; j < N; j++)
                    C[i][j] += A[i][k] * B[k][j];
    }
    else {
        vector<thread> thr;
        int block = N / threads;

        auto worker = [&](int start, int end) {
            for (int i = start; i < end; i++)
                for (int k = 0; k < N; k++)
                    for (int j = 0; j < N; j++)
                        C[i][j] += A[i][k] * B[k][j];
            };

        for (int t = 0; t < threads; t++) {
            int s = t * block;
            int e = (t == threads - 1) ? N : (t + 1) * block;
            thr.emplace_back(worker, s, e);
        }

        for (auto& t : thr) t.join();
    }

    auto end = high_resolution_clock::now();
    double time = duration<double>(end - start).count();

    cout << "Time: " << time << " seconds" << endl;
    cout << "Performance: " << (ops / time) / 1e9 << " GFLOPS" << endl;

    ofstream fout(fileOut);
    if (!fout.is_open()) {
        cerr << "Error: Cannot create output file " << fileOut << endl;
        return 1;
    }
    fout << fixed << setprecision(6);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++)
            fout << C[i][j] << " ";
        fout << endl;
    }
    fout.close();

    return 0;
}