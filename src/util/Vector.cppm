export module shark:util.Vector;

import std;

import :util.SIMD;

namespace shark {

template <std::size_t N, typename T>
inline constexpr bool is_simd_vec_v = std::is_same_v<T, float> && (N == 3 || N == 4);

} // namespace shark

export namespace shark {

template <std::size_t N, typename T = float> struct Vec;

template <std::size_t N, typename T>
[[nodiscard]] constexpr inline T dot(const Vec<N, T>& a, const Vec<N, T>& b) noexcept {
    if constexpr (is_simd_vec_v<N, T>) {
        if !consteval {
            if constexpr (N == 3) {
                return shark::simd::dot3(a.toSimd(), b.toSimd());
            } else {
                return shark::simd::dot4(a.toSimd(), b.toSimd());
            }
        }
    }
    if constexpr (N == 2) {
        return (a.x * b.x) + (a.y * b.y);
    } else if constexpr (N == 3) {
        return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    } else if constexpr (N == 4) {
        return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
    } else {
        T sum = 0;
        for (std::size_t i = 0; i < N; ++i)
            sum += a.v[i] * b.v[i];
        return sum;
    }
}

template <std::size_t N, typename T> [[nodiscard]] constexpr inline Vec<N, T> operator+(const Vec<N, T>& a) noexcept {
    return a;
}

template <std::size_t N, typename T> [[nodiscard]] constexpr inline Vec<N, T> operator-(const Vec<N, T>& a) noexcept {
    if constexpr (is_simd_vec_v<N, T>) {
        if !consteval {
            return Vec<N, T>::fromSimdFast(-a.toSimd());
        }
    }
    if constexpr (N == 2) {
        return {-a.x, -a.y};
    } else if constexpr (N == 3) {
        return {-a.x, -a.y, -a.z};
    } else if constexpr (N == 4) {
        return {-a.x, -a.y, -a.z, -a.w};
    } else {
        Vec<N, T> out{};
        for (std::size_t i = 0; i < N; ++i)
            out.v[i] = -a.v[i];
        return out;
    }
}

template <std::size_t N, typename T>
[[nodiscard]] constexpr inline Vec<N, T> operator+(const Vec<N, T>& a, const Vec<N, T>& b) noexcept {
    if constexpr (is_simd_vec_v<N, T>) {
        if !consteval {
            return Vec<N, T>::fromSimdFast(a.toSimd() + b.toSimd());
        }
    }
    if constexpr (N == 2) {
        return {a.x + b.x, a.y + b.y};
    } else if constexpr (N == 3) {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    } else if constexpr (N == 4) {
        return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
    } else {
        Vec<N, T> out{};
        for (std::size_t i = 0; i < N; ++i)
            out.v[i] = a.v[i] + b.v[i];
        return out;
    }
}

template <std::size_t N, typename T>
[[nodiscard]] constexpr inline Vec<N, T> operator-(const Vec<N, T>& a, const Vec<N, T>& b) noexcept {
    if constexpr (is_simd_vec_v<N, T>) {
        if !consteval {
            return Vec<N, T>::fromSimdFast(a.toSimd() - b.toSimd());
        }
    }
    if constexpr (N == 2) {
        return {a.x - b.x, a.y - b.y};
    } else if constexpr (N == 3) {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    } else if constexpr (N == 4) {
        return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
    } else {
        Vec<N, T> out{};
        for (std::size_t i = 0; i < N; ++i)
            out.v[i] = a.v[i] - b.v[i];
        return out;
    }
}

template <std::size_t N, typename T>
[[nodiscard]] constexpr inline Vec<N, T> operator*(const Vec<N, T>& a, T s) noexcept {
    if constexpr (is_simd_vec_v<N, T>) {
        if !consteval {
            return Vec<N, T>::fromSimdFast(a.toSimd() * s);
        }
    }
    if constexpr (N == 2) {
        return {a.x * s, a.y * s};
    } else if constexpr (N == 3) {
        return {a.x * s, a.y * s, a.z * s};
    } else if constexpr (N == 4) {
        return {a.x * s, a.y * s, a.z * s, a.w * s};
    } else {
        Vec<N, T> out{};
        for (std::size_t i = 0; i < N; ++i)
            out.v[i] = a.v[i] * s;
        return out;
    }
}

template <std::size_t N, typename T>
[[nodiscard]] constexpr inline Vec<N, T> operator*(T s, const Vec<N, T>& a) noexcept {
    return a * s;
}

template <std::size_t N, typename T>
[[nodiscard]] constexpr inline Vec<N, T> operator*(const Vec<N, T>& a, const Vec<N, T>& b) noexcept {
    if constexpr (is_simd_vec_v<N, T>) {
        if !consteval {
            return Vec<N, T>::fromSimdFast(a.toSimd() * b.toSimd());
        }
    }
    if constexpr (N == 2) {
        return {a.x * b.x, a.y * b.y};
    } else if constexpr (N == 3) {
        return {a.x * b.x, a.y * b.y, a.z * b.z};
    } else if constexpr (N == 4) {
        return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
    } else {
        Vec<N, T> out{};
        for (std::size_t i = 0; i < N; ++i)
            out.v[i] = a.v[i] * b.v[i];
        return out;
    }
}

template <std::size_t N, typename T>
[[nodiscard]] constexpr inline Vec<N, T> operator/(const Vec<N, T>& a, T s) noexcept {
    if constexpr (is_simd_vec_v<N, T>) {
        if !consteval {
            return Vec<N, T>::fromSimd(a.toSimd() * (1.0f / s));
        }
    }
    if constexpr (N == 2) {
        return {a.x / s, a.y / s};
    } else if constexpr (N == 3) {
        return {a.x / s, a.y / s, a.z / s};
    } else if constexpr (N == 4) {
        return {a.x / s, a.y / s, a.z / s, a.w / s};
    } else {
        Vec<N, T> out{};
        for (std::size_t i = 0; i < N; ++i)
            out.v[i] = a.v[i] / s;
        return out;
    }
}

template <std::size_t N, typename T>
[[nodiscard]] constexpr inline Vec<N, T> operator/(const Vec<N, T>& a, const Vec<N, T>& b) noexcept {
    if constexpr (is_simd_vec_v<N, T>) {
        if !consteval {
            return Vec<N, T>::fromSimd(a.toSimd() / b.toSimd());
        }
    }
    if constexpr (N == 2) {
        return {a.x / b.x, a.y / b.y};
    } else if constexpr (N == 3) {
        return {a.x / b.x, a.y / b.y, a.z / b.z};
    } else if constexpr (N == 4) {
        return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
    } else {
        Vec<N, T> out{};
        for (std::size_t i = 0; i < N; ++i)
            out.v[i] = a.v[i] / b.v[i];
        return out;
    }
}

template <std::size_t N, typename T> constexpr inline Vec<N, T>& operator+=(Vec<N, T>& a, const Vec<N, T>& b) noexcept {
    if constexpr (is_simd_vec_v<N, T>) {
        if !consteval {
            shark::simd::store4_aligned(&a.x, a.toSimd() + b.toSimd());
            return a;
        }
    }
    if constexpr (N == 2) {
        a.x += b.x;
        a.y += b.y;
    } else if constexpr (N == 3) {
        a.x += b.x;
        a.y += b.y;
        a.z += b.z;
    } else if constexpr (N == 4) {
        a.x += b.x;
        a.y += b.y;
        a.z += b.z;
        a.w += b.w;
    } else {
        for (std::size_t i = 0; i < N; ++i)
            a.v[i] += b.v[i];
    }
    return a;
}

template <std::size_t N, typename T> constexpr inline Vec<N, T>& operator-=(Vec<N, T>& a, const Vec<N, T>& b) noexcept {
    if constexpr (is_simd_vec_v<N, T>) {
        if !consteval {
            shark::simd::store4_aligned(&a.x, a.toSimd() - b.toSimd());
            return a;
        }
    }
    if constexpr (N == 2) {
        a.x -= b.x;
        a.y -= b.y;
    } else if constexpr (N == 3) {
        a.x -= b.x;
        a.y -= b.y;
        a.z -= b.z;
    } else if constexpr (N == 4) {
        a.x -= b.x;
        a.y -= b.y;
        a.z -= b.z;
        a.w -= b.w;
    } else {
        for (std::size_t i = 0; i < N; ++i)
            a.v[i] -= b.v[i];
    }
    return a;
}

template <std::size_t N, typename T> constexpr inline Vec<N, T>& operator*=(Vec<N, T>& a, T s) noexcept {
    if constexpr (is_simd_vec_v<N, T>) {
        if !consteval {
            shark::simd::store4_aligned(&a.x, a.toSimd() * s);
            return a;
        }
    }
    if constexpr (N == 2) {
        a.x *= s;
        a.y *= s;
    } else if constexpr (N == 3) {
        a.x *= s;
        a.y *= s;
        a.z *= s;
    } else if constexpr (N == 4) {
        a.x *= s;
        a.y *= s;
        a.z *= s;
        a.w *= s;
    } else {
        for (std::size_t i = 0; i < N; ++i)
            a.v[i] *= s;
    }
    return a;
}

template <std::size_t N, typename T> constexpr inline Vec<N, T>& operator*=(Vec<N, T>& a, const Vec<N, T>& b) noexcept {
    if constexpr (is_simd_vec_v<N, T>) {
        if !consteval {
            shark::simd::store4_aligned(&a.x, a.toSimd() * b.toSimd());
            return a;
        }
    }
    if constexpr (N == 2) {
        a.x *= b.x;
        a.y *= b.y;
    } else if constexpr (N == 3) {
        a.x *= b.x;
        a.y *= b.y;
        a.z *= b.z;
    } else if constexpr (N == 4) {
        a.x *= b.x;
        a.y *= b.y;
        a.z *= b.z;
        a.w *= b.w;
    } else {
        for (std::size_t i = 0; i < N; ++i)
            a.v[i] *= b.v[i];
    }
    return a;
}

template <std::size_t N, typename T> constexpr inline Vec<N, T>& operator/=(Vec<N, T>& a, T s) noexcept {
    if constexpr (is_simd_vec_v<N, T>) {
        if !consteval {
            if constexpr (N == 3) {
                shark::simd::store4_aligned(&a.x, shark::simd::zero_w(a.toSimd() * (1.0f / s)));
            } else {
                shark::simd::store4_aligned(&a.x, a.toSimd() * (1.0f / s));
            }
            return a;
        }
    }
    if constexpr (N == 2) {
        a.x /= s;
        a.y /= s;
    } else if constexpr (N == 3) {
        a.x /= s;
        a.y /= s;
        a.z /= s;
    } else if constexpr (N == 4) {
        a.x /= s;
        a.y /= s;
        a.z /= s;
        a.w /= s;
    } else {
        for (std::size_t i = 0; i < N; ++i)
            a.v[i] /= s;
    }
    return a;
}

template <std::size_t N, typename T> constexpr inline Vec<N, T>& operator/=(Vec<N, T>& a, const Vec<N, T>& b) noexcept {
    if constexpr (is_simd_vec_v<N, T>) {
        if !consteval {
            if constexpr (N == 3) {
                shark::simd::store4_aligned(&a.x, shark::simd::zero_w(a.toSimd() / b.toSimd()));
            } else {
                shark::simd::store4_aligned(&a.x, a.toSimd() / b.toSimd());
            }
            return a;
        }
    }
    if constexpr (N == 2) {
        a.x /= b.x;
        a.y /= b.y;
    } else if constexpr (N == 3) {
        a.x /= b.x;
        a.y /= b.y;
        a.z /= b.z;
    } else if constexpr (N == 4) {
        a.x /= b.x;
        a.y /= b.y;
        a.z /= b.z;
        a.w /= b.w;
    } else {
        for (std::size_t i = 0; i < N; ++i)
            a.v[i] /= b.v[i];
    }
    return a;
}

template <std::size_t N, typename T> struct Vec {
    std::array<T, N> v{};

    [[nodiscard]] constexpr inline T&       operator[](std::size_t i) noexcept { return v[i]; }
    [[nodiscard]] constexpr inline const T& operator[](std::size_t i) const noexcept { return v[i]; }

    [[nodiscard]] constexpr inline T dot(const Vec& r) const noexcept { return shark::dot(*this, r); }
    [[nodiscard]] constexpr inline T lengthSq() const noexcept { return shark::dot(*this, *this); }
    [[nodiscard]] inline T           length() const noexcept { return std::sqrt(lengthSq()); }
};

template <typename T> struct Vec<2, T> {
    T x{};
    T y{};

    [[nodiscard]] constexpr inline T&       operator[](std::size_t i) noexcept { return (i == 0) ? x : y; }
    [[nodiscard]] constexpr inline const T& operator[](std::size_t i) const noexcept { return (i == 0) ? x : y; }

    [[nodiscard]] constexpr inline T dot(const Vec& r) const noexcept { return shark::dot(*this, r); }
    [[nodiscard]] constexpr inline T lengthSq() const noexcept { return shark::dot(*this, *this); }
    [[nodiscard]] inline T           length() const noexcept { return std::sqrt(lengthSq()); }
};

template <typename T> struct Vec<3, T> {
    T x{};
    T y{};
    T z{};

    static constexpr inline Vec up() noexcept { return {0, 1, 0}; }
    static constexpr inline Vec down() noexcept { return {0, -1, 0}; }
    static constexpr inline Vec left() noexcept { return {-1, 0, 0}; }
    static constexpr inline Vec right() noexcept { return {1, 0, 0}; }
    static constexpr inline Vec forward() noexcept { return {0, 0, 1}; }
    static constexpr inline Vec back() noexcept { return {0, 0, -1}; }

    [[nodiscard]] constexpr inline T& operator[](std::size_t i) noexcept { return (i == 0) ? x : (i == 1) ? y : z; }
    [[nodiscard]] constexpr inline const T& operator[](std::size_t i) const noexcept {
        return (i == 0) ? x : (i == 1) ? y : z;
    }

    [[nodiscard]] constexpr inline T dot(const Vec& r) const noexcept { return shark::dot(*this, r); }
    [[nodiscard]] constexpr inline T lengthSq() const noexcept { return shark::dot(*this, *this); }
    [[nodiscard]] inline T           length() const noexcept { return std::sqrt(lengthSq()); }
};

// SIMD-backed Vector3: padded to 16 bytes so it loads straight into a float4.
template <> struct alignas(16) Vec<3, float> {
    float x{};
    float y{};
    float z{};
    float pad_{0.0f}; // must stay 0; ops that could break that (division) go through fromSimd/zero_w

    static constexpr inline Vec up() noexcept { return {0.0f, 1.0f, 0.0f}; }
    static constexpr inline Vec down() noexcept { return {0.0f, -1.0f, 0.0f}; }
    static constexpr inline Vec left() noexcept { return {-1.0f, 0.0f, 0.0f}; }
    static constexpr inline Vec right() noexcept { return {1.0f, 0.0f, 0.0f}; }
    static constexpr inline Vec forward() noexcept { return {0.0f, 0.0f, 1.0f}; }
    static constexpr inline Vec back() noexcept { return {0.0f, 0.0f, -1.0f}; }

    [[nodiscard]] inline shark::simd::float4 toSimd() const noexcept { return shark::simd::load4_aligned(&x); }

    [[nodiscard]] static inline Vec fromSimdFast(const shark::simd::float4& v) noexcept {
        Vec out;
        shark::simd::store4_aligned(&out.x, v);
        return out;
    }

    [[nodiscard]] static inline Vec fromSimd(const shark::simd::float4& v) noexcept {
        Vec out;
        shark::simd::store4_aligned(&out.x, shark::simd::zero_w(v));
        return out;
    }

    [[nodiscard]] constexpr inline float& operator[](std::size_t i) noexcept { return (i == 0) ? x : (i == 1) ? y : z; }
    [[nodiscard]] constexpr inline const float& operator[](std::size_t i) const noexcept {
        return (i == 0) ? x : (i == 1) ? y : z;
    }

    [[nodiscard]] constexpr inline float dot(const Vec& r) const noexcept { return shark::dot(*this, r); }
    [[nodiscard]] constexpr inline float lengthSq() const noexcept { return shark::dot(*this, *this); }
    [[nodiscard]] inline float           length() const noexcept { return std::sqrt(lengthSq()); }

    [[nodiscard]] inline Vec normalized() const noexcept {
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

    [[nodiscard]] inline shark::simd::float4 toSimd() const noexcept
        requires std::is_same_v<T, float>
    {
        return shark::simd::load4_aligned(&x);
    }

    [[nodiscard]] static inline Vec fromSimdFast(const shark::simd::float4& v) noexcept
        requires std::is_same_v<T, float>
    {
        Vec out;
        shark::simd::store4_aligned(&out.x, v);
        return out;
    }

    [[nodiscard]] static inline Vec fromSimd(const shark::simd::float4& v) noexcept
        requires std::is_same_v<T, float>
    {
        return fromSimdFast(v);
    }

    [[nodiscard]] constexpr inline T& operator[](std::size_t i) noexcept {
        return (i == 0) ? x : (i == 1) ? y : (i == 2) ? z : w;
    }
    [[nodiscard]] constexpr inline const T& operator[](std::size_t i) const noexcept {
        return (i == 0) ? x : (i == 1) ? y : (i == 2) ? z : w;
    }

    [[nodiscard]] constexpr inline T dot(const Vec& r) const noexcept { return shark::dot(*this, r); }
    [[nodiscard]] constexpr inline T lengthSq() const noexcept { return shark::dot(*this, *this); }
    [[nodiscard]] inline T           length() const noexcept { return std::sqrt(lengthSq()); }

    [[nodiscard]] inline Vec normalized() const noexcept
        requires std::is_same_v<T, float>
    {
        const float len2 = lengthSq();
        if (len2 == 0.0f)
            return Vec{};
        return (*this) * (1.0f / std::sqrt(len2));
    }
};

template <std::size_t N, typename T> [[nodiscard]] constexpr inline T lengthSq(const Vec<N, T>& v) noexcept {
    return v.lengthSq();
}

template <std::size_t N, typename T> [[nodiscard]] inline T length(const Vec<N, T>& v) noexcept {
    return v.length();
}

template <std::size_t N, typename T> [[nodiscard]] inline Vec<N, T> normalize(const Vec<N, T>& v) noexcept {
    if constexpr (is_simd_vec_v<N, T>) {
        return v.normalized();
    } else {
        T len2 = v.dot(v);
        if (len2 == T(0))
            return Vec<N, T>{};
        T invLen = T(1) / std::sqrt(len2);
        return v * invLen;
    }
}

template <std::size_t N, typename T>
[[nodiscard]] constexpr inline Vec<N, T> lerp(const Vec<N, T>& a, const Vec<N, T>& b, T t) noexcept {
    return a + ((b - a) * t);
}

template <typename T> [[nodiscard]] constexpr inline Vec<3, T> cross(const Vec<3, T>& a, const Vec<3, T>& b) noexcept {
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

} // namespace shark
