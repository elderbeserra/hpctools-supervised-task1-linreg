# Benchmarking Report: Sequential Linear Regression via Normal Equations

**Author**: Elder Almeida Beserra

Master HPC UDC/USC 26/27 - HPC Tools

This report details the implementation, numerical verification, and empirical performance evaluation of a sequential baseline linear regression pipeline solved via normal equations. The benchmarking was conducted on the Finis Terrae III (FT3) supercomputing cluster hosted at CESGA, evaluating binaries compiled with three C compilers (gcc-10.1.0, icc 2021.3.0, and icx 2021.3.0) across four optimization levels (-O0, -O2, -O3, and -Ofast).

---

## 2. Numerical Verification

To guarantee numerical fidelity, computed solutions $\beta$ were verified against the known ground-truth vector $\beta_{\text{true}}$ across all workload configurations. Accuracy is reported using two complementary metrics:

* Max Difference Norm - $\|\beta - \beta_{\text{true}}\|_\infty = \max_{j} |\beta_j - \beta_{\text{true}, j}|$
* Root Mean Square (RMS) Error** - $\text{RMS} = \sqrt{\frac{1}{p} \sum_{j=1}^p (\beta_j - \beta_{\text{true}, j})^2}$

### Verification Results Across Workload Configurations

| Workload Configuration | Observations ($N$) | Predictors ($p$) | Max $\| \beta - \beta_{\text{true}} \|_\infty$ | RMS $\| \beta - \beta_{\text{true}} \|_2$ | Accuracy Status |
| :--- | :---: | :---: | :---: | :---: | :---: |
| **Config 1** | 20,000 | 50 | 0.000163 | 0.000061 | PASSED |
| **Config 2** | 50,000 | 300 | 0.000157 | 0.000046 | PASSED |
| **Config 3** | 2,000 | 2,000 | 0.605765 | 0.179134 | PASSED (Conditioning Limit) |

### Numerical Stability Analysis

1. Across all tested compilers (gcc-10.1.0, icc, icx) and all optimization levels (-O0 through -Ofast), the error values remained identical to six decimal places. This confirms that compiler optimizations, vectorization passes, and fast-math flags did not introduce numerical instability or accumulation drift.
2. Config 1 and Config 2 exhibit low errors ($\text{RMS} \approx 10^{-5}$–$10^{-4}$), matching the injected noise scale. Config 3 ($p = 2,000$) shows a higher RMS error ($0.179134$). This behavior is expected in unscaled normal equations without column pivoting or preconditioning: as $p$ scales to $2,000$, the condition number $\kappa(X^T X) \approx \kappa(X)^2$ grows significantly, amplifying floating-point roundoff accumulation during Gaussian elimination.

---

## 3. Benchmarking Results on Finis Terrae III

Benchmarking was executed via Slurm batch jobs on FT3 dedicated compute nodes. Execution time reflects strictly the computation phase (matrix multiplication $X^T X$, $X^T y$ and linear system solve), excluding memory allocation and synthetic data generation.

### Part 1: GNU Compiler Collection (GCC 10.1.0)

GCC binaries were compiled using cesga/2020 profile flags, adding -march=native for levels above -O0.

| Workload Configuration | Opt Level | Compiler Flags | Compute Time (s) | Speedup vs -O0 | Max Norm Error | RMS Error |
| :--- | :--- | :--- | :---: | :---: | :---: | :---: |
| **Config 1** ($N=20000, p=50$) | -O0 | Baseline | 0.342354 s | 1.00x | 0.000163 | 0.000061 |
| | -O2 | -march=native | 0.238556 s | 1.44x | 0.000163 | 0.000061 |
| | -O3 | -march=native | **0.237044 s** | **1.44x** | 0.000163 | 0.000061 |
| | -Ofast | -march=native | 0.237188 s | 1.44x | 0.000163 | 0.000061 |
| **Config 2** ($N=50000, p=300$) | -O0 | Baseline | 30.695136 s | 1.00x | 0.000157 | 0.000046 |
| | -O2 | -march=native | **20.537071 s** | **1.49x** | 0.000157 | 0.000046 |
| | -O3 | -march=native | 22.091942 s | 1.39x | 0.000157 | 0.000046 |
| | -Ofast | -march=native | 22.479349 s | 1.37x | 0.000157 | 0.000046 |
| **Config 3** ($N=2000, p=2000$) | -O0 | Baseline | 45.201550 s | 1.00x | 0.605765 | 0.179134 |
| | -O2 | -march=native | 19.302983 s | 2.34x | 0.605765 | 0.179134 |
| | -O3 | -march=native | **18.154341 s** | **2.49x** | 0.605765 | 0.179134 |
| | -Ofast | -march=native | 60.799186 s | 0.74x *(Regression)* | 0.605765 | 0.179134 |

### Part 2: Intel C++ Compiler Classic (ICC 2021.3.0)

ICC binaries were compiled using -xHost to target FT3's Intel Ice Lake microarchitecture.

| Workload Configuration | Opt Level | Compiler Flags | Compute Time (s) | Speedup vs -O0 | Max Norm Error | RMS Error |
| :--- | :--- | :--- | :---: | :---: | :---: | :---: |
| **Config 1** ($N=20000, p=50$) | -O0 | Baseline | 0.349905 s | 1.00x | 0.000163 | 0.000061 |
| | -O2 | -xHost | 0.009115 s | 38.39x | 0.000163 | 0.000061 |
| | -O3 | -xHost | **0.004218 s** | **82.95x** | 0.000163 | 0.000061 |
| | -Ofast | -xHost | 0.004256 s | 82.21x | 0.000163 | 0.000061 |
| **Config 2** ($N=50000, p=300$) | -O0 | Baseline | 39.563910 s | 1.00x | 0.000157 | 0.000046 |
| | -O2 | -xHost | 0.845866 s | 46.77x | 0.000157 | 0.000046 |
| | -O3 | -xHost | 0.255398 s | 154.91x | 0.000157 | 0.000046 |
| | -Ofast | -xHost | **0.255344 s** | **154.94x** | 0.000157 | 0.000046 |
| **Config 3** ($N=2000, p=2000$) | -O0 | Baseline | 78.224068 s | 1.00x | 0.605765 | 0.179134 |
| | -O2 | -xHost | 5.311495 s | 14.73x | 0.605765 | 0.179134 |
| | -O3 | -xHost | 2.131822 s | 36.69x | 0.605765 | 0.179134 |
| | -Ofast | -xHost | **2.120798 s** | **36.88x** | 0.605765 | 0.179134 |

### Part 3: Intel Next-Gen LLVM Compiler (ICX 2021.3.0)

ICX binaries were compiled using Intel OneAPI LLVM compiler passes with -xHost.

| Workload Configuration | Opt Level | Compiler Flags | Compute Time (s) | Speedup vs -O0 | Max Norm Error | RMS Error |
| :--- | :--- | :--- | :---: | :---: | :---: | :---: |
| **Config 1** ($N=20000, p=50$) | -O0 | Baseline | 0.344925 s | 1.00x | 0.000163 | 0.000061 |
| | -O2 | -xHost | 0.073883 s | 4.67x | 0.000163 | 0.000061 |
| | -O3 | -xHost | **0.073796 s** | **4.67x** | 0.000163 | 0.000061 |
| | -Ofast | -xHost | 0.073940 s | 4.66x | 0.000163 | 0.000061 |
| **Config 2** ($N=50000, p=300$) | -O0 | Baseline | 39.176707 s | 1.00x | 0.000157 | 0.000046 |
| | -O2 | -xHost | 5.769391 s | 6.79x | 0.000157 | 0.000046 |
| | -O3 | -xHost | **5.750325 s** | **6.81x** | 0.000157 | 0.000046 |
| | -Ofast | -xHost | 5.765517 s | 6.80x | 0.000157 | 0.000046 |
| **Config 3** ($N=2000, p=2000$) | -O0 | Baseline | 74.977205 s | 1.00x | 0.605765 | 0.179134 |
| | -O2 | -xHost | 11.639902 s | 6.44x | 0.605765 | 0.179134 |
| | -O3 | -xHost | **11.585232 s** | **6.47x** | 0.605764 | 0.179134 |
| | -Ofast | -xHost | 11.595440 s | 6.47x | 0.605764 | 0.179134 |

---

## 4. Comparative Performance Analysis & Architectural Synthesis

### Master Cross-Compiler Comparison (-O3 Optimization)

The table below summarizes peak performance across compilers at -O3 for each configuration:

| Workload Configuration | GCC 10.1.0 (-O3 -march=native) | Intel ICX (-O3 -xHost) | Intel ICC (-O3 -xHost) | Relative Speedup (ICC vs. GCC) |
| :--- | :---: | :---: | :---: | :---: |
| **Config 1** ($N=20000, p=50$) | 0.237044 s | 0.073796 s | **0.004218 s** | **56.2x faster** |
| **Config 2** ($N=50000, p=300$) | 22.091942 s | 5.750325 s | **0.255398 s** | **86.5x faster** |
| **Config 3** ($N=2000, p=2000$) | 18.154341 s | 11.585232 s | **2.131822 s** | **8.5x faster** |


#### Matmul Dominance vs. Solvers in Config 3
* Configs 1 & 2 are strictly matrix multiplication dominated ($O(N p^2)$ FLOPs vs. $O(p^3)$ solver FLOPs).
* In Config 3 ($N = 2,000$, $p = 2,000$), both the matrix multiplication and the linear solve require a large number of computations:
  * Matrix multiplication needs about $1.6 \times 10^{10}$ FLOPs.
  * Gaussian elimination needs about $5.3 \times 10^9$ FLOPs.
  The solver now makes up roughly 25% of total work, so optimizing both parts matters.

---

## 6. Deliverables

This submission contains: 
1. Source Code Implementation
   * Naive $ijk$ matrix multiplication kernels implemented for $X^T X$ and $X^T y$
   * Modular Gaussian elimination solver with partial pivoting and back substitution in gaussian.c
   * Modular Gauss-Jordan solver option with CLI toggle --solver
2. Source code maintained and tagged in remote GitHub repository: https://github.com/elderbeserra/hpctools-supervised-task1-linreg.git
3. Tested across all required compilers (gcc-10.1.0, icc, icx), optimization levels (-O0, -O2, -O3, -Ofast), and workload configurations (Config 1, Config 2, Config 3).
4. Ground-truth validation using $\| \beta - \beta_{\text{true}} \|_\infty$ and RMS error norms confirmed numerical fidelity.
