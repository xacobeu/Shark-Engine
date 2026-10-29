export module shark:renderer.geometry.Mesh;

import std;

import :renderer.geometry.Vertex;
import :util.Matrix4;
import :util.Vector;

export namespace shark {
class Mesh {
public:
    struct MeshData {
        std::vector<Vertex> vertices;
        std::vector<int>    triangles;
    };

    Mesh() = default;
    inline explicit Mesh(MeshData data) : data(std::move(data)) {}
    [[nodiscard]] inline const MeshData& getData() const { return data; }

    Mesh& translate(const Vector3& t);

    Mesh& scale(const Vector3& s);
    Mesh& scale(float s);

    Mesh& rotateX(float angleDeg);
    Mesh& rotateY(float angleDeg);
    Mesh& rotateZ(float angleDeg);
    Mesh& transform(const Matrix4& mat);

private:
    MeshData data;
};

} // namespace shark
