#pragma once

#include <array>
#include <cmath>
#include <cstddef>

template <std::size_t N, typename T = float> struct Vec {
    std::array<T, N> v{};

    T&       operator[](std::size_t i) { return v[i]; }
    const T& operator[](std::size_t i) const { return v[i]; }

    Vec operator+(const Vec& rhs) const {
        Vec out;
        for (std::size_t i = 0; i < N; ++i)
            out[i] = v[i] + rhs[i];
        return out;
    }

    Vec operator-(const Vec& rhs) const {
        Vec out;
        for (std::size_t i = 0; i < N; ++i)
            out[i] = v[i] - rhs[i];
        return out;
    }

    Vec operator*(T s) const {
        Vec out;
        for (std::size_t i = 0; i < N; ++i)
            out[i] = v[i] * s;
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
        for (std::size_t i = 0; i < N; ++i)
            out[i] = v[i] / s;
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
        for (std::size_t i = 0; i < N; ++i)
            sum += v[i] * rhs[i];
        return sum;
    }

    T length() const { return std::sqrt(dot(*this)); }
};

template <size_t N, typename T> Vec<N, T> normalize(const Vec<N, T>& v) {
    T len2 = v.dot(v);
    if (len2 == T(0))
        return Vec<N, T>{};
    T         invLen = T(1) / std::sqrt(len2);
    Vec<N, T> out{};
    for (size_t i = 0; i < N; i++)
        out[i] = v[i] * invLen;
    return out;
}

template <std::size_t N, typename T> T length(const Vec<N, T>& v) {
    return std::sqrt(v.dot(v));
}

template <typename T> struct Vec<2, T> {
    T x{};
    T y{};

    T&       operator[](std::size_t i) { return (i == 0) ? x : y; }
    const T& operator[](std::size_t i) const { return (i == 0) ? x : y; }

    Vec operator+(const Vec& r) const { return {x + r.x, y + r.y}; }
    Vec operator-(const Vec& r) const { return {x - r.x, y - r.y}; }
    Vec operator*(T s) const { return {x * s, y * s}; }
    Vec operator*(const Vec& r) const { return {x * r.x, y * r.y}; }
    Vec operator/(T s) const { return {x / s, y / s}; }
    Vec operator/(const Vec& r) const { return {x / r.x, y / r.y}; }
    T   dot(const Vec& r) const { return (x * r.x) + (y * r.y); }
    T   length() const { return std::sqrt(dot(*this)); }
};

template <typename T> struct Vec<3, T> {
    T x{};
    T y{};
    T z{};

    // Direction constants
    static constexpr Vec up() { return {0, 1, 0}; }
    static constexpr Vec down() { return {0, -1, 0}; }
    static constexpr Vec left() { return {-1, 0, 0}; }
    static constexpr Vec right() { return {1, 0, 0}; }
    static constexpr Vec forward() { return {0, 0, 1}; }
    static constexpr Vec back() { return {0, 0, -1}; }

    T&       operator[](std::size_t i) { return (i == 0) ? x : (i == 1) ? y : z; }
    const T& operator[](std::size_t i) const { return (i == 0) ? x : (i == 1) ? y : z; }

    Vec operator+(const Vec& r) const { return {x + r.x, y + r.y, z + r.z}; }
    Vec operator-(const Vec& r) const { return {x - r.x, y - r.y, z - r.z}; }
    Vec operator*(T s) const { return {x * s, y * s, z * s}; }
    Vec operator*(const Vec& r) const { return {x * r.x, y * r.y, z * r.z}; }
    Vec operator/(T s) const { return {x / s, y / s, z / s}; }
    Vec operator/(const Vec& r) const { return {x / r.x, y / r.y, z / r.z}; }
    T   dot(const Vec& r) const { return (x * r.x) + (y * r.y) + (z * r.z); }
    T   length() const { return std::sqrt(dot(*this)); }
};

template <typename T> Vec<3, T> cross(const Vec<3, T>& a, const Vec<3, T>& b) {
    return {(a[1] * b[2]) - (a[2] * b[1]), (a[2] * b[0]) - (a[0] * b[2]), (a[0] * b[1]) - (a[1] * b[0])};
}

template <typename T> struct alignas(16) Vec<4, T> {
    T x{};
    T y{};
    T z{};
    T w{};

    T&       operator[](std::size_t i) { return (i == 0) ? x : (i == 1) ? y : (i == 2) ? z : w; }
    const T& operator[](std::size_t i) const { return (i == 0) ? x : (i == 1) ? y : (i == 2) ? z : w; }

    Vec operator+(const Vec& r) const { return {x + r.x, y + r.y, z + r.z, w + r.w}; }
    Vec operator-(const Vec& r) const { return {x - r.x, y - r.y, z - r.z, w - r.w}; }
    Vec operator*(T s) const { return {x * s, y * s, z * s, w * s}; }
    Vec operator*(const Vec& r) const { return {x * r.x, y * r.y, z * r.z, w * r.w}; }
    Vec operator/(T s) const { return {x / s, y / s, z / s, w / s}; }
    Vec operator/(const Vec& r) const { return {x / r.x, y / r.y, z / r.z, w / r.w}; }
    T   dot(const Vec& r) const { return (x * r.x) + (y * r.y) + (z * r.z) + (w * r.w); }
    T   length() const { return std::sqrt(dot(*this)); }
};

using Vector2 = Vec<2, float>;
using Vector3 = Vec<3, float>;
using Vector4 = Vec<4, float>;
