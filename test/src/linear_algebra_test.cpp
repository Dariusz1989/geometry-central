#include "geometrycentral/numerical/linear_algebra_utilities.h"
#include "geometrycentral/numerical/linear_solvers.h"
#include "geometrycentral/surface/meshio.h"

#include "load_test_meshes.h"

#include "gtest/gtest.h"

#include <iostream>
#include <random>
#include <string>
#include <unordered_set>


using namespace geometrycentral;
using namespace geometrycentral::surface;
using std::cout;
using std::endl;


// ============================================================
// =============== General helpers
// ============================================================

class LinearAlgebraTestSuite : public ::testing::Test {
protected:
  static void SetUpTestSuite() {

    // Load spot so we can use his connectivity as an interesting mesh
    std::string fullPath = std::string(GC_TEST_ASSETS_ABS_PATH) + "/" + "spot.ply";
    cout << "  -- info: Loading mesh asset from " << fullPath << endl;
    std::tie(spotMesh, spotGeometry) = loadMesh(fullPath);
  }

  // == Members

  // Set the RNG so results are predictable
  std::mt19937 test_mersenne_twister{0};

  // Use spot to generate interesting matrices
  static std::unique_ptr<HalfedgeMesh> spotMesh;
  static std::unique_ptr<VertexPositionGeometry> spotGeometry;

  // == Random generators

  // Generate a random in range [low, high]
  template <typename T>
  T randomFromRange(double low, double high);
  template <>
  float randomFromRange(double low, double high) {
    std::uniform_real_distribution<float> dist(low, high);
    return dist(test_mersenne_twister);
  };
  template <>
  double randomFromRange(double low, double high) {
    std::uniform_real_distribution<double> dist(low, high);
    return dist(test_mersenne_twister);
  };
  template <>
  std::complex<double> randomFromRange(double low, double high) {
    return std::complex<double>(randomFromRange<double>(low, high), randomFromRange<double>(low, high));
  }


  // == Random test matrices
  template <typename T>
  SparseMatrix<T> buildSPDTestMatrix() {

    // Use spot's graph laplcian, with random positive weights
    size_t N = spotMesh->nVertices();
    std::vector<Eigen::Triplet<T>> tripletList;
    VertexData<size_t> vertexIndices = spotMesh->getVertexIndices();

    for (Edge e : spotMesh->edges()) {

      size_t vA = vertexIndices[e.halfedge().vertex()];
      size_t vB = vertexIndices[e.halfedge().twin().vertex()];

      T w = randomFromRange<T>(0.1, 1.);

      tripletList.emplace_back(vA, vA, std::abs(w) + .1); // to make the matrix strictly positive definite
      tripletList.emplace_back(vB, vB, std::abs(w) + .1);
      tripletList.emplace_back(vA, vB, -w);
      tripletList.emplace_back(vB, vA, -conj(w));
    }

    SparseMatrix<T> mat(N, N);
    mat.setFromTriplets(tripletList.begin(), tripletList.end());

    return mat;
  }


  template <typename T>
  Vector<T> randomVector(size_t N) {
    Vector<T> vec(N);
    for (size_t i = 0; i < N; i++) {
      vec(i) = randomFromRange<T>(-1, 1);
    }
    return vec;
  }
};
std::unique_ptr<HalfedgeMesh> LinearAlgebraTestSuite::spotMesh;
std::unique_ptr<VertexPositionGeometry> LinearAlgebraTestSuite::spotGeometry;

// ============================================================
// =============== Validators and converters
// ============================================================

TEST_F(LinearAlgebraTestSuite, IdentityMatrixTest) {

  size_t count = 42;

  { // float
    SparseMatrix<float> idMat = identityMatrix<float>(count);

    EXPECT_EQ(idMat.rows(), count);
    EXPECT_EQ(idMat.cols(), count);
    EXPECT_NEAR(idMat.sum(), (double)count, 1e-6);

    EXPECT_EQ(idMat.coeffRef(7, 7), 1);
  }

  { // double
    SparseMatrix<double> idMat = identityMatrix<double>(count);

    EXPECT_EQ(idMat.rows(), count);
    EXPECT_EQ(idMat.cols(), count);
    EXPECT_NEAR(idMat.sum(), (double)count, 1e-6);

    EXPECT_EQ(idMat.coeffRef(7, 7), 1);
  }

  { // complex
    SparseMatrix<std::complex<double>> idMat = identityMatrix<std::complex<double>>(count);

    EXPECT_EQ(idMat.rows(), count);
    EXPECT_EQ(idMat.cols(), count);
    EXPECT_NEAR(std::abs(idMat.sum()), count, 1e-6);

    EXPECT_EQ(idMat.coeffRef(7, 7), std::complex<double>(1.));
  }
}


TEST_F(LinearAlgebraTestSuite, ShiftDiagonalTest) {

  size_t count = 42;
  double eps = 0.03;

  { // float
    SparseMatrix<float> idMat = identityMatrix<float>(count);
    shiftDiagonal<float>(idMat, eps);

    EXPECT_EQ(idMat.rows(), count);
    EXPECT_EQ(idMat.cols(), count);
    EXPECT_NEAR(idMat.sum(), (double)count + count * eps, 1e-4);
    EXPECT_NEAR(idMat.coeffRef(7, 7), 1 + eps, 1e-4);
  }

  { // double
    SparseMatrix<double> idMat = identityMatrix<double>(count);
    shiftDiagonal<double>(idMat, eps);

    EXPECT_EQ(idMat.rows(), count);
    EXPECT_EQ(idMat.cols(), count);
    EXPECT_NEAR(idMat.sum(), (double)count + count * eps, 1e-6);

    EXPECT_NEAR(idMat.coeffRef(7, 7), 1 + eps, 1e-6);
  }

  { // complex
    SparseMatrix<std::complex<double>> idMat = identityMatrix<std::complex<double>>(count);
    shiftDiagonal<std::complex<double>>(idMat, eps);

    EXPECT_EQ(idMat.rows(), count);
    EXPECT_EQ(idMat.cols(), count);
    EXPECT_NEAR(std::abs(idMat.sum()), (double)count + count * eps, 1e-6);

    EXPECT_LT(std::abs(idMat.coeffRef(7, 7) - std::complex<double>(1. + eps)), 1e-6);
  }
}

TEST_F(LinearAlgebraTestSuite, ComplexToRealTest) {

  // Get an interesting complex matrix
  SparseMatrix<std::complex<double>> mat = buildSPDTestMatrix<std::complex<double>>();

  // Get an interesting vector
  Vector<std::complex<double>> vec = randomVector<std::complex<double>>(mat.rows());

  // Original product
  Vector<std::complex<double>> prod = mat * vec;

  // Split
  SparseMatrix<double> matR = complexToReal(mat);
  Vector<double> vecR = complexToReal(vec);
  Vector<double> prodR = matR * vecR;

  // Check same
  for (long int i = 0; i < mat.rows(); i++) {
    EXPECT_NEAR(prod(i).real(), prodR(2 * i), 1e-6);
    EXPECT_NEAR(prod(i).imag(), prodR(2 * i + 1), 1e-6);
  }
}

TEST_F(LinearAlgebraTestSuite, CheckFiniteTest) {

  { // float
    SparseMatrix<float> mat = buildSPDTestMatrix<float>();
    EXPECT_NO_THROW(checkFinite(mat));
    mat.coeffRef(5, 5) = std::numeric_limits<float>::infinity();
    EXPECT_THROW(checkFinite(mat), std::logic_error);
  }


  { // double
    SparseMatrix<double> mat = buildSPDTestMatrix<double>();
    EXPECT_NO_THROW(checkFinite(mat));
    mat.coeffRef(5, 5) = std::numeric_limits<double>::infinity();
    EXPECT_THROW(checkFinite(mat), std::logic_error);
  }

  { // std::complex<double>
    SparseMatrix<std::complex<double>> mat = buildSPDTestMatrix<std::complex<double>>();
    EXPECT_NO_THROW(checkFinite(mat));
    mat.coeffRef(5, 5) = std::complex<double>(std::numeric_limits<double>::infinity(), 0.);
    EXPECT_THROW(checkFinite(mat), std::logic_error);
  }
}


TEST_F(LinearAlgebraTestSuite, CheckSymmetricTest) {

  { // float
    SparseMatrix<float> mat = buildSPDTestMatrix<float>();
    EXPECT_NO_THROW(checkSymmetric(mat));
    mat.coeffRef(5, 8) = 0.3;
    EXPECT_THROW(checkSymmetric(mat), std::logic_error);
  }


  { // double
    SparseMatrix<double> mat = buildSPDTestMatrix<double>();
    EXPECT_NO_THROW(checkSymmetric(mat));
    mat.coeffRef(5, 8) = 0.3;
    EXPECT_THROW(checkSymmetric(mat), std::logic_error);
  }

  { // std::complex<double>
    SparseMatrix<std::complex<double>> mat = buildSPDTestMatrix<std::complex<double>>();
    EXPECT_THROW(checkSymmetric(mat), std::logic_error);
    SparseMatrix<std::complex<double>> matT = mat.transpose();
    SparseMatrix<std::complex<double>> smat = mat + matT;
    EXPECT_NO_THROW(checkSymmetric(smat));
  }
}

TEST_F(LinearAlgebraTestSuite, CheckHermitianTest) {

  { // float
    SparseMatrix<float> mat = buildSPDTestMatrix<float>();
    EXPECT_NO_THROW(checkHermitian(mat));
    mat.coeffRef(5, 8) = 0.3;
    EXPECT_THROW(checkHermitian(mat), std::logic_error);
  }


  { // double
    SparseMatrix<double> mat = buildSPDTestMatrix<double>();
    EXPECT_NO_THROW(checkHermitian(mat));
    mat.coeffRef(5, 8) = 0.3;
    EXPECT_THROW(checkHermitian(mat), std::logic_error);
  }

  { // std::complex<double>
    SparseMatrix<std::complex<double>> mat = buildSPDTestMatrix<std::complex<double>>();
    EXPECT_NO_THROW(checkHermitian(mat));
    mat.coeffRef(5, 8) = 0.3;
    EXPECT_THROW(checkHermitian(mat), std::logic_error);
  }
}


TEST_F(LinearAlgebraTestSuite, BlockDecomposeTest) {

  SparseMatrix<double> mat = buildSPDTestMatrix<double>();

  Vector<bool> split(mat.rows());
  size_t nA = 0;
  for (long int i = 0; i < split.rows(); i++) {
    split(i) = (randomFromRange<double>(0., 1.) > 0.5);
    if (split(i)) nA++;
  }
  size_t nB = mat.rows() - nA;

  // Decompose
  BlockDecompositionResult<double> decomp = blockDecomposeSquare(mat, split, true);

  // Check sizes
  EXPECT_EQ(decomp.AA.rows(), nA);
  EXPECT_EQ(decomp.AA.cols(), nA);
  EXPECT_EQ(decomp.AB.rows(), nA);
  EXPECT_EQ(decomp.AB.cols(), nB);
  EXPECT_EQ(decomp.BA.rows(), nB);
  EXPECT_EQ(decomp.BA.cols(), nA);
  EXPECT_EQ(decomp.BB.rows(), nB);
  EXPECT_EQ(decomp.BB.cols(), nB);

  // Hopefully all the entries made it through
  EXPECT_NEAR(decomp.AA.sum() + decomp.BB.sum() + decomp.AB.sum() + decomp.BA.sum(), mat.sum(), 1e-6);

  // Some data
  Vector<double> x = randomVector<double>(mat.rows());

  // Split up the vec vector
  Vector<double> xA, xB;
  decomposeVector(decomp, x, xA, xB);

  EXPECT_EQ(xA.rows(), nA);
  EXPECT_EQ(xB.rows(), nB);
  EXPECT_NEAR(xA.sum() + xB.sum(), x.sum(), 1e-6);

  // Do some arithmetic
  Vector<double> y = mat * x;
  Vector<double> yA = decomp.AA * xA + decomp.AB * xB;
  Vector<double> yB = decomp.BA * xA + decomp.BB * xB;

  // Recombine
  Vector<double> yAssemb = reassembleVector(decomp, yA, yB);

  // Should be very close
  for (long int i = 0; i < mat.rows(); i++) {
    EXPECT_NEAR(y(i), yAssemb(i), 1e-6);
  }
}
