#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <mpi.h>

using namespace std;
using namespace chrono;

vector<vector<double>> readMatrix(const string& f, int n) {
    ifstream file(f);
    if (!file.is_open()) {
        cerr << "Ошибка: не удалось открыть файл " << f << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    vector<vector<double>> m(n, vector<double>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            file >> m[i][j];
    return m;
}

void writeMatrix(const string& f, const vector<vector<double>>& m) {
    ofstream file(f);
    if (!file.is_open()) {
        cerr << "Ошибка: не удалось создать файл " << f << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int n = m.size();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file << m[i][j];
            if (j < n - 1) file << " ";
        }
        file << "\n";
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int N = (argc > 2) ? atoi(argv[2]) : 400;
    string path = ".";
    string fileA = path + "/1_" + to_string(N) + ".txt";
    string fileB = path + "/2_" + to_string(N) + ".txt";
    string fileC = path + "/result.txt";
    string statsFile = path + "/stats.txt";

    int error_flag = 0;
    if (rank == 0) {
        ifstream testA(fileA), testB(fileB);
        if (!testA.is_open() || !testB.is_open()) {
            cerr << "Ошибка: файл не найден" << endl;
            error_flag = 1;
        }
    }
    MPI_Bcast(&error_flag, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (error_flag) {
        MPI_Finalize();
        return 1;
    }

    long long readTime = 0, multTime = 0, writeTime = 0;

    vector<vector<double>> A, B;
    if (rank == 0) {
        auto read_start = high_resolution_clock::now();
        A = readMatrix(fileA, N);
        B = readMatrix(fileB, N);
        readTime = duration_cast<milliseconds>(
            high_resolution_clock::now() - read_start).count();
    }

    vector<double> B_flat(N * N);
    if (rank == 0)
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                B_flat[i * N + j] = B[i][j];

    MPI_Bcast(B_flat.data(), N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    int rows_per_proc = N / size;
    int remainder = N % size;

    vector<int> sendcounts(size), displs(size);
    for (int p = 0; p < size; p++) {
        int p_rows = (p < remainder) ? rows_per_proc + 1 : rows_per_proc;
        sendcounts[p] = p_rows * N;
        displs[p] = (p < remainder)
            ? p * (rows_per_proc + 1) * N
            : (p * rows_per_proc + remainder) * N;
    }

    int local_rows = (rank < remainder) ? rows_per_proc + 1 : rows_per_proc;

    vector<double> A_flat;
    if (rank == 0) {
        A_flat.resize(N * N);
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                A_flat[i * N + j] = A[i][j];
    }

    vector<double> local_A_flat(local_rows * N);
    MPI_Scatterv(A_flat.data(), sendcounts.data(), displs.data(), MPI_DOUBLE,
        local_A_flat.data(), local_rows * N, MPI_DOUBLE,
        0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    auto local_start = high_resolution_clock::now();

    vector<double> local_C_flat(local_rows * N, 0.0);

    for (int i = 0; i < local_rows; i++) {
        for (int k = 0; k < N; k++) {
            double aik = local_A_flat[i * N + k];
            if (aik == 0.0) continue;
            for (int j = 0; j < N; j++) {
                local_C_flat[i * N + j] += aik * B_flat[k * N + j];
            }
        }
    }

    long long local_mult_time = duration_cast<milliseconds>(
        high_resolution_clock::now() - local_start).count();

    MPI_Reduce(&local_mult_time, &multTime, 1, MPI_LONG_LONG, MPI_MAX, 0, MPI_COMM_WORLD);

    vector<double> C_flat;
    if (rank == 0)
        C_flat.resize(N * N);

    MPI_Gatherv(
        local_C_flat.data(), local_rows * N, MPI_DOUBLE,
        rank == 0 ? C_flat.data() : nullptr,
        rank == 0 ? sendcounts.data() : nullptr,
        rank == 0 ? displs.data() : nullptr,
        MPI_DOUBLE, 0, MPI_COMM_WORLD
    );

    if (rank == 0) {
        vector<vector<double>> C(N, vector<double>(N));
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                C[i][j] = C_flat[i * N + j];

        auto write_start = high_resolution_clock::now();
        writeMatrix(fileC, C);
        writeTime = duration_cast<milliseconds>(
            high_resolution_clock::now() - write_start).count();

        cout << "========================================\n";
        cout << "Размер: " << N << "x" << N << "\n";
        cout << "Процессов: " << size << "\n";
        cout << "Чтение: " << readTime << " мс\n";
        cout << "Умножение: " << multTime << " мс\n";
        cout << "Запись: " << writeTime << " мс\n";
        cout << "Общее: " << readTime + multTime + writeTime << " мс\n";
        cout << "========================================\n";

        ofstream stats(statsFile, ios::app);
        if (stats.is_open())
            stats << left
            << setw(8) << N
            << setw(10) << size
            << setw(12) << readTime
            << setw(14) << multTime
            << setw(12) << writeTime
            << setw(10) << readTime + multTime + writeTime << "\n";
    }

    MPI_Finalize();
    return 0;
}