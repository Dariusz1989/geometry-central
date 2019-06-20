#pragma once

#include "geometrycentral/utilities/vector3.h"
#include <cmath>
#include <iostream>

namespace geometrycentral {

// Note: this class avoids any constructors so that it is a POD type
struct Vector2 {
  double x, y;

  static Vector2 zero() { return Vector2{0., 0.}; }
  static Vector2 constant(double c) { return Vector2{c, c}; }
  static Vector2 fromAngle(double theta) { return Vector2{std::cos(theta), std::sin(theta)}; }
  static Vector2 infinity() {
    const double inf = std::numeric_limits<double>::infinity();
    return Vector2{inf, inf};
  }

  static Vector2 undefined() {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    return Vector2{nan, nan};
  }


  // Access-by-index
  double& operator[](int index) { return (&x)[index]; }
  double operator[](int index) const { return (&x)[index]; };

  // Overloaded operators
  // Multiplication & division are in the sense of complex numbers
  Vector2 operator+(const Vector2& v) const;
  Vector2 operator-(const Vector2& v) const;
  Vector2 operator*(const Vector2& v) const;
  Vector2 operator/(const Vector2& v) const;
  Vector2 operator*(double s) const;
  Vector2 operator/(double s) const;
  Vector2& operator+=(const Vector2& other);
  Vector2& operator-=(const Vector2& other);
  Vector2& operator*=(const Vector2& other);
  Vector2& operator/=(const Vector2& other);
  Vector2& operator*=(const double& s);
  Vector2& operator/=(const double& s);
  bool operator==(const Vector2& v) const;
  bool operator!=(const Vector2& v) const;
  const Vector2 operator-() const;

  // Conversion to std::complex
  operator std::complex<double>() const;

  // Notice that all of these functions modify the vector in-place (but return a reference for chaining)
  // The non-member functions below return a new vector

  Vector2& normalize();
  Vector2& rotate(double theta);

  // Complex functions
  Vector2& pow(double p);  // complex power
  Vector2& pow(Vector2 p); // complex to complex power
  Vector2& conj();
  Vector2& inv();

  double arg() const;
  double norm() const;
  double norm2() const;

  bool isFinite() const;
  bool isDefined() const;
};

Vector2 operator*(const double s, const Vector2& v);

::std::ostream& operator<<(std::ostream& output, const Vector2& v);

// Notice that all of these functions return a new vector when applicable.
// The member functions above modify in place

double arg(const Vector2& v);
double norm(const Vector2& v);
double norm2(const Vector2& v);

double angle(const Vector2& u, const Vector2& v);
double dot(const Vector2& u, const Vector2& v);
double cross(const Vector2& u, const Vector2& v);
Vector3 cross3(const Vector2& u, const Vector2& v); // assumes arguments are in x-y plane

Vector2 unit(const Vector2& v);
Vector2 clamp(const Vector2& val, const Vector2& low, const Vector2& high);

bool isfinite(const Vector2& u); // break camel case rule to match std
bool isDefined(const Vector2& u);
Vector2 componentwiseMin(const Vector2& u, const Vector2& v);
Vector2 componentwiseMax(const Vector2& u, const Vector2& v);

} // namespace geometrycentral

#include "geometrycentral/utilities/vector2.ipp"
