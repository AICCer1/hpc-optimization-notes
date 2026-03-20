import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent
CPP = ROOT / "gemm_baseline.cpp"
BIN = ROOT / "gemm_baseline"


def build():
    cmd = [
        "g++",
        "-O3",
        "-march=native",
        "-std=c++17",
        str(CPP),
        "-o",
        str(BIN),
    ]
    print("Build:", " ".join(cmd))
    subprocess.run(cmd, check=True)


def run_case(m: int, n: int, k: int, repeat: int = 3):
    cmd = [str(BIN), str(m), str(n), str(k), str(repeat)]
    print("Run:", " ".join(cmd))
    subprocess.run(cmd, check=True)


if __name__ == "__main__":
    build()
    cases = [
        (256, 256, 256),
        (512, 512, 512),
        (1024, 1024, 1024),
    ]
    if len(sys.argv) == 4:
        cases = [(int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3]))]
    for m, n, k in cases:
        print("=" * 60)
        run_case(m, n, k)
