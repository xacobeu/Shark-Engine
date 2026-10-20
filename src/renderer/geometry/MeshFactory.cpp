#include "renderer/geometry/MeshFactory.hpp"

#include <array>
#include <cmath>
#include <numbers>

#include "renderer/geometry/Vertex.hpp"

Mesh::MeshData MeshFactory::createCubeMesh() {
    Mesh::MeshData mesh;

    static constexpr std::array<std::array<Vector3, 4>, 6> positions = {
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

    static constexpr std::array<Vector3, 6> normals = {
        Vector3{.x = 0.0f, .y = 0.0f, .z = 1.0f}, Vector3{.x = 0.0f, .y = 0.0f, .z = -1.0f},
        Vector3{.x = 0.0f, .y = 1.0f, .z = 0.0f}, Vector3{.x = 0.0f, .y = -1.0f, .z = 0.0f},
        Vector3{.x = 1.0f, .y = 0.0f, .z = 0.0f}, Vector3{.x = -1.0f, .y = 0.0f, .z = 0.0f},
    };

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

    // Build vertices
    for (std::size_t face = 0; face < positions.size(); ++face) {
        for (std::size_t i = 0; i < positions.at(face).size(); ++i) {
            mesh.vertices.push_back(
                Vertex{
                    .position = positions.at(face).at(i),
                    .normal   = normals.at(face),
                    .uv       = uvs.at(i),
                    .color    = colors.at(face),
                }
            );
        }
    }

    // Triangles
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