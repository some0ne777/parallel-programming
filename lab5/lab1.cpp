#include <mpi.h> 
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <sstream> 
#include <cstring> 
#include <algorithm> 

using namespace std;

string to_string_custom(int n) {
    ostringstream oss;
    oss << n;
    return oss.str();
}

vector<vector<double> > readMatrix(const string& f, int n) {
    ifstream file(f.c_str());
    vector<vector<double> > m(n, vector<double>(n));
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file >> m[i][j];
        }
    }
    
    file.close();
    return m;
}

void writeMatrix(const string& f, const vector<vector<double> >& m) {
    ofstream file(f.c_str());
    int n = m.size();
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file << m[i][j];
            if (j < n - 1) file << " ";
        }
        file << endl;
    }
    
    file.close();
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    string path = ".";
    int N = 400;
    
    if (argc > 1) {
        path = argv[1];
    }
    if (argc > 2) {
        N = atoi(argv[2]);
    }

    string fileA = path + "/1_" + to_string_custom(N) + ".txt";
    string fileB = path + "/2_" + to_string_custom(N) + ".txt";
    string fileC = path + "/3_" + to_string_custom(N) + ".txt";
    string statsFile = path + "/stats.txt";
    
    if (rank == 0) {
        ifstream testA(fileA.c_str());
        ifstream testB(fileB.c_str());
        
        if (!testA.good()) {
            cerr << "Ошибка: файл " << fileA << " не найден!" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        if (!testB.good()) {
            cerr << "Ошибка: файл " << fileB << " не найден!" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        testA.close();
        testB.close();
    }
    
    long long readTime = 0;
    if (rank == 0) {
        double read_start = MPI_Wtime();
        
        vector<vector<double> > A = readMatrix(fileA, N);
        vector<vector<double> > B = readMatrix(fileB, N);
        
        readTime = (long long)((MPI_Wtime() - read_start) * 1000);
        
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    int rows_per_proc = N / size;
    int remainder = N % size;
    
    int local_rows = rows_per_proc + (rank < remainder ? 1 : 0);
    
    vector<int> displs(size);
    vector<int> send_counts(size);
    
    int offset = 0;
    for (int i = 0; i < size; i++) {
        send_counts[i] = (rows_per_proc + (i < remainder ? 1 : 0)) * N;
        displs[i] = offset;
        offset += send_counts[i];
    }
    
    vector<double> local_A(local_rows * N);
    vector<double> local_B(N * N);
    vector<double> local_C(local_rows * N, 0.0);
    
    vector<double> full_A;
    if (rank == 0) {
        full_A.resize(N * N);
    }
    
    vector<double> full_B;
    if (rank == 0) {
        full_B.resize(N * N);
    }
    
    if (rank == 0) {
        vector<vector<double> > A_mat = readMatrix(fileA, N);
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                full_A[i * N + j] = A_mat[i][j];
            }
        }
        
        vector<vector<double> > B_mat = readMatrix(fileB, N);
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                full_B[i * N + j] = B_mat[i][j];
            }
        }
    }
    
    double mult_start = MPI_Wtime();
    
    if (rank == 0) {
        MPI_Bcast(&full_B[0], N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    } else {
        full_B.resize(N * N);
        MPI_Bcast(&full_B[0], N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    local_B = full_B;
    
    if (rank == 0) {
        MPI_Scatterv(&full_A[0], &send_counts[0], &displs[0], MPI_DOUBLE,
                    &local_A[0], local_rows * N, MPI_DOUBLE,
                    0, MPI_COMM_WORLD);
    } else {
        MPI_Scatterv(NULL, &send_counts[0], &displs[0], MPI_DOUBLE,
                    &local_A[0], local_rows * N, MPI_DOUBLE,
                    0, MPI_COMM_WORLD);
    }
    
    for (int i = 0; i < local_rows; i++) {
        for (int k = 0; k < N; k++) {
            double a_ik = local_A[i * N + k];
            if (a_ik != 0.0) {
                for (int j = 0; j < N; j++) {
                    local_C[i * N + j] += a_ik * local_B[k * N + j];
                }
            }
        }
    }
    
    double mult_end = MPI_Wtime();
    long long local_mult_time = (long long)((mult_end - mult_start) * 1000);
    
    vector<double> full_C;
    vector<int> recv_counts(size);
    vector<int> recv_displs(size);
    
    offset = 0;
    for (int i = 0; i < size; i++) {
        recv_counts[i] = (rows_per_proc + (i < remainder ? 1 : 0)) * N;
        recv_displs[i] = offset;
        offset += recv_counts[i];
    }
    
    if (rank == 0) {
        full_C.resize(N * N);
        MPI_Gatherv(&local_C[0], local_rows * N, MPI_DOUBLE,
                   &full_C[0], &recv_counts[0], &recv_displs[0], MPI_DOUBLE,
                   0, MPI_COMM_WORLD);
    } else {
        MPI_Gatherv(&local_C[0], local_rows * N, MPI_DOUBLE,
                   NULL, &recv_counts[0], &recv_displs[0], MPI_DOUBLE,
                   0, MPI_COMM_WORLD);
    }
    
    if (rank == 0) {
        vector<vector<double> > C(N, vector<double>(N));
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                C[i][j] = full_C[i * N + j];
            }
        }
        
        writeMatrix(fileC, C);
        
        ofstream stats(statsFile.c_str(), ios::app);
        stats << N << " " << size << " " << readTime << " " << local_mult_time << endl;
        stats.close();
        
        cout << "Результат записан в " << fileC << endl;
        cout << "Время чтения: " << readTime << " мс" << endl;
        cout << "Время умножения: " << local_mult_time << " мс" << endl;
    }
    
    MPI_Finalize();
    return 0;
}