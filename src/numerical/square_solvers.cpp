#include "geometrycentral/numerical/linear_solvers.h"

#include "geometrycentral/numerical/linear_algebra_utilities.h"

#ifdef HAVE_SUITESPARSE
#include "geometrycentral/numerical/suitesparse_utilities.h"
#include <umfpack.h>
#endif

using namespace Eigen;

namespace geometrycentral {

template <typename T>
SquareSolver<T>::~SquareSolver() {
#ifdef HAVE_SUITESPARSE
  if (cMat != nullptr) {
    cholmod_l_free_sparse(&cMat, context);
    cMat = nullptr;
  }
  if (symbolicFactorization != nullptr) {
    umfpack_dl_free_symbolic(&symbolicFactorization);
  }
  if (numericFactorization != nullptr) {
    umfpack_dl_free_numeric(&numericFactorization);
  }
#endif
}

// Helper functions to interface with umfpack without explicitly specializing all of prepare() and solve(). Different
// function calls are needed for real vs. complex case.
// Note that float case is identical to double; umfpack never uses single precision
// Note that we use packed (interleaved) complex formats, rather than passing separate real and imaginary arrays
// TODO: The old SparseMatrix code in grand-central does seem to do this. Was is a bad bug, or am I missing something?
namespace {

#ifdef HAVE_SUITESPARSE
// = Factorization
template <typename T>
void umfFactor(size_t N, cholmod_sparse* mat, void*& symbolicFac, void*& numericFac);

template <>
void umfFactor<double>(size_t N, cholmod_sparse* mat, void*& symbolicFac, void*& numericFac) {
  SuiteSparse_long* cMat_p = (SuiteSparse_long*)mat->p;
  SuiteSparse_long* cMat_i = (SuiteSparse_long*)mat->i;
  double* cMat_x = (double*)mat->x;
  umfpack_dl_symbolic(N, N, cMat_p, cMat_i, cMat_x, &symbolicFac, NULL, NULL);
  umfpack_dl_numeric(cMat_p, cMat_i, cMat_x, symbolicFac, &numericFac, NULL, NULL);
}
template <>
void umfFactor<float>(size_t N, cholmod_sparse* mat, void*& symbolicFac, void*& numericFac) {
  SuiteSparse_long* cMat_p = (SuiteSparse_long*)mat->p;
  SuiteSparse_long* cMat_i = (SuiteSparse_long*)mat->i;
  double* cMat_x = (double*)mat->x;
  umfpack_dl_symbolic(N, N, cMat_p, cMat_i, cMat_x, &symbolicFac, NULL, NULL);
  umfpack_dl_numeric(cMat_p, cMat_i, cMat_x, symbolicFac, &numericFac, NULL, NULL);
}
template <>
void umfFactor<Complex>(size_t N, cholmod_sparse* mat, void*& symbolicFac, void*& numericFac) {
  SuiteSparse_long* cMat_p = (SuiteSparse_long*)mat->p;
  SuiteSparse_long* cMat_i = (SuiteSparse_long*)mat->i;
  double* cMat_x = (double*)mat->x;
  umfpack_zl_symbolic(N, N, cMat_p, cMat_i, cMat_x, NULL, &symbolicFac, NULL, NULL);
  umfpack_zl_numeric(cMat_p, cMat_i, cMat_x, NULL, symbolicFac, &numericFac, NULL, NULL);
}

// = Solves
template <typename T>
void umfSolve(size_t N, cholmod_sparse* mat, void* numericFac, Vector<T>& x, const Vector<T>& rhs);

template <>
void umfSolve<float>(size_t N, cholmod_sparse* mat, void* numericFac, Vector<float>& x, const Vector<float>& rhs) {
  Vector<double> xD = Vector<double>(N);
  Vector<double> rhsD = rhs.cast<double>(); // explicitly convert to doubles
  SuiteSparse_long* cMat_p = (SuiteSparse_long*)mat->p;
  SuiteSparse_long* cMat_i = (SuiteSparse_long*)mat->i;
  double* cMat_x = (double*)mat->x;
  umfpack_dl_solve(UMFPACK_A, cMat_p, cMat_i, cMat_x, &(xD[0]), &(rhsD[0]), numericFac, NULL, NULL);
  x = xD.cast<float>();
}
template <>
void umfSolve<double>(size_t N, cholmod_sparse* mat, void* numericFac, Vector<double>& x, const Vector<double>& rhs) {
  x = Vector<double>(N);
  SuiteSparse_long* cMat_p = (SuiteSparse_long*)mat->p;
  SuiteSparse_long* cMat_i = (SuiteSparse_long*)mat->i;
  double* cMat_x = (double*)mat->x;
  umfpack_dl_solve(UMFPACK_A, cMat_p, cMat_i, cMat_x, &(x[0]), &(rhs[0]), numericFac, NULL, NULL);
}
template <>
void umfSolve<Complex>(size_t N, cholmod_sparse* mat, void* numericFac, Vector<Complex>& x,
                       const Vector<Complex>& rhs) {
  // Note: the ordering of std::complex is specified by the standard, so this certainly works
  x = Vector<Complex>(N);
  SuiteSparse_long* cMat_p = (SuiteSparse_long*)mat->p;
  SuiteSparse_long* cMat_i = (SuiteSparse_long*)mat->i;
  double* cMat_x = (double*)mat->x;
  umfpack_zl_solve(UMFPACK_A, cMat_p, cMat_i, cMat_x, NULL, (double*)&(x[0]), NULL, (double*)&(rhs[0]), NULL,
                   numericFac, NULL, NULL);
}

#endif

} // namespace


template <typename T>
void SquareSolver<T>::prepare() {

  size_t N = this->mat.rows();

// Check some sanity
#ifndef GC_NLINALG_DEBUG
  if ((size_t)this->mat.cols() != N) {
    throw std::logic_error("Matrix must be square");
  }
  checkFinite(this->mat);
#endif

  this->mat.makeCompressed();

// Suitesparse variant
#ifdef HAVE_SUITESPARSE
  // Convert suitesparse format
  if (cMat != nullptr) {
    cholmod_l_free_sparse(&cMat, context);
  }
  cMat = toCholmod(this->mat, context);

  // Factor
  umfFactor<T>(N, cMat, symbolicFactorization, numericFactorization);


// Eigen variant
#else
  solver.compute(this->mat);
  if (solver.info() != Eigen::Success) {
    std::cerr << "Solver factorization error: " << solver.info() << std::endl;
    throw std::invalid_argument("Solver factorization failed");
  }
#endif
};

template <typename T>
Vector<T> SquareSolver<T>::solve(const Vector<T>& rhs) {
  Vector<T> out;
  solve(out, rhs);
  return out;
}

template <typename T>
void SquareSolver<T>::solve(Vector<T>& x, const Vector<T>& rhs) {

  size_t N = this->mat.rows();

  // Check some sanity
#ifndef GC_NLINALG_DEBUG
  if ((size_t)rhs.rows() != N) {
    throw std::logic_error("Vector is not the right length");
  }
  checkFinite(rhs);
#endif

  // Suitesparse version
#ifdef HAVE_SUITESPARSE

  // Templated helper does all the hard work
  umfSolve<T>(N, cMat, numericFactorization, x, rhs);

  // Eigen version
#else
  // Solve
  x = solver.solve(rhs);
  if (solver.info() != Eigen::Success) {
    std::cerr << "Solver error: " << solver.info() << std::endl;
    std::cerr << "Solver says: " << solver.lastErrorMessage() << std::endl;
    throw std::invalid_argument("Solve failed");
  }
#endif

  // Compute residual to spot bad solves
#ifndef GC_NLINALG_DEBUG
  Matrix<T, Dynamic, 1> residual = this->mat * x - rhs;
  double residualNorm = residual.norm();
  double relativeResidualNorm = residualNorm / rhs.norm();
  std::cout << "  -- Residual norm: " << residualNorm << "   relative residual norm: " << relativeResidualNorm
            << std::endl;
#endif
}

template <typename T>
Vector<T> solveSquare(const Eigen::SparseMatrix<T>& A, const Vector<T>& rhs) {
  SquareSolver<T> s(A);
  return s.solve(rhs);
}

// Explicit instantiations
template class SquareSolver<double>;
template class SquareSolver<float>;
template class SquareSolver<Complex>;

template Vector<float> solveSquare(const Eigen::SparseMatrix<float>& A, const Vector<float>& rhs);
template Vector<double> solveSquare(const Eigen::SparseMatrix<double>& A, const Vector<double>& rhs);
template Vector<Complex> solveSquare(const Eigen::SparseMatrix<Complex>& A, const Vector<Complex>& rhs);


} // namespace geometrycentral
