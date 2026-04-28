%%writefile verify_cuda.py
import numpy as np
import sys

def main():
    if len(sys.argv) != 5:
        print("Usage: python verify_cuda.py <size> <file_a> <file_b> <file_result>")
        return 1

    N        = int(sys.argv[1])
    file_a   = sys.argv[2]
    file_b   = sys.argv[3]
    file_res = sys.argv[4]

    print(f"Загрузка матриц размером {N}x{N}...")
    A     = np.loadtxt(file_a  ).reshape(N, N)
    B     = np.loadtxt(file_b  ).reshape(N, N)
    C_gpu = np.loadtxt(file_res).reshape(N, N)

    print("Вычисление numpy-эталона...")
    C_ref = A @ B

    max_diff = float(np.max(np.abs(C_gpu - C_ref)))
    rel_err  = max_diff / (float(np.max(np.abs(C_ref))) + 1e-15)

    print(f"Максимальная абсолютная разница : {max_diff:.3e}")
    print(f"Относительная погрешность       : {rel_err:.3e}")

    if max_diff < 1e-6:
        print("✓ ВЕРИФИКАЦИЯ ПРОЙДЕНА")
        return 0
    else:
        print("✗ ВЕРИФИКАЦИЯ НЕ ПРОЙДЕНА")
        return 1

if __name__ == "__main__":
    sys.exit(main())