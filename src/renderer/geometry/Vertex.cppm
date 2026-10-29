export module shark:renderer.geometry.Vertex;

import :util.Vector;

export namespace shark {
struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
    Vector3 color;
};

} // namespace shark
