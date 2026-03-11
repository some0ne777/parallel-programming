#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
using namespace std;
using namespace chrono;

void setup_io()
{
#ifdef _MSC_VER
    system("chcp 65001 > nul");
#endif

    std::ios_base::sync_with_stdio(false);
}

vector<vector<double>> readMatrix(string f, int n) {
    ifstream file(f);
    if (!file.is_open()) { cerr << "Ошибка открытия " << f << endl; exit(1); }
    vector<vector<double>> m(n, vector<double>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            file >> m[i][j];
    return m;
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

vector<vector<double>> multiply(vector<vector<double>>& A, vector<vector<double>>& B) {
    int n = A.size();
    vector<vector<double>> C(n, vector<double>(n, 0));
    for (int i = 0; i < n; i++)
        for (int k = 0; k < n; k++)
            for (int j = 0; j < n; j++)
                C[i][j] += A[i][k] * B[k][j];
    return C;
}

void saveStats(string f, int n, long long rt, long long mt, long long wt, long long tt) {
    ofstream file(f, ios::app);
    file.seekp(0, ios::end);
    if (file.tellp() == 0) {
        file << left << setw(8) << "Размер"
            << setw(12) << "Элементов"
            << setw(12) << "Операций"
            << setw(12) << "Чтение(мс)"
            << setw(14) << "Умножение(мс)"
            << setw(12) << "Запись(мс)"
            << setw(10) << "Всего(мс)" << "\n";
        file << string(80, '-') << "\n";
    }

    file << left << setw(8) << n
        << setw(12) << n * n
        << setw(12) << n * n * n
        << setw(12) << rt
        << setw(14) << mt
        << setw(12) << wt
        << setw(10) << tt << "\n";
}

int main() {
    setup_io();
    string path = "C:\\Users\\jnnsi\\Desktop\\lab poralel prog\\";
    string fileA = path + "1_14.txt";
    string fileB = path + "2_14.txt";
    string fileC = path + "result.txt";
    int N = 14;

    cout << "Умножение матриц " << N << "x" << N << "\n\n";

    auto start = high_resolution_clock::now();
    auto A = readMatrix(fileA, N);
    auto B = readMatrix(fileB, N);
    auto readTime = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
    cout << "Чтение: " << readTime << " мс\n";

    start = high_resolution_clock::now();
    auto C = multiply(A, B);
    auto multTime = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
    cout << "Умножение: " << multTime << " мс\n";

    start = high_resolution_clock::now();
    writeMatrix(fileC, C);
    auto writeTime = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
    cout << "Запись: " << writeTime << " мс\n";

    auto totalTime = readTime + multTime + writeTime;
    cout << "\nВсего: " << totalTime << " мс\n";

    saveStats(path + "stats.txt", N, readTime, multTime, writeTime, totalTime);
    cout << "\nСтатистика в " << path + "stats.txt\n";

    return 0;
}