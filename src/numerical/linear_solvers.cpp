#include "geometrycentral/numerical/linear_solvers.h"

#include "geometrycentral/numerical/linear_algebra_utilities.h"


using namespace Eigen;

namespace geometrycentral {

// NOTE left empty for now

template class LinearSolver<double>;
template class LinearSolver<float>;
template class LinearSolver<Complex>;


} // namespace geometrycentral
