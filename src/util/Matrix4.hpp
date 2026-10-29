#pragma once
#include <array>
#include <cmath>
#include <numbers>
#include <span>

#include "util/SIMD.hpp"
#include "util/Vector.hpp"

struct alignas(16) Matrix4 {
    std::array<float, 16> m{}; // column-major

    [[nodiscard]] static constexpr Matrix4 identity() noexcept {
        Matrix4 r{};
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
        return r;
    }

    [[nodiscard]] std::span<const float, 16> asSpan() const noexcept { return m; }
    [[nodiscard]] const float*               data() const noexcept { return m.data(); }

    [[nodiscard]] Matrix4 operator*(const Matrix4& rhs) const noexcept {
        using namespace shark::simd;
        Matrix4      result{};
        const float4 c0 = load4_aligned(m.data());
        const float4 c1 = load4_aligned(&m[4]);
        const float4 c2 = load4_aligned(&m[8]);
        const float4 c3 = load4_aligned(&m[12]);

        for (std::size_t col = 0; col < 4; ++col) {
            const std::size_t base   = col * 4;
            const float4      resCol = fma(
                c0, float4(rhs.m[base + 0]),
                fma(c1, float4(rhs.m[base + 1]), fma(c2, float4(rhs.m[base + 2]), c3 * rhs.m[base + 3]))
            );
            store4_aligned(&result.m[base], resCol);
        }
        return result;
    }

    [[nodiscard]] Vector4 operator*(const Vector4& v) const noexcept {
        using namespace shark::simd;
        const float4 c0  = load4_aligned(m.data());
        const float4 c1  = load4_aligned(&m[4]);
        const float4 c2  = load4_aligned(&m[8]);
        const float4 c3  = load4_aligned(&m[12]);
        const float4 res = fma(c0, float4(v.x), fma(c1, float4(v.y), fma(c2, float4(v.z), c3 * v.w)));
        return Vector4::fromSimd(res);
    }

    [[nodiscard]] Vector3 transformPoint(const Vector3& p) const noexcept {
        using namespace shark::simd;
        const float4 c0  = load4_aligned(m.data());
        const float4 c1  = load4_aligned(&m[4]);
        const float4 c2  = load4_aligned(&m[8]);
        const float4 c3  = load4_aligned(&m[12]);
        const float4 res = fma(c0, float4(p.x), fma(c1, float4(p.y), fma(c2, float4(p.z), c3)));
        return Vector3::fromSimd(res);
    }

    [[nodiscard]] Vector3 transformVector(const Vector3& v) const noexcept {
        using namespace shark::simd;
        const float4 c0  = load4_aligned(m.data());
        const float4 c1  = load4_aligned(&m[4]);
        const float4 c2  = load4_aligned(&m[8]);
        const float4 res = fma(c0, float4(v.x), fma(c1, float4(v.y), c2 * v.z));
        return Vector3::fromSimd(res);
    }

    [[nodiscard]] Vector3 transformVectorTransposed(const Vector3& v) const noexcept {
        using namespace shark::simd;
        float4 c0 = load4_aligned(m.data());
        float4 c1 = load4_aligned(&m[4]);
        float4 c2 = load4_aligned(&m[8]);
        float4 c3(0.0f);
        transpose4x4(c0, c1, c2, c3);
        const float4 res = fma(c0, float4(v.x), fma(c1, float4(v.y), c2 * v.z));
        return Vector3::fromSimd(res);
    }

    struct PerspectiveDesc {
        float fovDeg{};
        float aspect{};
        float nearZ{};
        float farZ{};
    };

    [[nodiscard]] static Matrix4 perspective(const PerspectiveDesc& desc) noexcept {
        Matrix4 r{};
        float   f = 1.0f / std::tan((desc.fovDeg * 0.5f) * std::numbers::pi_v<float> / 180.0f);

        r.m[0]  = f / desc.aspect;
        r.m[5]  = f;
        r.m[10] = (desc.farZ + desc.nearZ) / (desc.nearZ - desc.farZ);
        r.m[11] = -1.0f;
        r.m[14] = (2.0f * desc.farZ * desc.nearZ) / (desc.nearZ - desc.farZ);
        return r;
    }

    struct FrustrumDesc {
        float left;
        float right;
        float bottom;
        float top;
        float nearZ;
        float farZ;
    };

    [[nodiscard]] static Matrix4 frustum(const FrustrumDesc& desc) noexcept {
        Matrix4 r{};
        r.m[0]  = (2.0f * desc.nearZ) / (desc.right - desc.left);
        r.m[5]  = (2.0f * desc.nearZ) / (desc.top - desc.bottom);
        r.m[8]  = (desc.right + desc.left) / (desc.right - desc.left);
        r.m[9]  = (desc.top + desc.bottom) / (desc.top - desc.bottom);
        r.m[10] = -(desc.farZ + desc.nearZ) / (desc.farZ - desc.nearZ);
        r.m[11] = -1.0f;
        r.m[14] = -(2.0f * desc.farZ * desc.nearZ) / (desc.farZ - desc.nearZ);
        return r;
    }

    struct OrthoDesc {
        float left;
        float right;
        float bottom;
        float top;
        float nearZ;
        float farZ;
    };

    [[nodiscard]] static Matrix4 ortho(const OrthoDesc& desc) noexcept {
        Matrix4 r{};
        r.m[0]  = 2.0f / (desc.right - desc.left);
        r.m[5]  = 2.0f / (desc.top - desc.bottom);
        r.m[10] = -2.0f / (desc.farZ - desc.nearZ);
        r.m[12] = -(desc.right + desc.left) / (desc.right - desc.left);
        r.m[13] = -(desc.top + desc.bottom) / (desc.top - desc.bottom);
        r.m[14] = -(desc.farZ + desc.nearZ) / (desc.farZ - desc.nearZ);
        r.m[15] = 1.0f;
        return r;
    }

    [[nodiscard]] static Matrix4 translate(const Vector3& t) noexcept {
        Matrix4 r = identity();
        shark::simd::store4_aligned(&r.m[12], shark::simd::float4(t.x, t.y, t.z, 1.0f));
        return r;
    }

    [[nodiscard]] static Matrix4 translateScale(const Vector3& t, const Vector3& s) noexcept {
        Matrix4 r{};
        r.m[0]  = s.x;
        r.m[5]  = s.y;
        r.m[10] = s.z;
        shark::simd::store4_aligned(&r.m[12], shark::simd::float4(t.x, t.y, t.z, 1.0f));
        return r;
    }

    [[nodiscard]] static Matrix4 translateScale(const Vector3& t, float s) noexcept {
        return translateScale(t, Vector3{s, s, s});
    }

    [[nodiscard]] static Matrix4 rotateX(float angleDeg) noexcept {
        Matrix4 r   = identity();
        float   rad = angleDeg * std::numbers::pi_v<float> / 180.0f;
        float   c   = std::cos(rad);
        float   s   = std::sin(rad);
        r.m[5]      = c;
        r.m[6]      = s;
        r.m[9]      = -s;
        r.m[10]     = c;
        return r;
    }

    [[nodiscard]] static Matrix4 rotateY(float angleDeg) noexcept {
        Matrix4 r   = identity();
        float   rad = angleDeg * std::numbers::pi_v<float> / 180.0f;
        float   c   = std::cos(rad);
        float   s   = std::sin(rad);
        r.m[0]      = c;
        r.m[2]      = -s;
        r.m[8]      = s;
        r.m[10]     = c;
        return r;
    }

    [[nodiscard]] static Matrix4 rotateZ(float angleDeg) noexcept {
        Matrix4 r   = identity();
        float   rad = angleDeg * std::numbers::pi_v<float> / 180.0f;
        float   c   = std::cos(rad);
        float   s   = std::sin(rad);
        r.m[0]      = c;
        r.m[1]      = s;
        r.m[4]      = -s;
        r.m[5]      = c;
        return r;
    }

    [[nodiscard]] static Matrix4 scale(const Vector3& s) noexcept {
        Matrix4 r = identity();
        r.m[0]    = s.x;
        r.m[5]    = s.y;
        r.m[10]   = s.z;
        return r;
    }

    [[nodiscard]] static Matrix4 scale(float s) noexcept { return scale(Vector3{s, s, s}); }

    // T * Rz * Ry * Rx * S
    [[nodiscard]] static Matrix4
    trs(const Vector3& translation, const Vector3& rotationDeg, const Vector3& scaleVec) noexcept {
        using namespace shark::simd;
        constexpr float degToRad = std::numbers::pi_v<float> / 180.0f;
        const float4    rads     = rotationDeg.toSimd() * degToRad;

        const float cx = std::cos(rads[0]);
        const float sx = std::sin(rads[0]);
        const float cy = std::cos(rads[1]);
        const float sy = std::sin(rads[1]);
        const float cz = std::cos(rads[2]);
        const float sz = std::sin(rads[2]);

        const float4 col0 = float4(cz * cy, sz * cy, -sy, 0.0f) * scaleVec.x;
        const float4 col1 = float4((cz * sy * sx) - (sz * cx), (sz * sy * sx) + (cz * cx), cy * sx, 0.0f) * scaleVec.y;
        const float4 col2 = float4((cz * sy * cx) + (sz * sx), (sz * sy * cx) - (cz * sx), cy * cx, 0.0f) * scaleVec.z;
        const float4 col3 = float4(translation.x, translation.y, translation.z, 1.0f);

        Matrix4 r{};
        store4_aligned(r.m.data(), col0);
        store4_aligned(&r.m[4], col1);
        store4_aligned(&r.m[8], col2);
        store4_aligned(&r.m[12], col3);
        return r;
    }

    [[nodiscard]] static Matrix4 lookAt(const Vector3& eye, const Vector3& target, const Vector3& up) noexcept {
        const Vector3 f = normalize(target - eye);
        const Vector3 s = normalize(cross(f, up));
        const Vector3 u = cross(s, f);

        Matrix4 r = identity();

        r.m[0]  = s.x;
        r.m[4]  = s.y;
        r.m[8]  = s.z;
        r.m[1]  = u.x;
        r.m[5]  = u.y;
        r.m[9]  = u.z;
        r.m[2]  = -f.x;
        r.m[6]  = -f.y;
        r.m[10] = -f.z;

        r.m[12] = -s.dot(eye);
        r.m[13] = -u.dot(eye);
        r.m[14] = f.dot(eye);

        return r;
    }
};

static_assert(sizeof(Matrix4) == 64 && alignof(Matrix4) == 16, "Matrix4 must be 64 bytes and 16-byte aligned for SIMD");
