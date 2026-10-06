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
  *   X         : N x p, row-major, entries ~ N(0,1)
  *   beta_true : p, entries ~ Uniform(-5, 5)
  *   y         : N, y = X * beta_true + noise, noise ~ N(0, noise_std^2)
  *
  * All arrays are caller-allocated.
  * ---------------------------------------------------------------------- */
void generate_data(double *X, double *beta_true, double *y, int N, int p, double noise_std) {
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
      if (diff > max_diff) {
          max_diff = diff;
      }
      sum_sq += diff * diff;
  }
  double rms = sqrt(sum_sq / p);
  printf("Max |beta - beta_true|: %.6f\n", max_diff);
  printf("RMS |beta - beta_true|: %.6f\n", rms);
}

// Naive (ijk) multiplication to compute XtX = X^T * X (p x p matrix)
void compute_XtX_naive(const double *X, double *XtX, int N, int p) {
  memset(XtX, 0, p * p * sizeof(double));
  for (int i = 0; i < p; i++) {
      for (int j = 0; j < p; j++) {
          double sum = 0.0;
          for (int k = 0; k < N; k++) {
              sum += X[k * p + i] * X[k * p + j];
          }
          XtX[i * p + j] = sum;
      }
  }
}
 
// Naive multiplication to compute Xty = X^T * y (p x 1 vector)
void compute_Xty_naive(const double *X, const double *y, double *Xty, int N, int p) {
  for (int i = 0; i < p; i++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++) {
          sum += X[k * p + i] * y[k];
      }
      Xty[i] = sum;
  }
}
 
/* -------------------------------------------------------------------------
 * main (DO NOT MODIFY, beyond adapting reporting/logging as needed)
 * ---------------------------------------------------------------------- */
int main(int argc, char **argv) {
  if (argc < 3) {
      fprintf(stderr, "Usage: %s N p [seed]\n", argv[0]);
      return 1;
  }

  int N = atoi(argv[1]);
  int p = atoi(argv[2]);
  unsigned long seed = (argc > 3) ? strtoul(argv[3], NULL, 10) : 42;

  rng_init(seed);

  double *X = (double *)malloc(N * p * sizeof(double));
  double *beta_true = (double *)malloc(p * sizeof(double));
  double *y = (double *)malloc(N * sizeof(double));
  double *XtX = (double *)malloc(p * p * sizeof(double));
  double *Xty = (double *)malloc(p * sizeof(double));
  double *beta = (double *)malloc(p * sizeof(double));

  if (!X || !beta_true || !y || !XtX || !Xty || !beta) {
      fprintf(stderr, "Error: Memory allocation failed.\n");
      return 1;
  }

  double noise_std = 0.01;
  generate_data(X, beta_true, y, N, p, noise_std);

  // Measure execution time
  timer_start();
  compute_XtX_naive(X, XtX, N, p);
  compute_Xty_naive(X, y, Xty, N, p);
  gaussian_elimination_solve(XtX, Xty, beta, p);
  double elapsed = timer_stop();

  printf("N = %d, p = %d\n", N, p);
  printf("Compute time: %.6f s\n", elapsed);
  check_solution(beta, beta_true, p);

  free(X);
  free(beta_true);
  free(y);
  free(XtX);
  free(Xty);
  free(beta);

  return 0;
}