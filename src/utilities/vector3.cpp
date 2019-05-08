#include "geometrycentral/utilities/vector3.h"

#include <geometrycentral/utilities/utilities.h>

#include <cmath>
#include <iostream>

namespace geometrycentral {

std::ostream &operator<<(std::ostream &output, const Vector3 &v) {
  output << "<" << v.x << ", " << v.y << ", " << v.z << ">";
  return output;
}

// Other functions
Vector3 Vector3::rotate_around(Vector3 axis, double theta) const {

  Vector3 thisV = {x, y, z};
  Vector3 axisN = unit(axis);
  Vector3 parallelComp = axisN * dot(thisV, axisN);
  Vector3 tangentComp = thisV - parallelComp;

  if (norm2(tangentComp) > 0.0) {
    Vector3 basisX = unit(tangentComp);
    Vector3 basisY = cross(axisN, basisX);

    double tangentMag = norm(tangentComp);

    Vector3 rotatedV = tangentMag * (cos(theta) * basisX + sin(theta) * basisY);
    return rotatedV + parallelComp;
  } else {
    return parallelComp;
  }
}

}  // namespace geometrycentral
