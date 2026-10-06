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

    if ! command -v "$compiler" >/dev/null 2>&1; then
        echo "ERROR: compiler '$compiler' not found in PATH after module load" >&2
        exit 1
    fi

    echo "Compiling with $compiler $opt..."
    # shellcheck disable=SC2086
    $compiler $opt "${CFLAGS_COMMON[@]}" -c gaussian.c -o gaussian.o
    # shellcheck disable=SC2086
    $compiler $opt "${CFLAGS_COMMON[@]}" -c linreg.c -o linreg.o
    # shellcheck disable=SC2086
    $compiler $opt "${CFLAGS_COMMON[@]}" -c rng.c -o rng.o
    # shellcheck disable=SC2086
    $compiler $opt -o "$out_binary" linreg.o gaussian.o rng.o -lm
}

clean_objects() {
    local binary=$1
    rm -f gaussian.o linreg.o rng.o "$binary"
}

load_cesga_base() {
    module --force purge
    module load cesga/2020
}

# ==============================================================================
# 1. GCC 10.1.0
# ==============================================================================
echo "------------------------------------------------------------------------"
echo "Loading GCC (cesga/2020 — Gentoo prefix GCC 10.1.0)..."
echo "------------------------------------------------------------------------"
load_cesga_base
gcc --version | head -n 1

GCC_OPT_LEVELS=("-O0" "-O2 -march=native" "-O3 -march=native" "-Ofast -march=native")

for opt in "${GCC_OPT_LEVELS[@]}"; do
    compile_linreg gcc "$opt" linreg_gcc
    run_benchmarks "GCC 10.1.0 ($opt)" "linreg_gcc"
    clean_objects linreg_gcc
done

# ==============================================================================
# 2. Intel ICC 2021.3.x 
# ==============================================================================
echo "------------------------------------------------------------------------"
echo "Loading Intel / icc (cesga/2020 + intel)..."
echo "------------------------------------------------------------------------"
load_cesga_base
module load intel
icc --version | head -n 1

INTEL_OPT_LEVELS=("-O0" "-O2 -xHost" "-O3 -xHost" "-Ofast -xHost")

for opt in "${INTEL_OPT_LEVELS[@]}"; do
    compile_linreg icc "$opt" linreg_icc
    run_benchmarks "Intel ICC ($opt)" "linreg_icc"
    clean_objects linreg_icc
done

# ==============================================================================
# 3. Intel ICX 2021.3.x
# ==============================================================================
echo "------------------------------------------------------------------------"
echo "Loading Intel ICX (cesga/2020 + intel)..."
echo "------------------------------------------------------------------------"
load_cesga_base
module load intel
icx --version | head -n 1

for opt in "${INTEL_OPT_LEVELS[@]}"; do
    compile_linreg icx "$opt" linreg_icx
    run_benchmarks "Intel ICX ($opt)" "linreg_icx"
    clean_objects linreg_icx
done

echo "All benchmark runs completed!"
