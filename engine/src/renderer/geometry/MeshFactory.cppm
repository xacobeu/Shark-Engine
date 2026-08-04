module;
#include <numbers>
#include <cmath>

export module SharkEngine:MeshFactory;
import :Math;
import :Vertex;
import :Mesh;

export class MeshFactory {
public:
    static Mesh::MeshData createCubeMesh() {
        Mesh::MeshData mesh;

        static constexpr Vector3 positions[6][4] = {
            { {-1.0f,-1.0f, 1.0f}, { 1.0f,-1.0f, 1.0f}, { 1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f} }, // Front
            { {-1.0f,-1.0f,-1.0f}, {-1.0f, 1.0f,-1.0f}, { 1.0f, 1.0f,-1.0f}, { 1.0f,-1.0f,-1.0f} }, // Back
            { {-1.0f, 1.0f,-1.0f}, {-1.0f, 1.0f, 1.0f}, { 1.0f, 1.0f, 1.0f}, { 1.0f, 1.0f,-1.0f} }, // Top
            { {-1.0f,-1.0f,-1.0f}, { 1.0f,-1.0f,-1.0f}, { 1.0f,-1.0f, 1.0f}, {-1.0f,-1.0f, 1.0f} }, // Bottom
            { { 1.0f,-1.0f,-1.0f}, { 1.0f, 1.0f,-1.0f}, { 1.0f, 1.0f, 1.0f}, { 1.0f,-1.0f, 1.0f} }, // Right
            { {-1.0f,-1.0f,-1.0f}, {-1.0f,-1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f,-1.0f} }  // Left
        };

        static constexpr Vector3 normals[6] = {
            {0.0f,0.0f,1.0f},  {0.0f,0.0f,-1.0f}, {0.0f,1.0f,0.0f},
            {0.0f,-1.0f,0.0f}, {1.0f,0.0f,0.0f},  {-1.0f,0.0f,0.0f}
        };

        static constexpr Vector2 uvs[4] = {
            {0.0f,0.0f}, {1.0f,0.0f}, {1.0f,1.0f}, {0.0f,1.0f}
        };

        static constexpr Vector3 colors[6] = {
            {  1.0f, 0.2f, 0.2f},
            {0.2f,   1.0f, 0.2f},
            {0.2f, 0.2f,   1.0f},
            {  1.0f,   1.0f, 0.2f},
            {  1.0f, 0.2f,   1.0f},
            {0.2f,   1.0f,   1.0f}
        };

        // Build vertices
        for (int face = 0; face < 6; face++) {
            for (int i = 0; i < 4; i++) {
                mesh.vertices.push_back(Vertex{
                    positions[face][i],
                    normals[face],
                    uvs[i],
                    colors[face]
                });
            }
        }

        // Triangles
        for (int face = 0; face < 6; face++) {
            int base = face * 4;
            mesh.triangles.insert(mesh.triangles.end(), {
                base, base+1, base+2,
                base, base+2, base+3
            });
        }

        return mesh;
    }

    static Mesh::MeshData createSphereMesh(const int resolution, const Vector3& color) {
        Mesh::MeshData mesh;

        // Generate UV sphere
        for (int lat = 0; lat <= resolution; lat++) {
            for (int lon = 0; lon <= resolution; lon++) {
                float theta = float(lat) / resolution * std::numbers::pi_v<float>;
                float phi = float(lon) / resolution * 2 * std::numbers::pi_v<float>;

                // Spherical coordinates to cartesian
                Vector3 pointOnSphere = {
                    std::sin(theta) * std::cos(phi),
                    std::cos(theta),
                    std::sin(theta) * std::sin(phi)
                };

                // UV coordinates
                Vector2 uv = {
                    1.0f - float(lon) / resolution,
                    float(lat) / resolution
                };

                mesh.vertices.push_back(Vertex{
                    pointOnSphere,
                    normalize(pointOnSphere),
                    uv,
                    color
                });
            }
        }

        // Generate triangles
        for (int lat = 0; lat < resolution; lat++) {
            for (int lon = 0; lon < resolution; lon++) {
                int current = lat * (resolution + 1) + lon;
                int next = current + resolution + 1;

                // Two triangles per quad
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

    static Mesh cube() { return Mesh(createCubeMesh()); }
    static Mesh sphere(int resolution, const Vector3& color) { return Mesh(createSphereMesh(resolution, color)); }
};
