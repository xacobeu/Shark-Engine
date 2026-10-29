module shark:renderer.geometry.Mesh.impl;

import std;

import :renderer.geometry.Mesh;
import :renderer.geometry.Vertex;
import :util.Execution;
import :util.Matrix4;
import :util.Vector;

namespace shark {

Mesh& Mesh::translate(const Vector3& t) {
    shark::exec::for_each(shark::exec::unseq, data.vertices, [&](Vertex& v) { v.position += t; });
    return *this;
}

Mesh& Mesh::scale(const Vector3& s) {
    const Vector3 invS = Vector3{1.0f, 1.0f, 1.0f} / s;
    shark::exec::for_each(shark::exec::unseq, data.vertices, [&](Vertex& v) {
        v.position *= s;
        v.normal = normalize(v.normal * invS);
    });
    return *this;
}

Mesh& Mesh::scale(float s) {
    shark::exec::for_each(shark::exec::unseq, data.vertices, [&](Vertex& v) { v.position *= s; });
    return *this;
}

Mesh& Mesh::rotateX(float angleDeg) {
    const Matrix4 rot = Matrix4::rotateX(angleDeg);
    shark::exec::for_each(shark::exec::unseq, data.vertices, [&](Vertex& v) {
        v.position = rot.transformVector(v.position);
        v.normal   = rot.transformVector(v.normal);
    });
    return *this;
}

Mesh& Mesh::rotateY(float angleDeg) {
    const Matrix4 rot = Matrix4::rotateY(angleDeg);
    shark::exec::for_each(shark::exec::unseq, data.vertices, [&](Vertex& v) {
        v.position = rot.transformVector(v.position);
        v.normal   = rot.transformVector(v.normal);
    });
    return *this;
}

Mesh& Mesh::rotateZ(float angleDeg) {
    const Matrix4 rot = Matrix4::rotateZ(angleDeg);
    shark::exec::for_each(shark::exec::unseq, data.vertices, [&](Vertex& v) {
        v.position = rot.transformVector(v.position);
        v.normal   = rot.transformVector(v.normal);
    });
    return *this;
}

Mesh& Mesh::transform(const Matrix4& mat) {
    shark::exec::for_each(shark::exec::unseq, data.vertices, [&](Vertex& v) {
        v.position = mat.transformPoint(v.position);
        v.normal   = normalize(mat.transformVector(v.normal));
    });
    return *this;
}

} // namespace shark
