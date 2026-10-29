export module shark:renderer.scene.Node;

import std;

import :util.Vector;

export namespace shark {
struct Transform {
    Vector3 position{0.0f, 0.0f, 0.0f};
    Vector3 rotationDeg{0.0f, 0.0f, 0.0f};
    Vector3 scale{1.0f, 1.0f, 1.0f};
};

class Node {
public:
    virtual ~Node() = default;

    Node(const Node&)            = delete ("nodes are shared via std::shared_ptr");
    Node& operator=(const Node&) = delete ("nodes are shared via std::shared_ptr");
    Node(Node&&)                 = delete ("Scene caches raw node pointers");
    Node& operator=(Node&&)      = delete ("Scene caches raw node pointers");

    // Transform setters.
    auto* setPosition(this auto& self, Vector3 pos) {
        self.transform.position = pos;
        return &self;
    }
    auto* setRotation(this auto& self, Vector3 rot) {
        self.transform.rotationDeg = rot;
        return &self;
    }
    auto* setScale(this auto& self, Vector3 scl) {
        self.transform.scale = scl;
        return &self;
    }
    auto* setTransform(this auto& self, const Transform& trns) {
        self.transform = trns;
        return &self;
    }

    // Transform getters.
    [[nodiscard]] inline const Vector3&   getPosition() const { return transform.position; }
    [[nodiscard]] inline const Vector3&   getRotation() const { return transform.rotationDeg; }
    [[nodiscard]] inline const Vector3&   getScale() const { return transform.scale; }
    [[nodiscard]] inline const Transform& getTransform() const { return transform; }

protected:
    Node() = default;

private:
    Transform transform;
};

} // namespace shark
