/* partial pivoting */
void gaussian_elimination_solve(const double *XtX, const double *Xty, double *beta, int p);

/* alternative solver using Gauss-Jordan elimination */
void gauss_jordan_solve(const double *XtX, const double *Xty, double *beta, int p);