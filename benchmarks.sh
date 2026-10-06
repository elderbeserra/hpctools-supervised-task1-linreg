#!/bin/bash
#SBATCH -J lr_benchmarks
#SBATCH -o benchmarks_%j.out
#SBATCH -e benchmarks_%j.err
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=02:00:00
#SBATCH --mem=8G

set -euo pipefail

cd "${SLURM_SUBMIT_DIR:-$(dirname "${BASH_SOURCE[0]}")}"

CFLAGS_COMMON=(-std=c11 -D_POSIX_C_SOURCE=200809L)

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
        echo "--- Configuration: N = $N, p = $p ---"
        ./"$binary" "$N" "$p"
    done
    echo ""
}

compile_linreg() {
    local compiler=$1
    local opt=$2
    local out_binary=$3

    echo "Compiling with $compiler $opt..."
    $compiler $opt "${CFLAGS_COMMON[@]}" -c gaussian.c -o gaussian.o
    $compiler $opt "${CFLAGS_COMMON[@]}" -c linreg.c -o linreg.o
    $compiler $opt "${CFLAGS_COMMON[@]}" -c rng.c -o rng.o
    $compiler $opt -o "$out_binary" linreg.o gaussian.o rng.o -lm
}

clean_objects() {
    local binary=$1
    rm -f gaussian.o linreg.o rng.o "$binary"
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
    compile_linreg gcc "$opt" linreg_gcc
    run_benchmarks "GCC 10.1.0 ($opt)" "linreg_gcc"
    clean_objects linreg_gcc
done

# ==============================================================================
# 2. Intel ICC Benchmarks
# ==============================================================================
echo "------------------------------------------------------------------------"
echo "Loading Intel 2021.3.0 (icc)..."
echo "------------------------------------------------------------------------"
module purge
module load cesga/2020 intel/2021.3.0

INTEL_OPT_LEVELS=("-O0" "-O2 -xHost" "-O3 -xHost" "-Ofast -xHost")

for opt in "${INTEL_OPT_LEVELS[@]}"; do
    compile_linreg icc "$opt" linreg_icc
    run_benchmarks "Intel ICC ($opt)" "linreg_icc"
    clean_objects linreg_icc
done

# ==============================================================================
# 3. Intel ICX Benchmarks
# ==============================================================================
echo "------------------------------------------------------------------------"
echo "Loading Intel 2021.3.0 (icx)..."
echo "------------------------------------------------------------------------"
for opt in "${INTEL_OPT_LEVELS[@]}"; do
    compile_linreg icx "$opt" linreg_icx
    run_benchmarks "Intel ICX ($opt)" "linreg_icx"
    clean_objects linreg_icx
done

echo "All benchmark runs completed!"
