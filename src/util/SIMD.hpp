#pragma once

#include <cstddef>
#include <cstring>
#include <version>

#if __has_include(<experimental/simd>)
#include <experimental/simd>
#endif

namespace shark::simd {

#if defined(__cpp_lib_experimental_parallel_simd)

namespace stdx = std::experimental;

using float4  = stdx::fixed_size_simd<float, 4>;
using float8  = stdx::fixed_size_simd<float, 8>;
using float_v = stdx::native_simd<float>;

template <typename T> [[nodiscard]] inline float4 load4(const T* ptr) noexcept {
    float4 v;
    v.copy_from(ptr, stdx::element_aligned);
    return v;
}

template <typename T> inline void store4(T* ptr, const float4& v) noexcept {
    v.copy_to(ptr, stdx::element_aligned);
}

[[nodiscard]] inline float dot4(const float4& a, const float4& b) noexcept {
    float4 prod = a * b;
    return prod[0] + prod[1] + prod[2] + prod[3];
}

#else

// Portable SIMD vector for WebAssembly / Libc++ using compiler vector extensions
struct alignas(16) float4 {
    typedef float vec4_t __attribute__((vector_size(16)));
    vec4_t        v{0.0f, 0.0f, 0.0f, 0.0f};

    float4() = default;
    float4(float s) noexcept : v{s, s, s, s} {}
    float4(float x, float y, float z, float w) noexcept : v{x, y, z, w} {}
    float4(vec4_t vec) noexcept : v(vec) {}

    [[nodiscard]] float&       operator[](std::size_t i) noexcept { return reinterpret_cast<float*>(&v)[i]; }
    [[nodiscard]] const float& operator[](std::size_t i) const noexcept {
        return reinterpret_cast<const float*>(&v)[i];
    }

    [[nodiscard]] float4 operator+(const float4& r) const noexcept { return float4(v + r.v); }
    [[nodiscard]] float4 operator-(const float4& r) const noexcept { return float4(v - r.v); }
    [[nodiscard]] float4 operator*(const float4& r) const noexcept { return float4(v * r.v); }
    [[nodiscard]] float4 operator/(const float4& r) const noexcept { return float4(v / r.v); }

    [[nodiscard]] float4 operator*(float s) const noexcept {
        vec4_t sv{s, s, s, s};
        return float4(v * sv);
    }
    [[nodiscard]] float4 operator/(float s) const noexcept {
        vec4_t sv{s, s, s, s};
        return float4(v / sv);
    }
};

using float_v = float4;

template <typename T> [[nodiscard]] inline float4 load4(const T* ptr) noexcept {
    float4 res;
    std::memcpy(&res.v, ptr, sizeof(float) * 4);
    return res;
}

template <typename T> inline void store4(T* ptr, const float4& val) noexcept {
    std::memcpy(ptr, &val.v, sizeof(float) * 4);
}

[[nodiscard]] inline float dot4(const float4& a, const float4& b) noexcept {
    float4 prod = a * b;
    return prod[0] + prod[1] + prod[2] + prod[3];
}

#endif

} // namespace shark::simd
