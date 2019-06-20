
namespace geometrycentral {

inline Vector2 Vector2::operator+(const Vector2& v) const { return Vector2{x + v.x, y + v.y}; }

inline Vector2 Vector2::operator-(const Vector2& v) const { return Vector2{x - v.x, y - v.y}; }

inline Vector2 Vector2::operator*(const Vector2& v) const { return Vector2{x * v.x - y * v.y, x * v.y + y * v.x}; }

inline Vector2 Vector2::operator/(const Vector2& v) const {
  double denom = v.x * v.x + v.y * v.y;
  return Vector2{x * v.x + y * v.y, y * v.x - x * v.y} / denom;
}

inline Vector2 Vector2::operator*(double s) const { return Vector2{x * s, y * s}; }

inline Vector2 Vector2::operator/(double s) const {
  const double r = 1. / s;
  return Vector2{x * r, y * r};
}

inline const Vector2 Vector2::operator-() const { return Vector2{-x, -y}; }

inline Vector2 operator*(const double s, const Vector2& v) { return Vector2{s * v.x, s * v.y}; }

inline Vector2& Vector2::operator+=(const Vector2& other) {
  x += other.x;
  y += other.y;
  return *this;
}

inline Vector2& Vector2::operator-=(const Vector2& other) {
  x -= other.x;
  y -= other.y;
  return *this;
}

inline Vector2& Vector2::operator*=(const Vector2& other) {
  Vector2 tmp = *this * other;
  *this = tmp;
  return *this;
}

inline Vector2& Vector2::operator/=(const Vector2& other) {
  Vector2 tmp = *this / other;
  *this = tmp;
  return *this;
}

inline Vector2& Vector2::operator*=(const double& s) {
  x *= s;
  y *= s;
  return *this;
}

inline Vector2& Vector2::operator/=(const double& s) {
  x /= s;
  y /= s;
  return *this;
}

inline bool Vector2::operator==(const Vector2& other) const { return x == other.x && y == other.y; }

inline bool Vector2::operator!=(const Vector2& other) const { return !(*this == other); }


inline Vector2::operator std::complex<double>() const {
  return std::complex<double>{x, y};
}

inline Vector2& Vector2::normalize() {
  double r = 1. / sqrt(x * x + y * y);
  x /= r;
  y /= r;
  return *this;
}

inline Vector2 unit(const Vector2& v) {
  double n = norm(v);
  return Vector2{v.x / n, v.y / n};
}

inline Vector2& Vector2::rotate(double theta) {
  double cosTh = std::cos(theta);
  double sinTh = std::sin(theta);
  *this = Vector2{cosTh * x + sinTh * y, -sinTh * x + cosTh * y};
  return *this;
}

inline Vector2& Vector2::pow(double p) {
  std::complex<double> c{x, y};
  c = std::pow(c, p);
  x = c.real();
  y = c.imag();
  return *this;
}

inline Vector2& Vector2::pow(Vector2 p) {
  std::complex<double> c{x, y};
  std::complex<double> pc{p.x, p.y};
  c = std::pow(c, pc);
  x = c.real();
  y = c.imag();
  return *this;
}

inline Vector2& Vector2::conj() {
  y *= -1.;
  return *this;
}

inline Vector2& Vector2::inv() {
  *this = Vector2{1., 0.} / *this;
  return *this;
}

inline double Vector2::arg() const { return std::atan2(y, x); }
inline double arg(const Vector2& v) { return std::atan2(v.y, v.x); }

inline double Vector2::norm() const { return std::sqrt(x * x + y * y); }
inline double norm(const Vector2& v) { return sqrt(v.x * v.x + v.y * v.y); }

inline double Vector2::norm2() const { return x * x + y * y; }
inline double norm2(const Vector2& v) { return v.x * v.x + v.y * v.y; }


inline double dot(const Vector2& u, const Vector2& v) { return u.x * v.x + u.y * v.y; }

inline double angle(const Vector2& u, const Vector2& v) { return acos(fmax(-1., fmin(1., dot(unit(u), unit(v))))); }

inline double cross(const Vector2& u, const Vector2& v) { return u.x * v.y - u.y * v.x; }
inline Vector3 cross3(const Vector2& u, const Vector2& v) { return Vector3{0., 0., u.x * v.y - u.y * v.x}; }
inline Vector2 clamp(const Vector2& val, const Vector2& low, const Vector2& high) {
  Vector2 rVal;
  for (int i = 0; i < 2; i++) {
    rVal[i] = clamp(val[i], low[i], high[i]);
  }
  return rVal;
}

inline bool Vector2::isFinite() const { return std::isfinite(x) && std::isfinite(y); }
inline bool isfinite(const Vector2& v) { return v.isFinite(); }
inline bool Vector2::isDefined() const { return (!std::isnan(x)) && (!std::isnan(y)); }
inline bool isDefined(const Vector2& v) { return v.isDefined(); }

inline Vector2 componentwiseMin(const Vector2& u, const Vector2& v) { return Vector2{fmin(u.x, v.x), fmin(u.y, v.y)}; }
inline Vector2 componentwiseMax(const Vector2& u, const Vector2& v) { return Vector2{fmax(u.x, v.x), fmax(u.y, v.y)}; }

inline std::ostream& operator<<(std::ostream& output, const Vector2& v) {
  output << "<" << v.x << ", " << v.y << ">";
  return output;
}

} // namespace geometrycentral
