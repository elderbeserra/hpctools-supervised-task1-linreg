#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gaussian.h"

/* -------------------------------------------------------------------------
 * TODO (STUDENT): gaussian_elimination_solve
 *
 * Solve the p x p system:
 *
 *   XtX * beta = Xty
 *
 * using Gaussian elimination with partial pivoting, followed by back
 * substitution:
 *
 *   1. Build an augmented p x (p+1) matrix [XtX | Xty] (work on a local
 *      copy — do not modify XtX/Xty in place, you may want to keep them
 *      for the report).
 *   2. Forward elimination: for each pivot column k = 0..p-1,
 *        a. partial pivoting: find the row r >= k with the largest
 *           absolute value in column k, and swap rows k and r if r != k
 *           (this avoids dividing by a very small/zero pivot).
 *        b. eliminate column k from all rows below k by subtracting an
 *           appropriate multiple of row k.
 *   3. Back substitution: once the augmented matrix is in upper
 *      triangular form, solve for beta[p-1], beta[p-2], ..., beta[0]
 *      from the bottom row upward.
 *
 * XtX  : p x p, row-major (read-only)
 * Xty  : p (right-hand side, read-only)
 * beta : p (output, caller-allocated)
 * ---------------------------------------------------------------------- */

/* initialize the augmented matrix */
 static void init_augmented(double *aug, const double *XtX, const double *Xty, int p, int stride) {
   for (int i = 0; i < p; i++) {
       memcpy(&aug[i * stride], &XtX[i * p], p * sizeof(double));
       aug[i * stride + p] = Xty[i];
   }
}

/* partial pivoting */
static int pivot_and_swap(double *aug, int k, int p, int stride) {
   int max_row = k;
   double max_val = fabs(aug[k * stride + k]);
   for (int r = k + 1; r < p; r++) {
       double val = fabs(aug[r * stride + k]);
       if (val > max_val) {
           max_val = val;
           max_row = r;
       }
   }
   if (max_row != k) {
       for (int j = k; j <= p; j++) {
           double temp = aug[k * stride + j];
           aug[k * stride + j] = aug[max_row * stride + j];
           aug[max_row * stride + j] = temp;
       }
   }
   return max_val > 1e-12;
}

void gaussian_elimination_solve(const double *XtX, const double *Xty, double *beta, int p) {
   int stride = p + 1;
   double *aug = (double *)malloc(p * stride * sizeof(double));
   if (!aug) return;

   init_augmented(aug, XtX, Xty, p, stride);

   for (int k = 0; k < p; k++) {
       if (!pivot_and_swap(aug, k, p, stride)) continue;
       for (int i = k + 1; i < p; i++) {
           double f = aug[i * stride + k] / aug[k * stride + k];
           for (int j = k; j <= p; j++) {
               aug[i * stride + j] -= f * aug[k * stride + j];
           }
       }
   }

   for (int i = p - 1; i >= 0; i--) {
       double sum = aug[i * stride + p];
       for (int j = i + 1; j < p; j++) sum -= aug[i * stride + j] * beta[j];
       beta[i] = sum / aug[i * stride + i];
   }
   free(aug);
}

void gauss_jordan_solve(const double *XtX, const double *Xty, double *beta, int p) {
   int stride = p + 1;
   double *aug = (double *)malloc(p * stride * sizeof(double));
   if (!aug) return;

   init_augmented(aug, XtX, Xty, p, stride);

   for (int k = 0; k < p; k++) {
       if (!pivot_and_swap(aug, k, p, stride)) continue;
       double pivot = aug[k * stride + k];
       for (int j = k; j <= p; j++) aug[k * stride + j] /= pivot;

       for (int i = 0; i < p; i++) {
           if (i == k) continue;
           double f = aug[i * stride + k];
           for (int j = k; j <= p; j++) aug[i * stride + j] -= f * aug[k * stride + j];
       }
   }

   for (int i = 0; i < p; i++) beta[i] = aug[i * stride + p];
   free(aug);
}

  /* TODO: implement Gaussian elimination with partial pivoting +
   * back substitution here. A scratch p x (p+1) augmented matrix can be
   * allocated locally with malloc and freed before returning.
   */
