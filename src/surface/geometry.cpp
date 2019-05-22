#include "geometrycentral/surface/geometry.h"
#include <fstream>
#include <limits>

namespace geometrycentral {

template <>
void Geometry<Euclidean>::normalize() {
  // compute center of mass
  Vector3 cm;
  for (Vertex v : mesh.vertices()) {
    cm += position(v);
  }
  cm /= mesh.nVertices();

  // translate to origin and determine radius
  double rMax = 0;
  for (Vertex v : mesh.vertices()) {
    Vector3& p = position(v);
    p -= cm;
    rMax = std::max(rMax, norm(p));
  }

  // rescale to unit sphere
  for (Vertex v : mesh.vertices()) {
    position(v) /= rMax;
  }
}

} // namespace geometrycentral
