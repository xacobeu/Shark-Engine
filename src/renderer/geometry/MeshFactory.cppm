export module shark:renderer.geometry.MeshFactory;

import std;

import :renderer.geometry.Mesh;
import :renderer.geometry.Vertex;
import :util.Vector;

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

namespace shark {

namespace {

constexpr std::array<std::array<Vector3, 4>, 6> CUBE_FACE_OFFSETS = {
    // Front
    std::array<Vector3, 4>{
        Vector3{.x = -1.0f, .y = -1.0f, .z = 1.0f},
        Vector3{.x = 1.0f, .y = -1.0f, .z = 1.0f},
        Vector3{.x = 1.0f, .y = 1.0f, .z = 1.0f},
        Vector3{.x = -1.0f, .y = 1.0f, .z = 1.0f},
    },
    // Back
    std::array<Vector3, 4>{
        Vector3{.x = -1.0f, .y = -1.0f, .z = -1.0f},
        Vector3{.x = -1.0f, .y = 1.0f, .z = -1.0f},
        Vector3{.x = 1.0f, .y = 1.0f, .z = -1.0f},
        Vector3{.x = 1.0f, .y = -1.0f, .z = -1.0f},
    },
    // Top
    std::array<Vector3, 4>{
        Vector3{.x = -1.0f, .y = 1.0f, .z = -1.0f},
        Vector3{.x = -1.0f, .y = 1.0f, .z = 1.0f},
        Vector3{.x = 1.0f, .y = 1.0f, .z = 1.0f},
        Vector3{.x = 1.0f, .y = 1.0f, .z = -1.0f},
    },
    // Bottom
    std::array<Vector3, 4>{
        Vector3{.x = -1.0f, .y = -1.0f, .z = -1.0f},
        Vector3{.x = 1.0f, .y = -1.0f, .z = -1.0f},
        Vector3{.x = 1.0f, .y = -1.0f, .z = 1.0f},
        Vector3{.x = -1.0f, .y = -1.0f, .z = 1.0f},
    },
    // Right
    std::array<Vector3, 4>{
        Vector3{.x = 1.0f, .y = -1.0f, .z = -1.0f},
        Vector3{.x = 1.0f, .y = 1.0f, .z = -1.0f},
        Vector3{.x = 1.0f, .y = 1.0f, .z = 1.0f},
        Vector3{.x = 1.0f, .y = -1.0f, .z = 1.0f},
    },
    // Left
    std::array<Vector3, 4>{
        Vector3{.x = -1.0f, .y = -1.0f, .z = -1.0f},
        Vector3{.x = -1.0f, .y = -1.0f, .z = 1.0f},
        Vector3{.x = -1.0f, .y = 1.0f, .z = 1.0f},
        Vector3{.x = -1.0f, .y = 1.0f, .z = -1.0f},
    },
};

constexpr std::array<Vector3, 6> CUBE_FACE_NORMALS = {
    Vector3{.x = 0.0f, .y = 0.0f, .z = 1.0f}, Vector3{.x = 0.0f, .y = 0.0f, .z = -1.0f},
    Vector3{.x = 0.0f, .y = 1.0f, .z = 0.0f}, Vector3{.x = 0.0f, .y = -1.0f, .z = 0.0f},
    Vector3{.x = 1.0f, .y = 0.0f, .z = 0.0f}, Vector3{.x = -1.0f, .y = 0.0f, .z = 0.0f},
};

} // namespace

Mesh::MeshData MeshFactory::createCubeMesh() {
    Mesh::MeshData mesh;

    static constexpr std::array<Vector2, 4> uvs = {
        Vector2{.x = 0.0f, .y = 0.0f},
        Vector2{.x = 1.0f, .y = 0.0f},
        Vector2{.x = 1.0f, .y = 1.0f},
        Vector2{.x = 0.0f, .y = 1.0f},
    };

    static constexpr std::array<Vector3, 6> colors = {
        Vector3{.x = 1.0f, .y = 0.2f, .z = 0.2f}, Vector3{.x = 0.2f, .y = 1.0f, .z = 0.2f},
        Vector3{.x = 0.2f, .y = 0.2f, .z = 1.0f}, Vector3{.x = 1.0f, .y = 1.0f, .z = 0.2f},
        Vector3{.x = 1.0f, .y = 0.2f, .z = 1.0f}, Vector3{.x = 0.2f, .y = 1.0f, .z = 1.0f},
    };

    for (std::size_t face = 0; face < CUBE_FACE_OFFSETS.size(); ++face) {
        for (std::size_t i = 0; i < CUBE_FACE_OFFSETS.at(face).size(); ++i) {
            mesh.vertices.push_back(
                Vertex{
                    .position = CUBE_FACE_OFFSETS.at(face).at(i),
                    .normal   = CUBE_FACE_NORMALS.at(face),
                    .uv       = uvs.at(i),
                    .color    = colors.at(face),
                }
            );
        }
    }

    for (int face = 0; face < 6; face++) {
        int base = face * 4;
        mesh.triangles.insert(mesh.triangles.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    }

    return mesh;
}

Mesh::MeshData MeshFactory::createSphereMesh(const int resolution, const Vector3& color) {
    Mesh::MeshData mesh;

    // Generate UV sphere
    for (int lat = 0; lat <= resolution; lat++) {
        for (int lon = 0; lon <= resolution; lon++) {
            float theta = static_cast<float>(lat) / static_cast<float>(resolution) * std::numbers::pi_v<float>;
            float phi   = static_cast<float>(lon) / static_cast<float>(resolution) * 2 * std::numbers::pi_v<float>;

            // Spherical coordinates to cartesian
            Vector3 pointOnSphere = {
                .x = std::sin(theta) * std::cos(phi),
                .y = std::cos(theta),
                .z = std::sin(theta) * std::sin(phi),
            };

            // UV coordinates
            Vector2 uv = {
                .x = 1.0f - (static_cast<float>(lon) / static_cast<float>(resolution)),
                .y = static_cast<float>(lat) / static_cast<float>(resolution),
            };

            mesh.vertices.push_back(
                Vertex{.position = pointOnSphere, .normal = normalize(pointOnSphere), .uv = uv, .color = color}
            );
        }
    }

    // Generate triangles
    for (int lat = 0; lat < resolution; lat++) {
        for (int lon = 0; lon < resolution; lon++) {
            int current = (lat * (resolution + 1)) + lon;
            int next    = current + resolution + 1;

            mesh.triangles.push_back(current);
            mesh.triangles.push_back(next);
            mesh.triangles.push_back(current + 1);

            mesh.triangles.push_back(current + 1);
            mesh.triangles.push_back(next);
            mesh.triangles.push_back(next + 1);
        }
    }

    return mesh;
}

Mesh::MeshData MeshFactory::createBoxFrameMesh(const Vector3& size, float thickness, const Vector3& color) {
    Mesh::MeshData mesh;
    float          halfX = size.x * 0.5f;
    float          halfY = size.y * 0.5f;
    float          halfZ = size.z * 0.5f;
    float          ht    = thickness * 0.5f;

    auto addCuboid = [&](Vector3 center, Vector3 extents, Vector3 barColor) {
        for (std::size_t f = 0; f < CUBE_FACE_OFFSETS.size(); ++f) {
            auto faceBase = static_cast<int>(mesh.vertices.size());
            for (std::size_t v = 0; v < CUBE_FACE_OFFSETS[f].size(); ++v) {
                Vector3 p = center + (CUBE_FACE_OFFSETS[f][v] * extents);
                mesh.vertices.push_back(
                    Vertex{.position = p, .normal = CUBE_FACE_NORMALS[f], .uv = Vector2{0.0f, 0.0f}, .color = barColor}
                );
            }
            mesh.triangles.push_back(faceBase + 0);
            mesh.triangles.push_back(faceBase + 1);
            mesh.triangles.push_back(faceBase + 2);
            mesh.triangles.push_back(faceBase + 2);
            mesh.triangles.push_back(faceBase + 3);
            mesh.triangles.push_back(faceBase + 0);
        }
    };

    // 4 edges along X
    addCuboid({0.0f, -halfY, -halfZ}, {halfX, ht, ht}, color);
    addCuboid({0.0f, halfY, -halfZ}, {halfX, ht, ht}, color);
    addCuboid({0.0f, -halfY, halfZ}, {halfX, ht, ht}, color);
    addCuboid({0.0f, halfY, halfZ}, {halfX, ht, ht}, color);

    // 4 edges along Y
    addCuboid({-halfX, 0.0f, -halfZ}, {ht, halfY, ht}, color);
    addCuboid({halfX, 0.0f, -halfZ}, {ht, halfY, ht}, color);
    addCuboid({-halfX, 0.0f, halfZ}, {ht, halfY, ht}, color);
    addCuboid({halfX, 0.0f, halfZ}, {ht, halfY, ht}, color);

    // 4 edges along Z
    addCuboid({-halfX, -halfY, 0.0f}, {ht, ht, halfZ}, color);
    addCuboid({halfX, -halfY, 0.0f}, {ht, ht, halfZ}, color);
    addCuboid({-halfX, halfY, 0.0f}, {ht, ht, halfZ}, color);
    addCuboid({halfX, halfY, 0.0f}, {ht, ht, halfZ}, color);

    return mesh;
}

} // namespace shark
