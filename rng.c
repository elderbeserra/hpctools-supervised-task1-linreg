#include <stdint.h>
#include <math.h>
#include "rng.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* -------------------------------------------------------------------------
 * Simple reproducible RNG (DO NOT MODIFY)
 * ---------------------------------------------------------------------- */
static unsigned int rng_state;

void rng_seed(unsigned int seed) {
  rng_state = seed ? seed : 1u;
}

double rng_uniform(void) {
  unsigned int x = rng_state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  rng_state = x;

  return (double)(x) / (double)UINT32_MAX;
}

double rng_gaussian(void) {
  double u1 = rng_uniform();
  double u2 = rng_uniform();
  if (u1 < 1e-12) u1 = 1e-12;

  return sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
}
