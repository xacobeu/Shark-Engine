#pragma once
#include <array>
#include <cmath>
#include <numbers>
#include <span>

#include "util/Vector.hpp"
struct Matrix4 {
    std::array<float, 16> m{}; // column-major

    static Matrix4 identity() {
        Matrix4 r{};
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
        return r;
    }

    [[nodiscard]] std::span<const float, 16> asSpan() const noexcept { return m; }
    [[nodiscard]] const float*               data() const noexcept { return m.data(); }

    Matrix4 operator*(const Matrix4& rhs) const {
        Matrix4 result{};
        for (std::size_t col = 0; col < 4; ++col) {
            for (std::size_t row = 0; row < 4; ++row) {
                result.m.at((col * 4uz) + row) = (m.at((0uz * 4uz) + row) * rhs.m.at((col * 4uz) + 0uz)) +
                                                 (m.at((1uz * 4uz) + row) * rhs.m.at((col * 4uz) + 1uz)) +
                                                 (m.at((2uz * 4uz) + row) * rhs.m.at((col * 4uz) + 2uz)) +
                                                 (m.at((3uz * 4uz) + row) * rhs.m.at((col * 4uz) + 3uz));
            }
        }
        return result;
    }

    struct PerspectiveDesc {
        float fovDeg{};
        float aspect{};
        float nearZ{};
        float farZ{};
    };

    static Matrix4 perspective(const PerspectiveDesc& desc) {
        Matrix4 r{};
        float   f = 1.0f / std::tan((desc.fovDeg * 0.5f) * std::numbers::pi_v<float> / 180.0f);

        r.m.at(0)  = f / desc.aspect;
        r.m.at(5)  = f;
        r.m.at(10) = (desc.farZ + desc.nearZ) / (desc.nearZ - desc.farZ);
        r.m.at(11) = -1.0f;
        r.m.at(14) = (2.0f * desc.farZ * desc.nearZ) / (desc.nearZ - desc.farZ);
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

    static Matrix4 frustum(const FrustrumDesc& desc) {
        Matrix4 r{};
        r.m.at(0)  = (2.0f * desc.nearZ) / (desc.right - desc.left);
        r.m.at(5)  = (2.0f * desc.nearZ) / (desc.top - desc.bottom);
        r.m.at(8)  = (desc.right + desc.left) / (desc.right - desc.left);
        r.m.at(9)  = (desc.top + desc.bottom) / (desc.top - desc.bottom);
        r.m.at(10) = -(desc.farZ + desc.nearZ) / (desc.farZ - desc.nearZ);
        r.m.at(11) = -1.0f;
        r.m[14]    = -(2.0f * desc.farZ * desc.nearZ) / (desc.farZ - desc.nearZ);
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

    static Matrix4 ortho(const OrthoDesc& desc) {
        Matrix4 r{};
        r.m.at(0)  = 2.0f / (desc.right - desc.left);
        r.m.at(5)  = 2.0f / (desc.top - desc.bottom);
        r.m.at(10) = -2.0f / (desc.farZ - desc.nearZ);
        r.m.at(12) = -(desc.right + desc.left) / (desc.right - desc.left);
        r.m.at(13) = -(desc.top + desc.bottom) / (desc.top - desc.bottom);
        r.m.at(14) = -(desc.farZ + desc.nearZ) / (desc.farZ - desc.nearZ);
        r.m.at(15) = 1.0f;
        return r;
    }

    static Matrix4 translate(const Vector3& t) {
        Matrix4 r  = identity();
        r.m.at(12) = t.x;
        r.m.at(13) = t.y;
        r.m.at(14) = t.z;
        return r;
    }

    static Matrix4 rotateX(float angleDeg) {
        Matrix4 r   = identity();
        float   rad = angleDeg * std::numbers::pi_v<float> / 180.0f;
        float   c   = std::cos(rad);
        float   s   = std::sin(rad);
        r.m.at(5)   = c;
        r.m.at(6)   = s;
        r.m.at(9)   = -s;
        r.m.at(10)  = c;
        return r;
    }

    static Matrix4 rotateY(float angleDeg) {
        Matrix4 r   = identity();
        float   rad = angleDeg * std::numbers::pi_v<float> / 180.0f;
        float   c   = std::cos(rad);
        float   s   = std::sin(rad);
        r.m.at(0)   = c;
        r.m.at(2)   = -s;
        r.m.at(8)   = s;
        r.m.at(10)  = c;
        return r;
    }

    static Matrix4 rotateZ(float angleDeg) {
        Matrix4 r   = identity();
        float   rad = angleDeg * std::numbers::pi_v<float> / 180.0f;
        float   c   = std::cos(rad);
        float   s   = std::sin(rad);
        r.m.at(0)   = c;
        r.m.at(1)   = s;
        r.m.at(4)   = -s;
        r.m.at(5)   = c;
        return r;
    }

    static Matrix4 scale(const Vector3& s) {
        Matrix4 r  = identity();
        r.m.at(0)  = s.x;
        r.m.at(5)  = s.y;
        r.m.at(10) = s.z;
        return r;
    }

    static Vector3 normalize(const Vector3& v) {
        float l = sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
        return Vector3{.x = v.x / l, .y = v.y / l, .z = v.z / l};
    }

    static Vector3 cross(const Vector3& a, const Vector3& b) {
        return Vector3{.x = (a.y * b.z) - (a.z * b.y), .y = (a.z * b.x) - (a.x * b.z), .z = (a.x * b.y) - (a.y * b.x)};
    }

    static Matrix4 lookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
        Vector3 f = normalize(Vector3{.x = target.x - eye.x, .y = target.y - eye.y, .z = target.z - eye.z});
        Vector3 s = normalize(cross(f, up));
        Vector3 u = cross(s, f);

        Matrix4 r = identity();

        r.m.at(0)  = s.x;
        r.m.at(4)  = s.y;
        r.m.at(8)  = s.z;
        r.m.at(1)  = u.x;
        r.m.at(5)  = u.y;
        r.m.at(9)  = u.z;
        r.m.at(2)  = -f.x;
        r.m.at(6)  = -f.y;
        r.m.at(10) = -f.z;

        r.m.at(12) = -((s.x * eye.x) + (s.y * eye.y) + (s.z * eye.z));
        r.m.at(13) = -((u.x * eye.x) + (u.y * eye.y) + (u.z * eye.z));
        r.m.at(14) = ((f.x * eye.x) + (f.y * eye.y) + (f.z * eye.z));

        return r;
    }
};
