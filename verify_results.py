import numpy as np
import sys

def main():
    if len(sys.argv) != 5:
        print("Usage: python verify.py <size> <file_a> <file_b> <file_result>")
        return 1
    
    N = int(sys.argv[1])
    file_a = sys.argv[2]
    file_b = sys.argv[3]
    file_res = sys.argv[4]
    
    A = np.loadtxt(file_a).reshape(N, N)
    B = np.loadtxt(file_b).reshape(N, N)
    C_cpp = np.loadtxt(file_res).reshape(N, N)
    
    C_numpy = np.dot(A, B)
    
    max_diff = np.max(np.abs(C_cpp - C_numpy))
    print(f"Max difference: {max_diff:.2e}")
    
    if max_diff < 1e-8:
        print("VERIFICATION PASSED")
        return 0
    else:
        print("VERIFICATION FAILED")
        return 1

if __name__ == "__main__":
    sys.exit(main())