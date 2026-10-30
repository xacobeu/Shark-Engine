module;

#include <cstddef>

export module shark:renderer.geometry.Vertex;

import :renderer.backends.IPipeline;
import :util.Vector;

export namespace shark {
struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
    Vector3 color;

    // Locations 0-3: position, normal, uv, color
    [[nodiscard]] static VertexBufferLayout layout() {
        return {
            .stride     = sizeof(Vertex),
            .stepMode   = VertexStepMode::Vertex,
            .attributes = {
                {.location = 0, .format = VertexFormat::Float3, .offset = offsetof(Vertex, position)},
                {.location = 1, .format = VertexFormat::Float3, .offset = offsetof(Vertex, normal)},
                {.location = 2, .format = VertexFormat::Float2, .offset = offsetof(Vertex, uv)},
                {.location = 3, .format = VertexFormat::Float3, .offset = offsetof(Vertex, color)},
            },
        };
    }
};

} // namespace shark
