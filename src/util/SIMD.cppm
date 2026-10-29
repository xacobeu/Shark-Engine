export module shark:util.SIMD;

import std;

namespace shark::simd::detail {

template <std::size_t N> struct RawVec;
template <> struct RawVec<4> {
    using type = float __attribute__((vector_size(16)));
    [[nodiscard]] static constexpr inline type splat(float s) noexcept { return {s, s, s, s}; }
};
template <> struct RawVec<8> {
    using type = float __attribute__((vector_size(32)));
    [[nodiscard]] static constexpr inline type splat(float s) noexcept { return {s, s, s, s, s, s, s, s}; }
};

template <std::size_t N> using raw_vec_t = typename RawVec<N>::type;

template <std::size_t N> [[nodiscard]] inline raw_vec_t<N> sqrt(raw_vec_t<N> v) noexcept {
#ifdef __clang__
    return __builtin_elementwise_sqrt(v);
#else
    raw_vec_t<N> out{};
    for (std::size_t i = 0; i < N; ++i)
        out[i] = std::sqrt(v[i]);
    return out;
#endif
}

template <std::size_t N> [[nodiscard]] inline raw_vec_t<N> abs(raw_vec_t<N> v) noexcept {
#ifdef __clang__
    return __builtin_elementwise_abs(v);
#else
    raw_vec_t<N> out{};
    for (std::size_t i = 0; i < N; ++i)
        out[i] = std::abs(v[i]);
    return out;
#endif
}

// NOLINTBEGIN(portability-simd-intrinsics): approximate rsqrt has no portable spelling
template <std::size_t N>
[[nodiscard]] inline raw_vec_t<N> fma(raw_vec_t<N> a, raw_vec_t<N> b, raw_vec_t<N> c) noexcept {
#ifdef __FMA__
    return __builtin_elementwise_fma(a, b, c);
#else
    return (a * b) + c;
#endif
}

template <std::size_t N> [[nodiscard]] inline raw_vec_t<N> rsqrt(raw_vec_t<N> a) noexcept {
#if defined(__SSE__)
    if constexpr (N == 4) {
        const raw_vec_t<4> y0        = __builtin_ia32_rsqrtps(a);
        const raw_vec_t<4> half_a_y0 = RawVec<4>::splat(0.5f) * a * y0;
        return y0 * detail::fma<4>(-half_a_y0, y0, RawVec<4>::splat(1.5f));
    }
#endif
#if defined(__AVX__)
    if constexpr (N == 8) {
        const raw_vec_t<8> y0        = __builtin_ia32_rsqrtps256(a);
        const raw_vec_t<8> half_a_y0 = RawVec<8>::splat(0.5f) * a * y0;
        return y0 * detail::fma<8>(-half_a_y0, y0, RawVec<8>::splat(1.5f));
    }
#endif
    return RawVec<N>::splat(1.0f) / detail::sqrt<N>(a);
}
// NOLINTEND(portability-simd-intrinsics)

[[nodiscard]] constexpr inline raw_vec_t<4> zero_w(raw_vec_t<4> a) noexcept {
#ifdef __clang__
    constexpr raw_vec_t<4> zero{0.0f, 0.0f, 0.0f, 0.0f};
    return __builtin_shufflevector(a, zero, 0, 1, 2, 7);
#else
    return {a[0], a[1], a[2], 0.0f};
#endif
}

[[nodiscard]] constexpr inline float reduce_add4(raw_vec_t<4> a) noexcept {
    return (a[0] + a[1]) + (a[2] + a[3]);
}

[[nodiscard]] constexpr inline float reduce_add8(raw_vec_t<8> a) noexcept {
    return ((a[0] + a[1]) + (a[2] + a[3])) + ((a[4] + a[5]) + (a[6] + a[7]));
}

[[nodiscard]] inline std::array<float, 3> reduce_add3(raw_vec_t<8> a, raw_vec_t<8> b, raw_vec_t<8> c) noexcept {
#ifdef __clang__
    using vec4_t        = raw_vec_t<4>;
    const vec4_t     a4 = __builtin_shufflevector(a, a, 0, 1, 2, 3) + __builtin_shufflevector(a, a, 4, 5, 6, 7);
    const vec4_t     b4 = __builtin_shufflevector(b, b, 0, 1, 2, 3) + __builtin_shufflevector(b, b, 4, 5, 6, 7);
    const vec4_t     c4 = __builtin_shufflevector(c, c, 0, 1, 2, 3) + __builtin_shufflevector(c, c, 4, 5, 6, 7);
    constexpr vec4_t zero{0.0f, 0.0f, 0.0f, 0.0f};

    const vec4_t ab = __builtin_shufflevector(a4, b4, 0, 4, 1, 5) + __builtin_shufflevector(a4, b4, 2, 6, 3, 7);
    const vec4_t cz = __builtin_shufflevector(c4, zero, 0, 4, 1, 5) + __builtin_shufflevector(c4, zero, 2, 6, 3, 7);

    const vec4_t r = __builtin_shufflevector(ab, cz, 0, 1, 4, 5) + __builtin_shufflevector(ab, cz, 2, 3, 6, 7);
    return {r[0], r[1], r[2]};
#else
    return {reduce_add8(a), reduce_add8(b), reduce_add8(c)};
#endif
}

[[nodiscard]] constexpr inline raw_vec_t<4> cross3(raw_vec_t<4> a, raw_vec_t<4> b) noexcept {
#ifdef __clang__
    const raw_vec_t<4> a_yzx = __builtin_shufflevector(a, a, 1, 2, 0, 3);
    const raw_vec_t<4> b_zxy = __builtin_shufflevector(b, b, 2, 0, 1, 3);
    const raw_vec_t<4> a_zxy = __builtin_shufflevector(a, a, 2, 0, 1, 3);
    const raw_vec_t<4> b_yzx = __builtin_shufflevector(b, b, 1, 2, 0, 3);
    return zero_w((a_yzx * b_zxy) - (a_zxy * b_yzx));
#else
    return {(a[1] * b[2]) - (a[2] * b[1]), (a[2] * b[0]) - (a[0] * b[2]), (a[0] * b[1]) - (a[1] * b[0]), 0.0f};
#endif
}

inline void transpose4x4(raw_vec_t<4>& c0, raw_vec_t<4>& c1, raw_vec_t<4>& c2, raw_vec_t<4>& c3) noexcept {
#ifdef __clang__
    const raw_vec_t<4> t0 = __builtin_shufflevector(c0, c1, 0, 4, 1, 5);
    const raw_vec_t<4> t1 = __builtin_shufflevector(c0, c1, 2, 6, 3, 7);
    const raw_vec_t<4> t2 = __builtin_shufflevector(c2, c3, 0, 4, 1, 5);
    const raw_vec_t<4> t3 = __builtin_shufflevector(c2, c3, 2, 6, 3, 7);

    c0 = __builtin_shufflevector(t0, t2, 0, 1, 4, 5);
    c1 = __builtin_shufflevector(t0, t2, 2, 3, 6, 7);
    c2 = __builtin_shufflevector(t1, t3, 0, 1, 4, 5);
    c3 = __builtin_shufflevector(t1, t3, 2, 3, 6, 7);
#else
    const raw_vec_t<4> r0 = c0;
    const raw_vec_t<4> r1 = c1;
    const raw_vec_t<4> r2 = c2;
    const raw_vec_t<4> r3 = c3;
    c0                    = {r0[0], r1[0], r2[0], r3[0]};
    c1                    = {r0[1], r1[1], r2[1], r3[1]};
    c2                    = {r0[2], r1[2], r2[2], r3[2]};
    c3                    = {r0[3], r1[3], r2[3], r3[3]};
#endif
}

} // namespace shark::simd::detail

export {
namespace shark::simd {

template <std::size_t N> struct alignas(N * sizeof(float)) SimdVec {
    using vec_t = detail::raw_vec_t<N>;
    vec_t v{};

    constexpr inline SimdVec() noexcept = default;
    constexpr explicit inline SimdVec(float s) noexcept : v(detail::RawVec<N>::splat(s)) {}
    constexpr inline SimdVec(float x, float y, float z, float w) noexcept
        requires(N == 4)
        : v{x, y, z, w} {}
    constexpr inline SimdVec(float v0, float v1, float v2, float v3, float v4, float v5, float v6, float v7) noexcept
        requires(N == 8)
        : v{v0, v1, v2, v3, v4, v5, v6, v7} {}
    constexpr explicit inline SimdVec(vec_t vec) noexcept : v(vec) {}

    [[nodiscard]] inline float&       operator[](std::size_t i) noexcept { return reinterpret_cast<float*>(&v)[i]; }
    [[nodiscard]] inline const float& operator[](std::size_t i) const noexcept {
        return reinterpret_cast<const float*>(&v)[i];
    }

    [[nodiscard]] constexpr inline SimdVec operator+() const noexcept { return *this; }
    [[nodiscard]] constexpr inline SimdVec operator-() const noexcept { return SimdVec(-v); }

    [[nodiscard]] constexpr inline SimdVec operator+(const SimdVec& r) const noexcept { return SimdVec(v + r.v); }
    [[nodiscard]] constexpr inline SimdVec operator-(const SimdVec& r) const noexcept { return SimdVec(v - r.v); }
    [[nodiscard]] constexpr inline SimdVec operator*(const SimdVec& r) const noexcept { return SimdVec(v * r.v); }
    [[nodiscard]] constexpr inline SimdVec operator/(const SimdVec& r) const noexcept { return SimdVec(v / r.v); }

    [[nodiscard]] constexpr inline SimdVec operator*(float s) const noexcept {
        return SimdVec(v * detail::RawVec<N>::splat(s));
    }
    [[nodiscard]] constexpr inline SimdVec operator/(float s) const noexcept {
        return SimdVec(v / detail::RawVec<N>::splat(s));
    }

    constexpr inline SimdVec& operator+=(const SimdVec& r) noexcept {
        v += r.v;
        return *this;
    }
    constexpr inline SimdVec& operator-=(const SimdVec& r) noexcept {
        v -= r.v;
        return *this;
    }
    constexpr inline SimdVec& operator*=(const SimdVec& r) noexcept {
        v *= r.v;
        return *this;
    }
    constexpr inline SimdVec& operator/=(const SimdVec& r) noexcept {
        v /= r.v;
        return *this;
    }
};

template <std::size_t N> [[nodiscard]] constexpr inline SimdVec<N> operator*(float s, const SimdVec<N>& r) noexcept {
    return r * s;
}

using float4  = SimdVec<4>;
using float8  = SimdVec<8>;
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

[[nodiscard]] constexpr inline float4 make4(float x, float y, float z, float w) noexcept {
    return {x, y, z, w};
}

template <typename T> [[nodiscard]] inline float8 load8(const T* ptr) noexcept {
    float8 res;
    std::memcpy(&res.v, ptr, sizeof(float) * 8);
    return res;
}

template <typename T> inline void store8(T* ptr, const float8& val) noexcept {
    std::memcpy(ptr, &val.v, sizeof(float) * 8);
}

[[nodiscard]] constexpr inline float4 zero_w(const float4& a) noexcept {
    return float4(detail::zero_w(a.v));
}

[[nodiscard]] constexpr inline float reduce_add(const float4& a) noexcept {
    return detail::reduce_add4(a.v);
}

[[nodiscard]] constexpr inline float reduce_add(const float8& a) noexcept {
    return detail::reduce_add8(a.v);
}

// {sum(a), sum(b), sum(c)} with the shuffle/add work shared through a partial 4x4 transpose.
[[nodiscard]] inline std::array<float, 3> reduce_add3(const float8& a, const float8& b, const float8& c) noexcept {
    return detail::reduce_add3(a.v, b.v, c.v);
}

template <std::size_t N> [[nodiscard]] inline SimdVec<N> sqrt(const SimdVec<N>& a) noexcept {
    return SimdVec<N>(detail::sqrt<N>(a.v));
}

template <std::size_t N> [[nodiscard]] inline SimdVec<N> rsqrt(const SimdVec<N>& a) noexcept {
    return SimdVec<N>(detail::rsqrt<N>(a.v));
}

template <std::size_t N>
[[nodiscard]] inline SimdVec<N> fma(const SimdVec<N>& a, const SimdVec<N>& b, const SimdVec<N>& c) noexcept {
    return SimdVec<N>(detail::fma<N>(a.v, b.v, c.v));
}

// Same semantics as minps/maxps; compiles to them on x86 and to f32x4.pmin/pmax on wasm.
template <std::size_t N> [[nodiscard]] inline SimdVec<N> min(const SimdVec<N>& a, const SimdVec<N>& b) noexcept {
    return SimdVec<N>(a.v < b.v ? a.v : b.v);
}

template <std::size_t N> [[nodiscard]] inline SimdVec<N> max(const SimdVec<N>& a, const SimdVec<N>& b) noexcept {
    return SimdVec<N>(a.v > b.v ? a.v : b.v);
}

template <std::size_t N>
[[nodiscard]] inline SimdVec<N> clamp(const SimdVec<N>& v, const SimdVec<N>& lo, const SimdVec<N>& hi) noexcept {
    return min(max(v, lo), hi);
}

template <std::size_t N> [[nodiscard]] inline SimdVec<N> abs(const SimdVec<N>& a) noexcept {
    return SimdVec<N>(detail::abs<N>(a.v));
}

[[nodiscard]] inline float reduce_max(const float4& a) noexcept {
    return std::max({a[0], a[1], a[2], a[3]});
}

[[nodiscard]] inline float reduce_max(const float8& a) noexcept {
    return std::max({a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]});
}

[[nodiscard]] constexpr inline float dot3(const float4& a, const float4& b) noexcept {
    const float4 prod = a * b;
    return (prod.v[0] + prod.v[1]) + prod.v[2];
}

[[nodiscard]] constexpr inline float dot4(const float4& a, const float4& b) noexcept {
    return reduce_add(a * b);
}

[[nodiscard]] constexpr inline float4 cross3(const float4& a, const float4& b) noexcept {
    return float4(detail::cross3(a.v, b.v));
}

inline void transpose4x4(float4& c0, float4& c1, float4& c2, float4& c3) noexcept {
    detail::transpose4x4(c0.v, c1.v, c2.v, c3.v);
}

} // namespace shark::simd
} // export
