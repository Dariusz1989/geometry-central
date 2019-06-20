#include <geometrycentral/utilities/utilities.h>

#include <cmath>
#include <iostream>

namespace geometrycentral {

inline Vector3& Vector3::normalize() {
  double r = 1. / sqrt(x * x + y * y + z * z);
  x *= r;
  y *= r;
  z *= r;
  return *this;
}

inline Vector3 Vector3::operator+(const Vector3& v) const { return Vector3{x + v.x, y + v.y, z + v.z}; }

inline Vector3 Vector3::operator-(const Vector3& v) const { return Vector3{x - v.x, y - v.y, z - v.z}; }

inline Vector3 Vector3::operator*(double s) const { return Vector3{x * s, y * s, z * s}; }

inline Vector3 Vector3::operator/(double s) const {
  const double r = 1. / s;
  return Vector3{x * r, y * r, z * r};
}

inline const Vector3 Vector3::operator-() const { return Vector3{-x, -y, -z}; }

inline Vector3 operator*(const double s, const Vector3& v) { return Vector3{s * v.x, s * v.y, s * v.z}; }

inline Vector3& Vector3::operator+=(const Vector3& other) {
  x += other.x;
  y += other.y;
  z += other.z;
  return *this;
}

inline Vector3& Vector3::operator-=(const Vector3& other) {
  x -= other.x;
  y -= other.y;
  z -= other.z;
  return *this;
}

inline Vector3& Vector3::operator*=(const double& s) {
  x *= s;
  y *= s;
  z *= s;
  return *this;
}

inline Vector3& Vector3::operator/=(const double& s) {
  x /= s;
  y /= s;
  z /= s;
  return *this;
}

inline bool Vector3::operator==(const Vector3& other) const { return x == other.x && y == other.y && z == other.z; }

inline bool Vector3::operator!=(const Vector3& other) const { return !(*this == other); }

inline double norm(const Vector3& v) { return sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

inline double norm2(const Vector3& v) { return v.x * v.x + v.y * v.y + v.z * v.z; }

inline Vector3 unit(const Vector3& v) {
  double n = norm(v);
  return Vector3{v.x / n, v.y / n, v.z / n};
}

inline Vector3 cross(const Vector3& u, const Vector3& v) {
  double x = u.y * v.z - u.z * v.y;
  double y = u.z * v.x - u.x * v.z;
  double z = u.x * v.y - u.y * v.x;
  return Vector3{x, y, z};
}

inline double dot(const Vector3& u, const Vector3& v) { return u.x * v.x + u.y * v.y + u.z * v.z; }

inline double angle(const Vector3& u, const Vector3& v) { return acos(fmax(-1., fmin(1., dot(unit(u), unit(v))))); }

inline double angleInPlane(const Vector3& u, const Vector3& v, const Vector3& normal) {
  // Put u in plane with the normal
  Vector3 N = unit(normal);
  Vector3 uPlane = unit(u - dot(u, N) * N);
  Vector3 basisY = unit(cross(normal, uPlane));

  double xComp = dot(v, uPlane);
  double yComp = dot(v, basisY);

  return ::std::atan2(yComp, xComp);
}

inline bool Vector3::isFinite() const { return ::std::isfinite(x) && ::std::isfinite(y) && ::std::isfinite(z); }
inline bool isfinite(const Vector3& v) { return v.isFinite(); }

inline bool Vector3::isDefined() const { return (!::std::isnan(x)) && (!::std::isnan(y)) && (!::std::isnan(z)); }
inline bool isDefined(const Vector3& v) { return v.isDefined(); }

inline Vector3 componentwiseMin(const Vector3& u, const Vector3& v) {
  return Vector3{fmin(u.x, v.x), fmin(u.y, v.y), fmin(u.z, v.z)};
}

inline Vector3 componentwiseMax(const Vector3& u, const Vector3& v) {
  return Vector3{fmax(u.x, v.x), fmax(u.y, v.y), fmax(u.z, v.z)};
}

inline Vector3& Vector3::rotate_around(Vector3 axis, double theta) {
  Vector3 thisV = {x, y, z};
  Vector3 axisN = unit(axis);
  Vector3 parallelComp = axisN * dot(thisV, axisN);
  Vector3 tangentComp = thisV - parallelComp;

  if (norm2(tangentComp) > 0.0) {
    Vector3 basisX = unit(tangentComp);
    Vector3 basisY = cross(axisN, basisX);

    double tangentMag = norm(tangentComp);

    Vector3 rotatedV = tangentMag * (cos(theta) * basisX + sin(theta) * basisY);
    *this = rotatedV + parallelComp;
  } else {
    *this = parallelComp;
  }
  return *this;
}

inline Vector3& Vector3::removeComponent(const Vector3& unitDir) {
  *this -= unitDir * dot(unitDir, *this);
  return *this;
}

inline std::ostream& operator<<(std::ostream& output, const Vector3& v) {
  output << "<" << v.x << ", " << v.y << ", " << v.z << ">";
  return output;
}

} // namespace geometrycentral

namespace std {
inline std::size_t std::hash<geometrycentral::Vector3>::operator()(const geometrycentral::Vector3& v) const {
  return std::hash<double>{}(v.x) ^ (std::hash<double>{}(v.y) + (std::hash<double>{}(v.y) << 2)) ^
         (std::hash<double>{}(v.z) + (std::hash<double>{}(v.z) << 4));
}

} // namespace std
