#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <omp.h>
using namespace std;
using namespace chrono;

void setup_io()
{
#ifdef _MSC_VER
    system("chcp 65001 > nul");
#endif
    std::ios_base::sync_with_stdio(false);
}

string to_string_int(int n) {
    stringstream ss;
    ss << n;
    return ss.str();
}

vector<vector<double>> readMatrix(string f, int n) {
    ifstream file(f);
    if (!file.is_open()) {
        cerr << "Ошибка открытия файла " << f << endl;
        exit(1);
    }

    vector<vector<double>> m(n, vector<double>(n));

    string first_line;
    getline(file, first_line);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file >> m[i][j];
        }
    }
    return m;
}

void writeMatrix(string f, vector<vector<double>>& m) {
    ofstream file(f);
    int n = m.size();

    ios_base::sync_with_stdio(false);
    file.tie(NULL);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file << m[i][j];
            if (j < n - 1) file << " ";
        }
        file << "\n";
    }
}

void writeMatrix(string f, vector<vector<double>>& m) {
    ofstream file(f);
    int n = m.size();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            file << m[i][j] << " ";
        file << "\n";
    }
}

vector<vector<double>> multiplyParallel(vector<vector<double>>& A,
    vector<vector<double>>& B,
    int num_threads) {
    int n = A.size();
    vector<vector<double>> C(n, vector<double>(n, 0.0));

    omp_set_num_threads(num_threads);

#pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            double aik = A[i][k];
            for (int j = 0; j < n; j++) {
                C[i][j] += aik * B[k][j];
            }
        }
    }

    return C;
}

void saveStats(string f, int n, int num_threads, long long rt,
    long long mt, long long wt, long long tt) {
    ofstream file(f, ios::app);
    file.seekp(0, ios::end);
    if (file.tellp() == 0) {
        file << left << setw(8) << "Размер"
            << setw(10) << "Потоков"
            << setw(12) << "Чтение(мс)"
            << setw(14) << "Умножение(мс)"
            << setw(12) << "Запись(мс)"
            << setw(10) << "Всего(мс)" << "\n";
        file << string(80, '-') << "\n";
    }

    file << left << setw(8) << n
        << setw(10) << num_threads
        << setw(12) << rt
        << setw(14) << mt
        << setw(12) << wt
        << setw(10) << tt << "\n";
}

int main(int argc, char* argv[]) {
    setup_io();

    cout << "========================================" << endl;
    cout << "Доступно потоков: " << omp_get_max_threads() << endl;
    cout << "Количество ядер: " << omp_get_num_procs() << endl;
    cout << "========================================" << endl;

    string current_path = ".";

    int N = 200;
    int num_threads = 1;

    if (argc > 1) num_threads = atoi(argv[1]);
    if (argc > 2) N = atoi(argv[2]);
    if (argc > 3) current_path = argv[3];

    string fileA = current_path + "/1_" + to_string_int(N) + ".txt";
    string fileB = current_path + "/2_" + to_string_int(N) + ".txt";
    string fileC = current_path + "/result.txt";
    string statsFile = current_path + "/stats.txt";

    cout << "\n========================================" << endl;
    cout << "Умножение матриц " << N << "x" << N << endl;
    cout << "Количество потоков: " << num_threads << endl;
    cout << "========================================\n" << endl;

    auto start = high_resolution_clock::now();
    auto A = readMatrix(fileA, N);
    auto B = readMatrix(fileB, N);
    auto readTime = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
    cout << "Чтение матриц: " << readTime << " мс" << endl;

    start = high_resolution_clock::now();
    auto C = multiplyParallel(A, B, num_threads);
    auto multTime = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
    cout << "Умножение: " << multTime << " мс" << endl;

    start = high_resolution_clock::now();
    writeMatrix(fileC, C);
    auto writeTime = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
    cout << "Запись результата: " << writeTime << " мс" << endl;

    auto totalTime = readTime + multTime + writeTime;
    cout << "\nОбщее время: " << totalTime << " мс" << endl;
    cout << "========================================\n" << endl;

    saveStats(statsFile, N, num_threads, readTime, multTime, writeTime, totalTime);

    return 0;
}