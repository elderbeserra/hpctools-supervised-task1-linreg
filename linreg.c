/* ============================================================================
 * linreg.c
 *
 * Linear regression via the normal equations (XtX * beta = Xty)
 *
 * Sequential baseline skeleton
 *
 * Provided (DO NOT MODIFY):
 *   - generate_data()   : synthetic X, beta_true, y = X*beta_true + noise
 *   - check_solution()  : compares computed beta against beta_true
 *   - main()            : argument parsing, timing, orchestration
 *
 * TO BE IMPLEMENTED BY THE STUDENT (see "TODO" markers):
 *   - compute_XtX()                : XtX = X^T * X   (naive triple-nested loop)
 *   - compute_Xty()                : Xty = X^T * y   (naive double loop)
 *   - gaussian_elimination_solve() :
 *
 * Build (example):
 *   gcc -O0 -std=c11 -o linreg linreg_skeleton.c -lm
 *
 * Usage:
 *   ./linreg N p [seed]
 *   e.g. ./linreg 20000 50
 * ==========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "rng.h"
#include "timer.h"
#include "gaussian.h"
#include "gemm.h"
#include "gemv.h"

/* -------------------------------------------------------------------------
 * generate_data (DO NOT MODIFY)
 *
 * Produces:
 *   X          : N x p, row-major, entries ~ N(0,1)
 *   beta_true  : p, entries ~ Uniform(-5, 5)
 *   y          : N,  y = X * beta_true + noise, noise ~ N(0, noise_std^2)
 *
 * All arrays are caller-allocated.
 * ---------------------------------------------------------------------- */
void generate_data(double *X, double *beta_true, double *y,
                   int N, int p, double noise_std) {
  for (int j = 0; j < p; j++) {
    beta_true[j] = -5.0 + 10.0 * rng_uniform();
  }

  for (int i = 0; i < N; i++) {
    double pred = 0.0;
    for (int j = 0; j < p; j++) {
      double xij = rng_gaussian();
      X[i * p + j] = xij;
      pred += xij * beta_true[j];
    }
    y[i] = pred + noise_std * rng_gaussian();
  }
}

/* -------------------------------------------------------------------------
 * check_solution (DO NOT MODIFY)
 *
 * Reports the max-norm and RMS difference between the computed beta and
 * the ground-truth beta_true used to generate the data.
 * ---------------------------------------------------------------------- */
static void check_solution(const double *beta, const double *beta_true, int p) {
  double max_diff = 0.0, sum_sq = 0.0;

  for (int j = 0; j < p; j++) {
    double diff = fabs(beta[j] - beta_true[j]);
    if (diff > max_diff) max_diff = diff;
    sum_sq += diff * diff;
  }

  double rms = sqrt(sum_sq / p);
  printf("Max |beta - beta_true|: %.6f\n", max_diff);
  printf("RMS |beta - beta_true|: %.6f\n", rms);
}

/* -------------------------------------------------------------------------
 * main (DO NOT MODIFY, beyond adapting reporting/logging as needed)
 * ---------------------------------------------------------------------- */
int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s N p [seed] [noise_std]\n", argv[0]);
    return 1;
  }

  int N = atoi(argv[1]);
  int p = atoi(argv[2]);
  unsigned int seed = (argc > 3) ? (unsigned int)atoi(argv[3]) : 42u;
  double noise_std = (argc > 4) ? atof(argv[4]) : 0.5;

  printf("Config: N=%d p=%d seed=%u noise_std=%.3f\n", N, p, seed, noise_std);

  rng_seed(seed);

  double *X         = malloc((size_t)N * p * sizeof(double));
  double *beta_true = malloc((size_t)p * sizeof(double));
  double *y         = malloc((size_t)N * sizeof(double));
  double *XtX       = malloc((size_t)p * p * sizeof(double));
  double *Xty       = malloc((size_t)p * sizeof(double));
  double *beta      = malloc((size_t)p * sizeof(double));

  if (!X || !beta_true || !y || !XtX || !Xty || !beta) {
    fprintf(stderr, "Allocation failed (try smaller N/p).\n");
    return 1;
  }

  generate_data(X, beta_true, y, N, p, noise_std);

  /* --- Timed region: only the compute kernels, not data generation --- */
  struct timespec t0, t1, t2, t3;

  timestamp(&t0);
  compute_XtX(X, XtX, N, p);
  timestamp(&t1);

  compute_Xty(X, y, Xty, N, p);
  timestamp(&t2);

  gaussian_elimination_solve(XtX, Xty, beta, p);
  timestamp(&t3);

  check_solution(beta, beta_true, p);

  // Print times:
  // printf("Time taken by ...: %.2f s\n", diff_seconds(&t1, &t0));

  free(X);
  free(beta_true);
  free(y);
  free(XtX);
  free(Xty);
  free(beta);

  return 0;
}
