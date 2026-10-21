#pragma once

#include "renderer/geometry/Mesh.hpp"

class MeshFactory {
public:
    static Mesh::MeshData createCubeMesh();
    static Mesh::MeshData createSphereMesh(int resolution, const Vector3& color);
    static Mesh::MeshData
    createBoxFrameMesh(const Vector3& size, float thickness = 0.04f, const Vector3& color = {0.8f, 0.9f, 1.0f});

    static Mesh cube() { return Mesh(createCubeMesh()); }
    static Mesh sphere(int resolution, const Vector3& color) { return Mesh(createSphereMesh(resolution, color)); }
    static Mesh boxFrame(const Vector3& size, float thickness = 0.04f, const Vector3& color = {0.8f, 0.9f, 1.0f}) {
        return Mesh(createBoxFrameMesh(size, thickness, color));
    }
};
