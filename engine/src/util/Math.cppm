module;
#include <array>
#include <cmath>
#include <cstddef>
#include <numbers>

export module SharkEngine:Math;

export template<std::size_t N, typename T = float>
struct Vec {
    std::array<T, N> v{};

    T& operator[](std::size_t i)             { return v[i]; }
    const T& operator[](std::size_t i) const { return v[i]; }

    Vec operator+(const Vec& rhs) const {
        Vec out;
        for (std::size_t i = 0; i < N; ++i) out[i] = v[i] + rhs[i];
        return out;
    }

    Vec operator-(const Vec& rhs) const {
        Vec out;
        for (std::size_t i = 0; i < N; ++i) out[i] = v[i] - rhs[i];
        return out;
    }

    Vec operator*(T s) const {
        Vec out;
        for (std::size_t i = 0; i < N; ++i) out[i] = v[i] * s;
        return out;
    }

    Vec operator*(const Vec& rhs) const {
        Vec out{};
        for (std::size_t i = 0; i < N; ++i)
            out[i] = v[i] * rhs[i];
        return out;
    }

    Vec operator/(T s) const {
        Vec out{};
        for (std::size_t i = 0; i < N; ++i) out[i] = v[i] / s;
        return out;
    }

    Vec operator/(const Vec& rhs) const {
        Vec out{};
        for (std::size_t i = 0; i < N; ++i)
            out[i] = v[i] / rhs[i];
        return out;
    }

    T dot(const Vec& rhs) const {
        T sum = 0;
        for (std::size_t i = 0; i < N; ++i) sum += v[i] * rhs[i];
        return sum;
    }
};

export template<size_t N, typename T>
Vec<N,T> normalize(const Vec<N,T>& v) {
    T len2 = v.dot(v);
    if (len2 == T(0))
        return Vec<N,T>{};
    T invLen = T(1) / std::sqrt(len2);
    Vec<N,T> out{};
    for (size_t i = 0; i < N; i++)
        out[i] = v[i] * invLen;
    return out;
}

template<typename T>
struct Vec<2, T> {
    T x{};
    T y{};

    T& operator[](std::size_t i)             { return (i == 0) ? x : y; }
    const T& operator[](std::size_t i) const { return (i == 0) ? x : y; }

    Vec operator+(const Vec& r) const { return {x + r.x, y + r.y}; }
    Vec operator-(const Vec& r) const { return {x - r.x, y - r.y}; }
    Vec operator*(T s) const          { return {x * s, y * s}; }
    Vec operator*(const Vec& r) const { return {x*r.x, y*r.y}; }
    Vec operator/(T s) const          { return {x / s, y / s}; }
    Vec operator/(const Vec& r) const { return {x/r.x, y/r.y}; }
    T dot(const Vec& r) const         { return x*r.x + y*r.y; }
};

template<typename T>
struct Vec<3, T> {
    T x{};
    T y{};
    T z{};

    // Direction constants
    static const Vec up()       { return { 0,  1,  0}; }
    static const Vec down()     { return { 0, -1,  0}; }
    static const Vec left()     { return {-1,  0,  0}; }
    static const Vec right()    { return { 1,  0,  0}; }
    static const Vec forward()  { return { 0,  0,  1}; }
    static const Vec back()     { return { 0,  0, -1}; }

    T& operator[](std::size_t i)             { return (i == 0) ? x : (i == 1) ? y : z; }
    const T& operator[](std::size_t i) const { return (i == 0) ? x : (i == 1) ? y : z; }

    Vec operator+(const Vec& r) const { return {x + r.x, y + r.y, z + r.z}; }
    Vec operator-(const Vec& r) const { return {x - r.x, y - r.y, z - r.z}; }
    Vec operator*(T s) const          { return {x * s, y * s, z * s}; }
    Vec operator*(const Vec& r) const { return {x*r.x, y*r.y, z*r.z}; }
    Vec operator/(T s) const          { return {x / s, y / s, z / s}; }
    Vec operator/(const Vec& r) const { return {x/r.x, y/r.y, z/r.z}; }
    T dot(const Vec& r) const         { return x*r.x + y*r.y + z*r.z; }
};

template<typename T>
Vec<3,T> cross(const Vec<3,T>& a, const Vec<3,T>& b) {
    return {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };
}

template<typename T>
struct Vec<4, T> {
    T x{};
    T y{};
    T z{};
    T w{};

    T& operator[](std::size_t i)             { return (i == 0) ? x : (i == 1) ? y : (i == 2) ? z : w; }
    const T& operator[](std::size_t i) const { return (i == 0) ? x : (i == 1) ? y : (i == 2) ? z : w; }

    Vec operator+(const Vec& r) const { return {x+r.x, y+r.y, z+r.z, w+r.w}; }
    Vec operator-(const Vec& r) const { return {x-r.x, y-r.y, z-r.z, w-r.w}; }
    Vec operator*(T s) const          { return {x*s, y*s, z*s, w*s}; }
    Vec operator*(const Vec& r) const { return {x*r.x, y*r.y, z*r.z, w*r.w}; }
    Vec operator/(T s) const          { return {x / s, y / s, z / s, w / s}; }
    Vec operator/(const Vec& r) const { return {x/r.x, y/r.y, z/r.z, w/r.w}; }
    T dot(const Vec& r) const         { return x*r.x + y*r.y + z*r.z + w*r.w; }
};

export using Vector2 = Vec<2,float>;
export using Vector3 = Vec<3,float>;
export using Vector4 = Vec<4,float>;

export struct Matrix4 {
    float m[16]{}; // column-major

    static Matrix4 identity() {
        Matrix4 r{};
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
        return r;
    }

    const float* data() const { return m; }

    Matrix4 operator*(const Matrix4& rhs) const {
        Matrix4 result{};
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                result.m[col * 4 + row] =
                    m[0 * 4 + row] * rhs.m[col * 4 + 0] +
                    m[1 * 4 + row] * rhs.m[col * 4 + 1] +
                    m[2 * 4 + row] * rhs.m[col * 4 + 2] +
                    m[3 * 4 + row] * rhs.m[col * 4 + 3];
            }
        }
        return result;
    }

    static Matrix4 perspective(float fovDeg, float aspect, float nearZ, float farZ) {
        Matrix4 r{};
        float f = 1.0f / std::tan((fovDeg * 0.5f) * std::numbers::pi_v<float> / 180.0f);

        r.m[0]  = f / aspect;
        r.m[5]  = f;
        r.m[10] = (farZ + nearZ) / (nearZ - farZ);
        r.m[11] = -1.0f;
        r.m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
        return r;
    }

    static Matrix4 frustrum(float left, float right, float bottom, float top, float nearZ, float farZ) {
        Matrix4 r{};
        r.m[0]  = (2.0f * nearZ) / (right - left);
        r.m[5]  = (2.0f * nearZ) / (top - bottom);
        r.m[8]  = (right + left) / (right - left);
        r.m[9]  = (top + bottom) / (top - bottom);
        r.m[10] = -(farZ + nearZ) / (farZ - nearZ);
        r.m[11] = -1.0f;
        r.m[14] = -(2.0f * farZ * nearZ) / (farZ - nearZ);
        return r;
    }

    static Matrix4 ortho(float left, float right, float bottom, float top, float nearZ, float farZ) {
        Matrix4 r{};
        r.m[0]  = 2.0f / (right - left);
        r.m[5]  = 2.0f / (top - bottom);
        r.m[10] = -2.0f / (farZ - nearZ);
        r.m[12] = -(right + left) / (right - left);
        r.m[13] = -(top + bottom) / (top - bottom);
        r.m[14] = -(farZ + nearZ) / (farZ - nearZ);
        r.m[15] = 1.0f;
        return r;
    }

    static Matrix4 translate(const Vector3& t) {
        Matrix4 r = identity();
        r.m[12] = t.x;
        r.m[13] = t.y;
        r.m[14] = t.z;
        return r;
    }

    static Matrix4 rotateX(float angleDeg) {
        Matrix4 r = identity();
        float rad = angleDeg * std::numbers::pi_v<float> / 180.0f;
        float c = std::cos(rad);
        float s = std::sin(rad);
        r.m[5] = c;
        r.m[6] = s;
        r.m[9] = -s;
        r.m[10] = c;
        return r;
    }

    static Matrix4 rotateY(float angleDeg) {
        Matrix4 r = identity();
        float rad = angleDeg * std::numbers::pi_v<float> / 180.0f;
        float c = std::cos(rad);
        float s = std::sin(rad);
        r.m[0] = c;
        r.m[2] = -s;
        r.m[8] = s;
        r.m[10] = c;
        return r;
    }

    static Matrix4 rotateZ(float angleDeg) {
        Matrix4 r = identity();
        float rad = angleDeg * std::numbers::pi_v<float> / 180.0f;
        float c = std::cos(rad);
        float s = std::sin(rad);
        r.m[0] = c;
        r.m[1] = s;
        r.m[4] = -s;
        r.m[5] = c;
        return r;
    }

    static Matrix4 scale(const Vector3& s) {
        Matrix4 r = identity();
        r.m[0] = s.x;
        r.m[5] = s.y;
        r.m[10] = s.z;
        return r;
    }

    static Vector3 normalize(const Vector3& v) {
        float l = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
        return { v.x/l, v.y/l, v.z/l };
    }

    static Vector3 cross(const Vector3& a, const Vector3& b) {
        return {
            a.y*b.z - a.z*b.y,
            a.z*b.x - a.x*b.z,
            a.x*b.y - a.y*b.x
        };
    }

    static Matrix4 lookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
        Vector3 f = normalize({ target.x - eye.x, target.y - eye.y, target.z - eye.z });
        Vector3 s = normalize(cross(f, up));
        Vector3 u = cross(s, f);

        Matrix4 r = identity();

        r.m[0] = s.x; r.m[4] = s.y; r.m[8]  = s.z;
        r.m[1] = u.x; r.m[5] = u.y; r.m[9]  = u.z;
        r.m[2] = -f.x; r.m[6] = -f.y; r.m[10] = -f.z;

        r.m[12] = -(s.x*eye.x + s.y*eye.y + s.z*eye.z);
        r.m[13] = -(u.x*eye.x + u.y*eye.y + u.z*eye.z);
        r.m[14] =  (f.x*eye.x + f.y*eye.y + f.z*eye.z);

        return r;
    }
};
