export module shark:renderer.geometry.MeshFactory;

import :renderer.geometry.Mesh;

export namespace shark {
class MeshFactory {
public:
    static Mesh::MeshData createCubeMesh();
    static Mesh::MeshData createSphereMesh(int resolution, const Vector3& color);
    static Mesh::MeshData
    createBoxFrameMesh(const Vector3& size, float thickness = 0.04f, const Vector3& color = {0.8f, 0.9f, 1.0f});

    static inline Mesh cube() { return Mesh(createCubeMesh()); }
    static inline Mesh sphere(int resolution, const Vector3& color) {
        return Mesh(createSphereMesh(resolution, color));
    }
    static inline Mesh
    boxFrame(const Vector3& size, float thickness = 0.04f, const Vector3& color = {0.8f, 0.9f, 1.0f}) {
        return Mesh(createBoxFrameMesh(size, thickness, color));
    }
};

} // namespace shark
