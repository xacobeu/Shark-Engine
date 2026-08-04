export module SharkEngine:Node;
import :Math;

export struct Transform {
    Vector3 position{0.0f, 0.0f, 0.0f};
    Vector3 rotationDeg{0.0f, 0.0f, 0.0f};
    Vector3 scale{1.0f, 1.0f, 1.0f};
};

export class Node {
public:
    virtual ~Node() = default;

    // Transform setters.
    Node* setPosition(Vector3 pos) { transform.position = pos; return this; }
    Node* setRotation(Vector3 rot) { transform.rotationDeg = rot; return this; }
    Node* setScale(Vector3 scl) { transform.scale = scl; return this; }
    Node* setTransform(const Transform& trns) { transform = trns; return this; }

    // Transform getters.
    [[nodiscard]] const Vector3& getPosition() const { return transform.position; }
    [[nodiscard]] const Vector3& getRotation() const { return transform.rotationDeg; }
    [[nodiscard]] const Vector3& getScale() const { return transform.scale; }
    [[nodiscard]] const Transform& getTransform() const { return transform; }

protected:
    Transform transform;
};
