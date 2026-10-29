#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <memory>
#include <version>

#if defined(__SSE__) || defined(__AVX__)
#include <immintrin.h>
#endif

#if __has_include(<simd>)
#include <simd>
#endif
#if __has_include(<experimental/simd>)
#include <experimental/simd>
#endif

namespace shark::simd {

#ifdef __cpp_lib_simd

using float4  = std::fixed_size_simd<float, 4>;
using float8  = std::fixed_size_simd<float, 8>;
using float_v = std::native_simd<float>;

template <typename T> [[nodiscard]] inline float4 load4(const T* ptr) noexcept {
    float4 v;
    v.copy_from(ptr, std::element_aligned);
    return v;
}

template <typename T> [[nodiscard]] inline float4 load4_aligned(const T* ptr) noexcept {
    float4 v;
    v.copy_from(std::assume_aligned<16>(ptr), std::element_aligned);
    return v;
}

template <typename T> inline void store4(T* ptr, const float4& v) noexcept {
    v.copy_to(ptr, std::element_aligned);
}

template <typename T> inline void store4_aligned(T* ptr, const float4& v) noexcept {
    v.copy_to(std::assume_aligned<16>(ptr), std::element_aligned);
}

template <typename T> [[nodiscard]] inline float8 load8(const T* ptr) noexcept {
    float8 v;
    v.copy_from(ptr, std::element_aligned);
    return v;
}

template <typename T> inline void store8(T* ptr, const float8& v) noexcept {
    v.copy_to(ptr, std::element_aligned);
}

[[nodiscard]] inline float4 zero_w(float4 a) noexcept {
    a[3] = 0.0f;
    return a;
}

[[nodiscard]] inline float reduce_add(const float4& a) noexcept {
    return std::reduce(a);
}

[[nodiscard]] inline float reduce_add(const float8& a) noexcept {
    return std::reduce(a);
}

// Horizontal sums of three vectors at once: {sum(a), sum(b), sum(c)}
[[nodiscard]] inline std::array<float, 3> reduce_add3(const float8& a, const float8& b, const float8& c) noexcept {
    return {std::reduce(a), std::reduce(b), std::reduce(c)};
}

[[nodiscard]] inline float4 sqrt(const float4& a) noexcept {
    return std::sqrt(a);
}

[[nodiscard]] inline float8 sqrt(const float8& a) noexcept {
    return std::sqrt(a);
}

[[nodiscard]] inline float4 rsqrt(const float4& a) noexcept {
    return float4(1.0f) / std::sqrt(a);
}

[[nodiscard]] inline float8 rsqrt(const float8& a) noexcept {
    return float8(1.0f) / std::sqrt(a);
}

[[nodiscard]] inline float4 fma(const float4& a, const float4& b, const float4& c) noexcept {
    return std::fma(a, b, c);
}

[[nodiscard]] inline float8 fma(const float8& a, const float8& b, const float8& c) noexcept {
    return std::fma(a, b, c);
}

[[nodiscard]] inline float4 min(const float4& a, const float4& b) noexcept {
    return std::min(a, b);
}

[[nodiscard]] inline float8 min(const float8& a, const float8& b) noexcept {
    return std::min(a, b);
}

[[nodiscard]] inline float4 max(const float4& a, const float4& b) noexcept {
    return std::max(a, b);
}

[[nodiscard]] inline float8 max(const float8& a, const float8& b) noexcept {
    return std::max(a, b);
}

[[nodiscard]] inline float4 clamp(const float4& v, const float4& lo, const float4& hi) noexcept {
    return std::clamp(v, lo, hi);
}

[[nodiscard]] inline float8 clamp(const float8& v, const float8& lo, const float8& hi) noexcept {
    return std::clamp(v, lo, hi);
}

[[nodiscard]] inline float4 abs(const float4& a) noexcept {
    return std::abs(a);
}

[[nodiscard]] inline float8 abs(const float8& a) noexcept {
    return std::abs(a);
}

[[nodiscard]] inline float reduce_max(const float4& a) noexcept {
    return std::reduce_max(a);
}

[[nodiscard]] inline float reduce_max(const float8& a) noexcept {
    return std::reduce_max(a);
}

[[nodiscard]] inline float dot3(const float4& a, const float4& b) noexcept {
    float4 prod = a * b;
    prod[3]     = 0.0f;
    return std::reduce(prod);
}

[[nodiscard]] inline float dot4(const float4& a, const float4& b) noexcept {
    return reduce_add(a * b);
}

[[nodiscard]] inline float4 cross3(const float4& a, const float4& b) noexcept {
    float4 res;
    res[0] = (a[1] * b[2]) - (a[2] * b[1]);
    res[1] = (a[2] * b[0]) - (a[0] * b[2]);
    res[2] = (a[0] * b[1]) - (a[1] * b[0]);
    res[3] = 0.0f;
    return res;
}

inline void transpose4x4(float4& c0, float4& c1, float4& c2, float4& c3) noexcept {
    const float4 r0 = c0;
    const float4 r1 = c1;
    const float4 r2 = c2;
    const float4 r3 = c3;
    c0[0]           = r0[0];
    c0[1]           = r1[0];
    c0[2]           = r2[0];
    c0[3]           = r3[0];
    c1[0]           = r0[1];
    c1[1]           = r1[1];
    c1[2]           = r2[1];
    c1[3]           = r3[1];
    c2[0]           = r0[2];
    c2[1]           = r1[2];
    c2[2]           = r2[2];
    c2[3]           = r3[2];
    c3[0]           = r0[3];
    c3[1]           = r1[3];
    c3[2]           = r2[3];
    c3[3]           = r3[3];
}

#elifdef __cpp_lib_experimental_parallel_simd

namespace stdx = std::experimental;

using float4  = stdx::fixed_size_simd<float, 4>;
using float8  = stdx::fixed_size_simd<float, 8>;
using float_v = stdx::native_simd<float>;

template <typename T> [[nodiscard]] inline float4 load4(const T* ptr) noexcept {
    float4 v;
    v.copy_from(ptr, stdx::element_aligned);
    return v;
}

template <typename T> [[nodiscard]] inline float4 load4_aligned(const T* ptr) noexcept {
    float4 v;
    v.copy_from(std::assume_aligned<16>(ptr), stdx::element_aligned);
    return v;
}

template <typename T> inline void store4(T* ptr, const float4& v) noexcept {
    v.copy_to(ptr, stdx::element_aligned);
}

template <typename T> inline void store4_aligned(T* ptr, const float4& v) noexcept {
    v.copy_to(std::assume_aligned<16>(ptr), stdx::element_aligned);
}

template <typename T> [[nodiscard]] inline float8 load8(const T* ptr) noexcept {
    float8 v;
    v.copy_from(ptr, stdx::element_aligned);
    return v;
}

template <typename T> inline void store8(T* ptr, const float8& v) noexcept {
    v.copy_to(ptr, stdx::element_aligned);
}

[[nodiscard]] inline float4 zero_w(float4 a) noexcept {
    a[3] = 0.0f;
    return a;
}

[[nodiscard]] inline float reduce_add(const float4& a) noexcept {
    return stdx::reduce(a);
}

[[nodiscard]] inline float reduce_add(const float8& a) noexcept {
    return stdx::reduce(a);
}

// Horizontal sums of three vectors at once: {sum(a), sum(b), sum(c)}
[[nodiscard]] inline std::array<float, 3> reduce_add3(const float8& a, const float8& b, const float8& c) noexcept {
    return {stdx::reduce(a), stdx::reduce(b), stdx::reduce(c)};
}

[[nodiscard]] inline float4 sqrt(const float4& a) noexcept {
    return stdx::sqrt(a);
}

[[nodiscard]] inline float8 sqrt(const float8& a) noexcept {
    return stdx::sqrt(a);
}

[[nodiscard]] inline float4 rsqrt(const float4& a) noexcept {
    return float4(1.0f) / stdx::sqrt(a);
}

[[nodiscard]] inline float8 rsqrt(const float8& a) noexcept {
    return float8(1.0f) / stdx::sqrt(a);
}

[[nodiscard]] inline float4 fma(const float4& a, const float4& b, const float4& c) noexcept {
    return stdx::fma(a, b, c);
}

[[nodiscard]] inline float8 fma(const float8& a, const float8& b, const float8& c) noexcept {
    return stdx::fma(a, b, c);
}

[[nodiscard]] inline float4 min(const float4& a, const float4& b) noexcept {
    return stdx::min(a, b);
}

[[nodiscard]] inline float8 min(const float8& a, const float8& b) noexcept {
    return stdx::min(a, b);
}

[[nodiscard]] inline float4 max(const float4& a, const float4& b) noexcept {
    return stdx::max(a, b);
}

[[nodiscard]] inline float8 max(const float8& a, const float8& b) noexcept {
    return stdx::max(a, b);
}

[[nodiscard]] inline float4 clamp(const float4& v, const float4& lo, const float4& hi) noexcept {
    return stdx::clamp(v, lo, hi);
}

[[nodiscard]] inline float8 clamp(const float8& v, const float8& lo, const float8& hi) noexcept {
    return stdx::clamp(v, lo, hi);
}

[[nodiscard]] inline float4 abs(const float4& a) noexcept {
    return stdx::abs(a);
}

[[nodiscard]] inline float8 abs(const float8& a) noexcept {
    return stdx::abs(a);
}

[[nodiscard]] inline float reduce_max(const float4& a) noexcept {
    return stdx::hmax(a);
}

[[nodiscard]] inline float reduce_max(const float8& a) noexcept {
    return stdx::hmax(a);
}

[[nodiscard]] inline float dot3(const float4& a, const float4& b) noexcept {
    float4 prod = a * b;
    prod[3]     = 0.0f;
    return stdx::reduce(prod);
}

[[nodiscard]] inline float dot4(const float4& a, const float4& b) noexcept {
    return reduce_add(a * b);
}

[[nodiscard]] inline float4 cross3(const float4& a, const float4& b) noexcept {
    float4 res;
    res[0] = (a[1] * b[2]) - (a[2] * b[1]);
    res[1] = (a[2] * b[0]) - (a[0] * b[2]);
    res[2] = (a[0] * b[1]) - (a[1] * b[0]);
    res[3] = 0.0f;
    return res;
}

inline void transpose4x4(float4& c0, float4& c1, float4& c2, float4& c3) noexcept {
    const float4 r0 = c0;
    const float4 r1 = c1;
    const float4 r2 = c2;
    const float4 r3 = c3;
    c0[0]           = r0[0];
    c0[1]           = r1[0];
    c0[2]           = r2[0];
    c0[3]           = r3[0];
    c1[0]           = r0[1];
    c1[1]           = r1[1];
    c1[2]           = r2[1];
    c1[3]           = r3[1];
    c2[0]           = r0[2];
    c2[1]           = r1[2];
    c2[2]           = r2[2];
    c2[3]           = r3[2];
    c3[0]           = r0[3];
    c3[1]           = r1[3];
    c3[2]           = r2[3];
    c3[3]           = r3[3];
}

#else

// Portable SIMD vector for WebAssembly / Clang / GCC using compiler vector extensions
struct alignas(16) float4 {
    using vec4_t = float __attribute__((vector_size(16)));
    vec4_t v{0.0f, 0.0f, 0.0f, 0.0f};

    constexpr float4() noexcept = default;
    constexpr explicit float4(float s) noexcept : v{s, s, s, s} {}
    constexpr float4(float x, float y, float z, float w) noexcept : v{x, y, z, w} {}
    constexpr explicit float4(vec4_t vec) noexcept : v(vec) {}

    [[nodiscard]] float&       operator[](std::size_t i) noexcept { return reinterpret_cast<float*>(&v)[i]; }
    [[nodiscard]] const float& operator[](std::size_t i) const noexcept {
        return reinterpret_cast<const float*>(&v)[i];
    }

    [[nodiscard]] constexpr float4 operator+() const noexcept { return *this; }
    [[nodiscard]] constexpr float4 operator-() const noexcept { return float4(-v); }

    [[nodiscard]] constexpr float4 operator+(const float4& r) const noexcept { return float4(v + r.v); }
    [[nodiscard]] constexpr float4 operator-(const float4& r) const noexcept { return float4(v - r.v); }
    [[nodiscard]] constexpr float4 operator*(const float4& r) const noexcept { return float4(v * r.v); }
    [[nodiscard]] constexpr float4 operator/(const float4& r) const noexcept { return float4(v / r.v); }

    [[nodiscard]] constexpr float4 operator*(float s) const noexcept {
        vec4_t sv{s, s, s, s};
        return float4(v * sv);
    }
    [[nodiscard]] constexpr float4 operator/(float s) const noexcept {
        vec4_t sv{s, s, s, s};
        return float4(v / sv);
    }

    constexpr float4& operator+=(const float4& r) noexcept {
        v += r.v;
        return *this;
    }
    constexpr float4& operator-=(const float4& r) noexcept {
        v -= r.v;
        return *this;
    }
    constexpr float4& operator*=(const float4& r) noexcept {
        v *= r.v;
        return *this;
    }
    constexpr float4& operator/=(const float4& r) noexcept {
        v /= r.v;
        return *this;
    }
};

[[nodiscard]] constexpr float4 operator*(float s, const float4& r) noexcept {
    return r * s;
}

struct alignas(32) float8 {
    using vec8_t = float __attribute__((vector_size(32)));
    vec8_t v{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    constexpr float8() noexcept = default;
    constexpr explicit float8(float s) noexcept : v{s, s, s, s, s, s, s, s} {}
    constexpr float8(float v0, float v1, float v2, float v3, float v4, float v5, float v6, float v7) noexcept
        : v{v0, v1, v2, v3, v4, v5, v6, v7} {}
    constexpr explicit float8(vec8_t vec) noexcept : v(vec) {}

    [[nodiscard]] float&       operator[](std::size_t i) noexcept { return reinterpret_cast<float*>(&v)[i]; }
    [[nodiscard]] const float& operator[](std::size_t i) const noexcept {
        return reinterpret_cast<const float*>(&v)[i];
    }

    [[nodiscard]] constexpr float8 operator+() const noexcept { return *this; }
    [[nodiscard]] constexpr float8 operator-() const noexcept { return float8(-v); }

    [[nodiscard]] constexpr float8 operator+(const float8& r) const noexcept { return float8(v + r.v); }
    [[nodiscard]] constexpr float8 operator-(const float8& r) const noexcept { return float8(v - r.v); }
    [[nodiscard]] constexpr float8 operator*(const float8& r) const noexcept { return float8(v * r.v); }
    [[nodiscard]] constexpr float8 operator/(const float8& r) const noexcept { return float8(v / r.v); }

    [[nodiscard]] constexpr float8 operator*(float s) const noexcept {
        vec8_t sv{s, s, s, s, s, s, s, s};
        return float8(v * sv);
    }
    [[nodiscard]] constexpr float8 operator/(float s) const noexcept {
        vec8_t sv{s, s, s, s, s, s, s, s};
        return float8(v / sv);
    }

    constexpr float8& operator+=(const float8& r) noexcept {
        v += r.v;
        return *this;
    }
    constexpr float8& operator-=(const float8& r) noexcept {
        v -= r.v;
        return *this;
    }
    constexpr float8& operator*=(const float8& r) noexcept {
        v *= r.v;
        return *this;
    }
    constexpr float8& operator/=(const float8& r) noexcept {
        v /= r.v;
        return *this;
    }
};

[[nodiscard]] constexpr float8 operator*(float s, const float8& r) noexcept {
    return r * s;
}

using float_v = float4;

template <typename T> [[nodiscard]] inline float4 load4(const T* ptr) noexcept {
    float4 res;
    std::memcpy(&res.v, ptr, sizeof(float) * 4);
    return res;
}

template <typename T> [[nodiscard]] inline float4 load4_aligned(const T* ptr) noexcept {
    float4 res;
    std::memcpy(&res.v, std::assume_aligned<16>(ptr), sizeof(float) * 4);
    return res;
}

template <typename T> inline void store4(T* ptr, const float4& val) noexcept {
    std::memcpy(ptr, &val.v, sizeof(float) * 4);
}

template <typename T> inline void store4_aligned(T* ptr, const float4& val) noexcept {
    std::memcpy(std::assume_aligned<16>(ptr), &val.v, sizeof(float) * 4);
}

template <typename T> [[nodiscard]] inline float8 load8(const T* ptr) noexcept {
    float8 res;
    std::memcpy(&res.v, ptr, sizeof(float) * 8);
    return res;
}

template <typename T> inline void store8(T* ptr, const float8& val) noexcept {
    std::memcpy(ptr, &val.v, sizeof(float) * 8);
}

[[nodiscard]] constexpr float4 zero_w(const float4& a) noexcept {
#ifdef __clang__
    constexpr float4::vec4_t zero{0.0f, 0.0f, 0.0f, 0.0f};
    return float4(__builtin_shufflevector(a.v, zero, 0, 1, 2, 7));
#else
    return float4(a.v[0], a.v[1], a.v[2], 0.0f);
#endif
}

[[nodiscard]] constexpr float reduce_add(const float4& a) noexcept {
    return (a.v[0] + a.v[1]) + (a.v[2] + a.v[3]);
}

[[nodiscard]] constexpr float reduce_add(const float8& a) noexcept {
    return ((a.v[0] + a.v[1]) + (a.v[2] + a.v[3])) + ((a.v[4] + a.v[5]) + (a.v[6] + a.v[7]));
}

// {sum(a), sum(b), sum(c)} with the shuffle/add work shared through a partial 4x4 transpose.
[[nodiscard]] inline std::array<float, 3> reduce_add3(const float8& a, const float8& b, const float8& c) noexcept {
#ifdef __clang__
    using vec4_t = float4::vec4_t;
    // 8 -> 4 lanes: fold upper half onto lower half
    const vec4_t     a4 = __builtin_shufflevector(a.v, a.v, 0, 1, 2, 3) + __builtin_shufflevector(a.v, a.v, 4, 5, 6, 7);
    const vec4_t     b4 = __builtin_shufflevector(b.v, b.v, 0, 1, 2, 3) + __builtin_shufflevector(b.v, b.v, 4, 5, 6, 7);
    const vec4_t     c4 = __builtin_shufflevector(c.v, c.v, 0, 1, 2, 3) + __builtin_shufflevector(c.v, c.v, 4, 5, 6, 7);
    constexpr vec4_t zero{0.0f, 0.0f, 0.0f, 0.0f};

    // Interleave + add: ab = {a0+a2, b0+b2, a1+a3, b1+b3}, cz = {c0+c2, 0, c1+c3, 0}
    const vec4_t ab = __builtin_shufflevector(a4, b4, 0, 4, 1, 5) + __builtin_shufflevector(a4, b4, 2, 6, 3, 7);
    const vec4_t cz = __builtin_shufflevector(c4, zero, 0, 4, 1, 5) + __builtin_shufflevector(c4, zero, 2, 6, 3, 7);

    // Final combine: {sum(a), sum(b), sum(c), 0}
    const vec4_t r = __builtin_shufflevector(ab, cz, 0, 1, 4, 5) + __builtin_shufflevector(ab, cz, 2, 3, 6, 7);
    return {r[0], r[1], r[2]};
#else
    return {reduce_add(a), reduce_add(b), reduce_add(c)};
#endif
}

[[nodiscard]] inline float4 sqrt(const float4& a) noexcept {
#ifdef __clang__
    return float4(__builtin_elementwise_sqrt(a.v));
#else
    return float4(std::sqrt(a[0]), std::sqrt(a[1]), std::sqrt(a[2]), std::sqrt(a[3]));
#endif
}

[[nodiscard]] inline float8 sqrt(const float8& a) noexcept {
#ifdef __clang__
    return float8(__builtin_elementwise_sqrt(a.v));
#else
    return float8(
        std::sqrt(a[0]), std::sqrt(a[1]), std::sqrt(a[2]), std::sqrt(a[3]), std::sqrt(a[4]), std::sqrt(a[5]),
        std::sqrt(a[6]), std::sqrt(a[7])
    );
#endif
}

// NOLINTBEGIN(portability-simd-intrinsics): approximate rsqrt and FMA have no portable spelling
[[nodiscard]] inline float4 rsqrt(const float4& a) noexcept {
#ifdef __FMA__
    const float4 y0(_mm_rsqrt_ps(a.v));
    const float4 half_a_y0 = float4(0.5f) * a * y0;
    return y0 * float4(_mm_fnmadd_ps(half_a_y0.v, y0.v, _mm_set1_ps(1.5f)));
#elifdef __SSE__
    const float4 y0(_mm_rsqrt_ps(a.v));
    return y0 * (float4(1.5f) - (float4(0.5f) * a * y0 * y0));
#else
    return float4(1.0f) / sqrt(a);
#endif
}

[[nodiscard]] inline float8 rsqrt(const float8& a) noexcept {
#ifdef __FMA__
    const float8 y0(_mm256_rsqrt_ps(a.v));
    const float8 half_a_y0 = float8(0.5f) * a * y0;
    return y0 * float8(_mm256_fnmadd_ps(half_a_y0.v, y0.v, _mm256_set1_ps(1.5f)));
#elifdef __AVX__
    const float8 y0(_mm256_rsqrt_ps(a.v));
    return y0 * (float8(1.5f) - (float8(0.5f) * a * y0 * y0));
#else
    return float8(1.0f) / sqrt(a);
#endif
}

// Without a hardware FMA the builtin lowers to scalar fmaf calls, so fall back to mul + add.
[[nodiscard]] inline float4 fma(const float4& a, const float4& b, const float4& c) noexcept {
#ifdef __FMA__
    return float4(_mm_fmadd_ps(a.v, b.v, c.v));
#else
    return (a * b) + c;
#endif
}

[[nodiscard]] inline float8 fma(const float8& a, const float8& b, const float8& c) noexcept {
#ifdef __FMA__
    return float8(_mm256_fmadd_ps(a.v, b.v, c.v));
#else
    return (a * b) + c;
#endif
}
// NOLINTEND(portability-simd-intrinsics)

// Same semantics as minps/maxps; compiles to them on x86 and to f32x4.pmin/pmax on wasm.
[[nodiscard]] inline float4 min(const float4& a, const float4& b) noexcept {
    return float4(a.v < b.v ? a.v : b.v);
}

[[nodiscard]] inline float8 min(const float8& a, const float8& b) noexcept {
    return float8(a.v < b.v ? a.v : b.v);
}

[[nodiscard]] inline float4 max(const float4& a, const float4& b) noexcept {
    return float4(a.v > b.v ? a.v : b.v);
}

[[nodiscard]] inline float8 max(const float8& a, const float8& b) noexcept {
    return float8(a.v > b.v ? a.v : b.v);
}

[[nodiscard]] inline float4 clamp(const float4& v, const float4& lo, const float4& hi) noexcept {
    return min(max(v, lo), hi);
}

[[nodiscard]] inline float8 clamp(const float8& v, const float8& lo, const float8& hi) noexcept {
    return min(max(v, lo), hi);
}

[[nodiscard]] inline float4 abs(const float4& a) noexcept {
#ifdef __clang__
    return float4(__builtin_elementwise_abs(a.v));
#else
    return float4(std::abs(a[0]), std::abs(a[1]), std::abs(a[2]), std::abs(a[3]));
#endif
}

[[nodiscard]] inline float8 abs(const float8& a) noexcept {
#ifdef __clang__
    return float8(__builtin_elementwise_abs(a.v));
#else
    return {std::abs(a[0]), std::abs(a[1]), std::abs(a[2]), std::abs(a[3]),
            std::abs(a[4]), std::abs(a[5]), std::abs(a[6]), std::abs(a[7])};
#endif
}

[[nodiscard]] inline float reduce_max(const float4& a) noexcept {
    return std::max({a[0], a[1], a[2], a[3]});
}

[[nodiscard]] inline float reduce_max(const float8& a) noexcept {
    return std::max({a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]});
}

[[nodiscard]] constexpr float dot3(const float4& a, const float4& b) noexcept {
    const float4 prod = a * b;
    return (prod.v[0] + prod.v[1]) + prod.v[2];
}

[[nodiscard]] constexpr float dot4(const float4& a, const float4& b) noexcept {
    return reduce_add(a * b);
}

[[nodiscard]] constexpr float4 cross3(const float4& a, const float4& b) noexcept {
#ifdef __clang__
    const float4::vec4_t a_yzx = __builtin_shufflevector(a.v, a.v, 1, 2, 0, 3);
    const float4::vec4_t b_zxy = __builtin_shufflevector(b.v, b.v, 2, 0, 1, 3);
    const float4::vec4_t a_zxy = __builtin_shufflevector(a.v, a.v, 2, 0, 1, 3);
    const float4::vec4_t b_yzx = __builtin_shufflevector(b.v, b.v, 1, 2, 0, 3);
    return zero_w(float4((a_yzx * b_zxy) - (a_zxy * b_yzx)));
#else
    return float4(
        (a.v[1] * b.v[2]) - (a.v[2] * b.v[1]), (a.v[2] * b.v[0]) - (a.v[0] * b.v[2]),
        (a.v[0] * b.v[1]) - (a.v[1] * b.v[0]), 0.0f
    );
#endif
}

inline void transpose4x4(float4& c0, float4& c1, float4& c2, float4& c3) noexcept {
#ifdef __clang__
    const float4::vec4_t t0 = __builtin_shufflevector(c0.v, c1.v, 0, 4, 1, 5);
    const float4::vec4_t t1 = __builtin_shufflevector(c0.v, c1.v, 2, 6, 3, 7);
    const float4::vec4_t t2 = __builtin_shufflevector(c2.v, c3.v, 0, 4, 1, 5);
    const float4::vec4_t t3 = __builtin_shufflevector(c2.v, c3.v, 2, 6, 3, 7);

    c0 = float4(__builtin_shufflevector(t0, t2, 0, 1, 4, 5));
    c1 = float4(__builtin_shufflevector(t0, t2, 2, 3, 6, 7));
    c2 = float4(__builtin_shufflevector(t1, t3, 0, 1, 4, 5));
    c3 = float4(__builtin_shufflevector(t1, t3, 2, 3, 6, 7));
#elifdef __SSE__
    _MM_TRANSPOSE4_PS(c0.v, c1.v, c2.v, c3.v);
#else
    const float4 r0 = c0;
    const float4 r1 = c1;
    const float4 r2 = c2;
    const float4 r3 = c3;
    c0              = float4(r0[0], r1[0], r2[0], r3[0]);
    c1              = float4(r0[1], r1[1], r2[1], r3[1]);
    c2              = float4(r0[2], r1[2], r2[2], r3[2]);
    c3              = float4(r0[3], r1[3], r2[3], r3[3]);
#endif
}

#endif

} // namespace shark::simd
