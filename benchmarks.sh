#!/bin/bash
#SBATCH -J lr_benchmarks
#SBATCH -o benchmarks_%j.out
#SBATCH -e benchmarks_%j.err
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=02:00:00
#SBATCH --mem=8G

# Workload configurations (N p)
CONFIGS=(
    "20000 50"
    "50000 300"
    "2000 2000"
)

run_benchmarks() {
    local compiler_label=$1
    local binary=$2

    echo "========================================================================"
    echo " Running Benchmarks for Compiler: $compiler_label"
    echo "========================================================================"

    for cfg in "${CONFIGS[@]}"; do
        read -r N p <<< "$cfg"
        echo ""
        echo "--- Configuration: N = $N, p =$p ---"
        ./"$binary" "$N" "$p"
    done
    echo ""
}

# ==============================================================================
# 1. GCC 10.1.0 Benchmarks
# ==============================================================================
echo "------------------------------------------------------------------------"
echo "Loading GCC 10.1.0..."
echo "------------------------------------------------------------------------"
module purge
module load cesga/2020 gcc/10.1.0

GCC_OPT_LEVELS=("-O0" "-O2 -march=native" "-O3 -march=native" "-Ofast -march=native")

for opt in "${GCC_OPT_LEVELS[@]}"; do
    echo "Compiling with gcc $opt..."
    gcc $opt -std=c11 -c gaussian.c -o gaussian.o
    gcc $opt -std=c11 -c linreg.c -o linreg.o
    gcc $opt -o linreg_gcc linreg.o gaussian.o -lm

    run_benchmarks "GCC 10.1.0 ($opt)" "linreg_gcc"

    rm -f *.o linreg_gcc
done

# ==============================================================================
# 2. Intel ICC Benchmarks
# ==============================================================================
echo "------------------------------------------------------------------------"
echo "Loading Intel 2021.3.0 (icc)..."
echo "------------------------------------------------------------------------"
module purge
module load cesga/2020 intel/2021.3.0

INTEL_OPT_LEVELS=("-O0