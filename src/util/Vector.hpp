#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <type_traits>

#include "util/SIMD.hpp"

template <std::size_t N, typename T = float> struct Vec {
    std::array<T, N> v{};

    [[nodiscard]] constexpr T&       operator[](std::size_t i) noexcept { return v[i]; }
    [[nodiscard]] constexpr const T& operator[](std::size_t i) const noexcept { return v[i]; }

    [[nodiscard]] constexpr Vec operator+() const noexcept { return *this; }
    [[nodiscard]] constexpr Vec operator-() const noexcept {
        Vec out{};
        for (std::size_t i = 0; i < N; ++i)
            out[i] = -v[i];
        return out;
    }

    [[nodiscard]] constexpr Vec operator+(const Vec& rhs) const noexcept {
        Vec out{};
        for (std::size_t i = 0; i < N; ++i)
            out[i] = v[i] + rhs[i];
        return out;
    }

    [[nodiscard]] constexpr Vec operator-(const Vec& rhs) const noexcept {
        Vec out{};
        for (std::size_t i = 0; i < N; ++i)
            out[i] = v[i] - rhs[i];
        return out;
    }

    [[nodiscard]] constexpr Vec operator*(T s) const noexcept {
        Vec out{};
        for (std::size_t i = 0; i < N; ++i)
            out[i] = v[i] * s;
        return out;
    }

    [[nodiscard]] constexpr Vec operator*(const Vec& rhs) const noexcept {
        Vec out{};
        for (std::size_t i = 0; i < N; ++i)
            out[i] = v[i] * rhs[i];
        return out;
    }

    [[nodiscard]] constexpr Vec operator/(T s) const noexcept {
        Vec out{};
        for (std::size_t i = 0; i < N; ++i)
            out[i] = v[i] / s;
        return out;
    }

    [[nodiscard]] constexpr Vec operator/(const Vec& rhs) const noexcept {
        Vec out{};
        for (std::size_t i = 0; i < N; ++i)
            out[i] = v[i] / rhs[i];
        return out;
    }

    constexpr Vec& operator+=(const Vec& rhs) noexcept {
        for (std::size_t i = 0; i < N; ++i)
            v[i] += rhs[i];
        return *this;
    }

    constexpr Vec& operator-=(const Vec& rhs) noexcept {
        for (std::size_t i = 0; i < N; ++i)
            v[i] -= rhs[i];
        return *this;
    }

    constexpr Vec& operator*=(T s) noexcept {
        for (std::size_t i = 0; i < N; ++i)
            v[i] *= s;
        return *this;
    }

    constexpr Vec& operator*=(const Vec& rhs) noexcept {
        for (std::size_t i = 0; i < N; ++i)
            v[i] *= rhs[i];
        return *this;
    }

    constexpr Vec& operator/=(T s) noexcept {
        for (std::size_t i = 0; i < N; ++i)
            v[i] /= s;
        return *this;
    }

    constexpr Vec& operator/=(const Vec& rhs) noexcept {
        for (std::size_t i = 0; i < N; ++i)
            v[i] /= rhs[i];
        return *this;
    }

    [[nodiscard]] constexpr T dot(const Vec& rhs) const noexcept {
        T sum = 0;
        for (std::size_t i = 0; i < N; ++i)
            sum += v[i] * rhs[i];
        return sum;
    }

    [[nodiscard]] constexpr T lengthSq() const noexcept { return dot(*this); }
    [[nodiscard]] T           length() const noexcept { return std::sqrt(lengthSq()); }
};

template <typename T> struct Vec<2, T> {
    T x{};
    T y{};

    [[nodiscard]] constexpr T&       operator[](std::size_t i) noexcept { return (i == 0) ? x : y; }
    [[nodiscard]] constexpr const T& operator[](std::size_t i) const noexcept { return (i == 0) ? x : y; }

    [[nodiscard]] constexpr Vec operator+() const noexcept { return *this; }
    [[nodiscard]] constexpr Vec operator-() const noexcept { return {-x, -y}; }

    [[nodiscard]] constexpr Vec operator+(const Vec& r) const noexcept { return {x + r.x, y + r.y}; }
    [[nodiscard]] constexpr Vec operator-(const Vec& r) const noexcept { return {x - r.x, y - r.y}; }
    [[nodiscard]] constexpr Vec operator*(T s) const noexcept { return {x * s, y * s}; }
    [[nodiscard]] constexpr Vec operator*(const Vec& r) const noexcept { return {x * r.x, y * r.y}; }
    [[nodiscard]] constexpr Vec operator/(T s) const noexcept { return {x / s, y / s}; }
    [[nodiscard]] constexpr Vec operator/(const Vec& r) const noexcept { return {x / r.x, y / r.y}; }

    constexpr Vec& operator+=(const Vec& r) noexcept {
        x += r.x;
        y += r.y;
        return *this;
    }
    constexpr Vec& operator-=(const Vec& r) noexcept {
        x -= r.x;
        y -= r.y;
        return *this;
    }
    constexpr Vec& operator*=(T s) noexcept {
        x *= s;
        y *= s;
        return *this;
    }
    constexpr Vec& operator*=(const Vec& r) noexcept {
        x *= r.x;
        y *= r.y;
        return *this;
    }
    constexpr Vec& operator/=(T s) noexcept {
        x /= s;
        y /= s;
        return *this;
    }
    constexpr Vec& operator/=(const Vec& r) noexcept {
        x /= r.x;
        y /= r.y;
        return *this;
    }

    [[nodiscard]] constexpr T dot(const Vec& r) const noexcept { return (x * r.x) + (y * r.y); }
    [[nodiscard]] constexpr T lengthSq() const noexcept { return dot(*this); }
    [[nodiscard]] T           length() const noexcept { return std::sqrt(lengthSq()); }
};

template <typename T> struct Vec<3, T> {
    T x{};
    T y{};
    T z{};

    static constexpr Vec up() noexcept { return {0, 1, 0}; }
    static constexpr Vec down() noexcept { return {0, -1, 0}; }
    static constexpr Vec left() noexcept { return {-1, 0, 0}; }
    static constexpr Vec right() noexcept { return {1, 0, 0}; }
    static constexpr Vec forward() noexcept { return {0, 0, 1}; }
    static constexpr Vec back() noexcept { return {0, 0, -1}; }

    [[nodiscard]] constexpr T&       operator[](std::size_t i) noexcept { return (i == 0) ? x : (i == 1) ? y : z; }
    [[nodiscard]] constexpr const T& operator[](std::size_t i) const noexcept {
        return (i == 0) ? x : (i == 1) ? y : z;
    }

    [[nodiscard]] constexpr Vec operator+() const noexcept { return *this; }
    [[nodiscard]] constexpr Vec operator-() const noexcept { return {-x, -y, -z}; }

    [[nodiscard]] constexpr Vec operator+(const Vec& r) const noexcept { return {x + r.x, y + r.y, z + r.z}; }
    [[nodiscard]] constexpr Vec operator-(const Vec& r) const noexcept { return {x - r.x, y - r.y, z - r.z}; }
    [[nodiscard]] constexpr Vec operator*(T s) const noexcept { return {x * s, y * s, z * s}; }
    [[nodiscard]] constexpr Vec operator*(const Vec& r) const noexcept { return {x * r.x, y * r.y, z * r.z}; }
    [[nodiscard]] constexpr Vec operator/(T s) const noexcept { return {x / s, y / s, z / s}; }
    [[nodiscard]] constexpr Vec operator/(const Vec& r) const noexcept { return {x / r.x, y / r.y, z / r.z}; }

    constexpr Vec& operator+=(const Vec& r) noexcept {
        x += r.x;
        y += r.y;
        z += r.z;
        return *this;
    }
    constexpr Vec& operator-=(const Vec& r) noexcept {
        x -= r.x;
        y -= r.y;
        z -= r.z;
        return *this;
    }
    constexpr Vec& operator*=(T s) noexcept {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    constexpr Vec& operator*=(const Vec& r) noexcept {
        x *= r.x;
        y *= r.y;
        z *= r.z;
        return *this;
    }
    constexpr Vec& operator/=(T s) noexcept {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }
    constexpr Vec& operator/=(const Vec& r) noexcept {
        x /= r.x;
        y /= r.y;
        z /= r.z;
        return *this;
    }

    [[nodiscard]] constexpr T dot(const Vec& r) const noexcept { return (x * r.x) + (y * r.y) + (z * r.z); }
    [[nodiscard]] constexpr T lengthSq() const noexcept { return dot(*this); }
    [[nodiscard]] T           length() const noexcept { return std::sqrt(lengthSq()); }
};

// SIMD-backed Vector3: padded to 16 bytes so it loads straight into a float4.
template <> struct alignas(16) Vec<3, float> {
    float x{};
    float y{};
    float z{};
    float pad_{0.0f}; // must stay 0; ops that could break that (division) go through fromSimd/zero_w

    static constexpr Vec up() noexcept { return {0.0f, 1.0f, 0.0f}; }
    static constexpr Vec down() noexcept { return {0.0f, -1.0f, 0.0f}; }
    static constexpr Vec left() noexcept { return {-1.0f, 0.0f, 0.0f}; }
    static constexpr Vec right() noexcept { return {1.0f, 0.0f, 0.0f}; }
    static constexpr Vec forward() noexcept { return {0.0f, 0.0f, 1.0f}; }
    static constexpr Vec back() noexcept { return {0.0f, 0.0f, -1.0f}; }

    [[nodiscard]] shark::simd::float4 toSimd() const noexcept { return shark::simd::load4_aligned(&x); }

    [[nodiscard]] static Vec fromSimdFast(const shark::simd::float4& v) noexcept {
        Vec out;
        shark::simd::store4_aligned(&out.x, v);
        return out;
    }

    [[nodiscard]] static Vec fromSimd(const shark::simd::float4& v) noexcept {
        Vec out;
        shark::simd::store4_aligned(&out.x, shark::simd::zero_w(v));
        return out;
    }

    [[nodiscard]] constexpr float&       operator[](std::size_t i) noexcept { return (i == 0) ? x : (i == 1) ? y : z; }
    [[nodiscard]] constexpr const float& operator[](std::size_t i) const noexcept {
        return (i == 0) ? x : (i == 1) ? y : z;
    }

    [[nodiscard]] constexpr Vec operator+() const noexcept { return *this; }
    [[nodiscard]] constexpr Vec operator-() const noexcept {
        if consteval {
            return {-x, -y, -z};
        }
        return fromSimdFast(-toSimd());
    }

    [[nodiscard]] constexpr Vec operator+(const Vec& r) const noexcept {
        if consteval {
            return {x + r.x, y + r.y, z + r.z};
        }
        return fromSimdFast(toSimd() + r.toSimd());
    }

    [[nodiscard]] constexpr Vec operator-(const Vec& r) const noexcept {
        if consteval {
            return {x - r.x, y - r.y, z - r.z};
        }
        return fromSimdFast(toSimd() - r.toSimd());
    }

    [[nodiscard]] constexpr Vec operator*(float s) const noexcept {
        if consteval {
            return {x * s, y * s, z * s};
        }
        return fromSimdFast(toSimd() * s);
    }

    [[nodiscard]] constexpr Vec operator*(const Vec& r) const noexcept {
        if consteval {
            return {x * r.x, y * r.y, z * r.z};
        }
        return fromSimdFast(toSimd() * r.toSimd());
    }

    [[nodiscard]] constexpr Vec operator/(float s) const noexcept {
        if consteval {
            return {x / s, y / s, z / s};
        }
        return fromSimd(toSimd() * (1.0f / s));
    }

    [[nodiscard]] constexpr Vec operator/(const Vec& r) const noexcept {
        if consteval {
            return {x / r.x, y / r.y, z / r.z};
        }
        return fromSimd(toSimd() / r.toSimd());
    }

    constexpr Vec& operator+=(const Vec& r) noexcept {
        if consteval {
            x += r.x;
            y += r.y;
            z += r.z;
            return *this;
        }
        shark::simd::store4_aligned(&x, toSimd() + r.toSimd());
        return *this;
    }

    constexpr Vec& operator-=(const Vec& r) noexcept {
        if consteval {
            x -= r.x;
            y -= r.y;
            z -= r.z;
            return *this;
        }
        shark::simd::store4_aligned(&x, toSimd() - r.toSimd());
        return *this;
    }

    constexpr Vec& operator*=(float s) noexcept {
        if consteval {
            x *= s;
            y *= s;
            z *= s;
            return *this;
        }
        shark::simd::store4_aligned(&x, toSimd() * s);
        return *this;
    }

    constexpr Vec& operator*=(const Vec& r) noexcept {
        if consteval {
            x *= r.x;
            y *= r.y;
            z *= r.z;
            return *this;
        }
        shark::simd::store4_aligned(&x, toSimd() * r.toSimd());
        return *this;
    }

    constexpr Vec& operator/=(float s) noexcept {
        if consteval {
            x /= s;
            y /= s;
            z /= s;
            return *this;
        }
        shark::simd::store4_aligned(&x, shark::simd::zero_w(toSimd() * (1.0f / s)));
        return *this;
    }

    constexpr Vec& operator/=(const Vec& r) noexcept {
        if consteval {
            x /= r.x;
            y /= r.y;
            z /= r.z;
            return *this;
        }
        shark::simd::store4_aligned(&x, shark::simd::zero_w(toSimd() / r.toSimd()));
        return *this;
    }

    [[nodiscard]] constexpr float dot(const Vec& r) const noexcept {
        if consteval {
            return (x * r.x) + (y * r.y) + (z * r.z);
        }
        return shark::simd::dot3(toSimd(), r.toSimd());
    }

    [[nodiscard]] constexpr float lengthSq() const noexcept { return dot(*this); }
    [[nodiscard]] float           length() const noexcept { return std::sqrt(lengthSq()); }

    [[nodiscard]] Vec normalized() const noexcept {
        const float len2 = lengthSq();
        if (len2 == 0.0f)
            return Vec{};
        return (*this) * (1.0f / std::sqrt(len2));
    }
};

template <typename T> struct alignas(16) Vec<4, T> {
    T x{};
    T y{};
    T z{};
    T w{};

    [[nodiscard]] constexpr T& operator[](std::size_t i) noexcept {
        return (i == 0) ? x : (i == 1) ? y : (i == 2) ? z : w;
    }
    [[nodiscard]] constexpr const T& operator[](std::size_t i) const noexcept {
        return (i == 0) ? x : (i == 1) ? y : (i == 2) ? z : w;
    }

    [[nodiscard]] constexpr Vec operator+() const noexcept { return *this; }
    [[nodiscard]] constexpr Vec operator-() const noexcept { return {-x, -y, -z, -w}; }

    [[nodiscard]] constexpr Vec operator+(const Vec& r) const noexcept { return {x + r.x, y + r.y, z + r.z, w + r.w}; }
    [[nodiscard]] constexpr Vec operator-(const Vec& r) const noexcept { return {x - r.x, y - r.y, z - r.z, w - r.w}; }
    [[nodiscard]] constexpr Vec operator*(T s) const noexcept { return {x * s, y * s, z * s, w * s}; }
    [[nodiscard]] constexpr Vec operator*(const Vec& r) const noexcept { return {x * r.x, y * r.y, z * r.z, w * r.w}; }
    [[nodiscard]] constexpr Vec operator/(T s) const noexcept { return {x / s, y / s, z / s, w / s}; }
    [[nodiscard]] constexpr Vec operator/(const Vec& r) const noexcept { return {x / r.x, y / r.y, z / r.z, w / r.w}; }

    constexpr Vec& operator+=(const Vec& r) noexcept {
        x += r.x;
        y += r.y;
        z += r.z;
        w += r.w;
        return *this;
    }
    constexpr Vec& operator-=(const Vec& r) noexcept {
        x -= r.x;
        y -= r.y;
        z -= r.z;
        w -= r.w;
        return *this;
    }
    constexpr Vec& operator*=(T s) noexcept {
        x *= s;
        y *= s;
        z *= s;
        w *= s;
        return *this;
    }
    constexpr Vec& operator*=(const Vec& r) noexcept {
        x *= r.x;
        y *= r.y;
        z *= r.z;
        w *= r.w;
        return *this;
    }
    constexpr Vec& operator/=(T s) noexcept {
        x /= s;
        y /= s;
        z /= s;
        w /= s;
        return *this;
    }
    constexpr Vec& operator/=(const Vec& r) noexcept {
        x /= r.x;
        y /= r.y;
        z /= r.z;
        w /= r.w;
        return *this;
    }

    [[nodiscard]] constexpr T dot(const Vec& r) const noexcept { return (x * r.x) + (y * r.y) + (z * r.z) + (w * r.w); }
    [[nodiscard]] constexpr T lengthSq() const noexcept { return dot(*this); }
    [[nodiscard]] T           length() const noexcept { return std::sqrt(lengthSq()); }
};

// SIMD-backed Vector4.
template <> struct alignas(16) Vec<4, float> {
    float x{};
    float y{};
    float z{};
    float w{};

    [[nodiscard]] shark::simd::float4 toSimd() const noexcept { return shark::simd::load4_aligned(&x); }

    [[nodiscard]] static Vec fromSimd(const shark::simd::float4& v) noexcept {
        Vec out;
        shark::simd::store4_aligned(&out.x, v);
        return out;
    }

    [[nodiscard]] constexpr float& operator[](std::size_t i) noexcept {
        return (i == 0) ? x : (i == 1) ? y : (i == 2) ? z : w;
    }
    [[nodiscard]] constexpr const float& operator[](std::size_t i) const noexcept {
        return (i == 0) ? x : (i == 1) ? y : (i == 2) ? z : w;
    }

    [[nodiscard]] constexpr Vec operator+() const noexcept { return *this; }
    [[nodiscard]] constexpr Vec operator-() const noexcept {
        if consteval {
            return {-x, -y, -z, -w};
        }
        return fromSimd(-toSimd());
    }

    [[nodiscard]] constexpr Vec operator+(const Vec& r) const noexcept {
        if consteval {
            return {x + r.x, y + r.y, z + r.z, w + r.w};
        }
        return fromSimd(toSimd() + r.toSimd());
    }

    [[nodiscard]] constexpr Vec operator-(const Vec& r) const noexcept {
        if consteval {
            return {x - r.x, y - r.y, z - r.z, w - r.w};
        }
        return fromSimd(toSimd() - r.toSimd());
    }

    [[nodiscard]] constexpr Vec operator*(float s) const noexcept {
        if consteval {
            return {x * s, y * s, z * s, w * s};
        }
        return fromSimd(toSimd() * s);
    }

    [[nodiscard]] constexpr Vec operator*(const Vec& r) const noexcept {
        if consteval {
            return {x * r.x, y * r.y, z * r.z, w * r.w};
        }
        return fromSimd(toSimd() * r.toSimd());
    }

    [[nodiscard]] constexpr Vec operator/(float s) const noexcept {
        if consteval {
            return {x / s, y / s, z / s, w / s};
        }
        return fromSimd(toSimd() * (1.0f / s));
    }

    [[nodiscard]] constexpr Vec operator/(const Vec& r) const noexcept {
        if consteval {
            return {x / r.x, y / r.y, z / r.z, w / r.w};
        }
        return fromSimd(toSimd() / r.toSimd());
    }

    constexpr Vec& operator+=(const Vec& r) noexcept {
        if consteval {
            x += r.x;
            y += r.y;
            z += r.z;
            w += r.w;
            return *this;
        }
        shark::simd::store4_aligned(&x, toSimd() + r.toSimd());
        return *this;
    }

    constexpr Vec& operator-=(const Vec& r) noexcept {
        if consteval {
            x -= r.x;
            y -= r.y;
            z -= r.z;
            w -= r.w;
            return *this;
        }
        shark::simd::store4_aligned(&x, toSimd() - r.toSimd());
        return *this;
    }

    constexpr Vec& operator*=(float s) noexcept {
        if consteval {
            x *= s;
            y *= s;
            z *= s;
            w *= s;
            return *this;
        }
        shark::simd::store4_aligned(&x, toSimd() * s);
        return *this;
    }

    constexpr Vec& operator*=(const Vec& r) noexcept {
        if consteval {
            x *= r.x;
            y *= r.y;
            z *= r.z;
            w *= r.w;
            return *this;
        }
        shark::simd::store4_aligned(&x, toSimd() * r.toSimd());
        return *this;
    }

    constexpr Vec& operator/=(float s) noexcept {
        if consteval {
            x /= s;
            y /= s;
            z /= s;
            w /= s;
            return *this;
        }
        shark::simd::store4_aligned(&x, toSimd() * (1.0f / s));
        return *this;
    }

    constexpr Vec& operator/=(const Vec& r) noexcept {
        if consteval {
            x /= r.x;
            y /= r.y;
            z /= r.z;
            w /= r.w;
            return *this;
        }
        shark::simd::store4_aligned(&x, toSimd() / r.toSimd());
        return *this;
    }

    [[nodiscard]] constexpr float dot(const Vec& r) const noexcept {
        if consteval {
            return (x * r.x) + (y * r.y) + (z * r.z) + (w * r.w);
        }
        return shark::simd::dot4(toSimd(), r.toSimd());
    }

    [[nodiscard]] constexpr float lengthSq() const noexcept { return dot(*this); }
    [[nodiscard]] float           length() const noexcept { return std::sqrt(lengthSq()); }

    [[nodiscard]] Vec normalized() const noexcept {
        const float len2 = lengthSq();
        if (len2 == 0.0f)
            return Vec{};
        return (*this) * (1.0f / std::sqrt(len2));
    }
};

template <std::size_t N, typename T> [[nodiscard]] constexpr Vec<N, T> operator*(T s, const Vec<N, T>& v) noexcept {
    return v * s;
}

template <std::size_t N, typename T> [[nodiscard]] constexpr T dot(const Vec<N, T>& a, const Vec<N, T>& b) noexcept {
    return a.dot(b);
}

template <std::size_t N, typename T> [[nodiscard]] constexpr T lengthSq(const Vec<N, T>& v) noexcept {
    return v.lengthSq();
}

template <std::size_t N, typename T> [[nodiscard]] T length(const Vec<N, T>& v) noexcept {
    return v.length();
}

template <std::size_t N, typename T> [[nodiscard]] Vec<N, T> normalize(const Vec<N, T>& v) noexcept {
    if constexpr ((N == 3 || N == 4) && std::is_same_v<T, float>) {
        return v.normalized();
    } else {
        T len2 = v.dot(v);
        if (len2 == T(0))
            return Vec<N, T>{};
        T invLen = T(1) / std::sqrt(len2);
        return v * invLen;
    }
}

template <typename T> [[nodiscard]] constexpr Vec<3, T> cross(const Vec<3, T>& a, const Vec<3, T>& b) noexcept {
    return {(a[1] * b[2]) - (a[2] * b[1]), (a[2] * b[0]) - (a[0] * b[2]), (a[0] * b[1]) - (a[1] * b[0])};
}

[[nodiscard]] constexpr inline Vec<3, float> cross(const Vec<3, float>& a, const Vec<3, float>& b) noexcept {
    if consteval {
        return {(a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x)};
    }
    return Vec<3, float>::fromSimdFast(shark::simd::cross3(a.toSimd(), b.toSimd()));
}

[[nodiscard]] inline Vec<3, float> min(const Vec<3, float>& a, const Vec<3, float>& b) noexcept {
    return Vec<3, float>::fromSimdFast(shark::simd::min(a.toSimd(), b.toSimd()));
}

[[nodiscard]] inline Vec<3, float> max(const Vec<3, float>& a, const Vec<3, float>& b) noexcept {
    return Vec<3, float>::fromSimdFast(shark::simd::max(a.toSimd(), b.toSimd()));
}

[[nodiscard]] inline Vec<3, float>
clamp(const Vec<3, float>& v, const Vec<3, float>& lo, const Vec<3, float>& hi) noexcept {
    return Vec<3, float>::fromSimdFast(shark::simd::clamp(v.toSimd(), lo.toSimd(), hi.toSimd()));
}

[[nodiscard]] inline Vec<3, float> abs(const Vec<3, float>& v) noexcept {
    return Vec<3, float>::fromSimdFast(shark::simd::abs(v.toSimd()));
}

[[nodiscard]] inline Vec<4, float> min(const Vec<4, float>& a, const Vec<4, float>& b) noexcept {
    return Vec<4, float>::fromSimd(shark::simd::min(a.toSimd(), b.toSimd()));
}

[[nodiscard]] inline Vec<4, float> max(const Vec<4, float>& a, const Vec<4, float>& b) noexcept {
    return Vec<4, float>::fromSimd(shark::simd::max(a.toSimd(), b.toSimd()));
}

[[nodiscard]] inline Vec<4, float>
clamp(const Vec<4, float>& v, const Vec<4, float>& lo, const Vec<4, float>& hi) noexcept {
    return Vec<4, float>::fromSimd(shark::simd::clamp(v.toSimd(), lo.toSimd(), hi.toSimd()));
}

[[nodiscard]] inline Vec<4, float> abs(const Vec<4, float>& v) noexcept {
    return Vec<4, float>::fromSimd(shark::simd::abs(v.toSimd()));
}

using Vector2 = Vec<2, float>;
using Vector3 = Vec<3, float>;
using Vector4 = Vec<4, float>;

static_assert(sizeof(Vector3) == 16 && alignof(Vector3) == 16, "Vector3 must be 16-byte aligned for SIMD");
static_assert(sizeof(Vector4) == 16 && alignof(Vector4) == 16, "Vector4 must be 16-byte aligned for SIMD");
