module;
#include <memory>
#include <numbers>
#include <cmath>

export module SharkEngine:SceneNodes;
import :Math;
import :Node;
import :Mesh;
import :RenderDeviceAbstractions;

export class Application;

export class Camera : public Node {
public:
    Camera* setPosition(Vector3 pos) { Node::setPosition(pos); return this; }
    Camera* setRotation(Vector3 rot) { Node::setRotation(rot); return this; }
    Camera* setScale(Vector3 scl) { Node::setScale(scl); return this; }
    Camera* setTransform(const Transform& trns) { Node::setTransform(trns); return this; }

    Camera* setFov(float newFov) { this->fov = newFov; return this; }
    Camera* setNear(float newNear) { this->near = newNear; return this; }
    Camera* setFar(float newFar) { this->far = newFar; return this; }

    [[nodiscard]] float getFov() const { return fov; }
    [[nodiscard]] float getNear() const { return near; }
    [[nodiscard]] float getFar() const { return far; }

    [[nodiscard]] virtual Matrix4 getProjectionMatrix(float aspect) const {
        return Matrix4::perspective(fov, aspect, near, far);
    }
    
    [[nodiscard]] virtual Matrix4 getViewMatrix() const {
        return Matrix4::lookAt(transform.position, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    }

    virtual void update(Application* app, float deltaTime);
    virtual ~Camera() override;

private:
    float fov = 45.0f;
    float near = 0.1f;
    float far = 1000.0f;
};

export class Entity : public Node {
public:
    Entity* setPosition(Vector3 pos) { Node::setPosition(pos); return this; }
    Entity* setRotation(Vector3 rot) { Node::setRotation(rot); return this; }
    Entity* setScale(Vector3 scl) { Node::setScale(scl); return this; }
    Entity* setTransform(const Transform& trns) { Node::setTransform(trns); return this; }

    Entity* setMesh(std::shared_ptr<Mesh> newMesh) { this->mesh = std::move(newMesh); return this; }
    Entity* setTexture(std::shared_ptr<ITexture> newTex) { this->texture = std::move(newTex); return this; }
    Entity* setColor(Vector4 newColor) { this->color = newColor; return this; }

    [[nodiscard]] const std::shared_ptr<Mesh>& getMesh() const { return mesh; }
    [[nodiscard]] const std::shared_ptr<ITexture>& getTexture() const { return texture; }
    [[nodiscard]] const Vector4& getColor() const { return color; }

    virtual ~Entity() override;

private:
    std::shared_ptr<Mesh> mesh = nullptr;
    std::shared_ptr<ITexture> texture = nullptr;
    Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
};

export enum class LightType { Ambient, Directional, Point };

export class Light : public Node {
public:
    Light* setPosition(Vector3 pos) { Node::setPosition(pos); return this; }
    Light* setRotation(Vector3 rot) { Node::setRotation(rot); return this; }
    Light* setScale(Vector3 scl) { Node::setScale(scl); return this; }
    Light* setTransform(const Transform& trns) { Node::setTransform(trns); return this; }

    Light* setColor(Vector4 newColor) { this->color = newColor; return this; }
    Light* setType(LightType newType) { this->type = newType; return this; }
    Light* setIntensity(float newIntensity) { this->intensity = newIntensity; return this; }

    [[nodiscard]] Vector4 getColor() const { return color; }
    [[nodiscard]] LightType getType() const { return type; }
    [[nodiscard]] float getIntensity() const { return intensity; }

    virtual ~Light() override;

private:
    LightType type;
    Vector4 color{1.0f, 1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
};

export class FlyingCamera : public Camera {
public:
    FlyingCamera* setPosition(Vector3 pos) { Node::setPosition(pos); return this; }
    FlyingCamera* setRotation(Vector3 rot) { Node::setRotation(rot); return this; }
    FlyingCamera* setScale(Vector3 scl) { Node::setScale(scl); return this; }
    FlyingCamera* setTransform(const Transform& trns) { Node::setTransform(trns); return this; }

    FlyingCamera* setFov(float newFov) { Camera::setFov(newFov); return this; }
    FlyingCamera* setNear(float newNear) { Camera::setNear(newNear); return this; }
    FlyingCamera* setFar(float newFar) { Camera::setFar(newFar); return this; }

    [[nodiscard]] Matrix4 getViewMatrix() const override {
        float pitchRad = transform.rotationDeg.x * std::numbers::pi_v<float> / 180.0f;
        float yawRad = transform.rotationDeg.y * std::numbers::pi_v<float> / 180.0f;

        Vector3 direction = {
            std::cos(pitchRad) * std::cos(yawRad),
            std::sin(pitchRad),
            std::cos(pitchRad) * std::sin(yawRad)
        };

        float len = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
        if (len > 0.0f) {
            direction.x /= len;
            direction.y /= len;
            direction.z /= len;
        }

        Vector3 target = transform.position + direction;
        return Matrix4::lookAt(transform.position, target, {0.0f, 1.0f, 0.0f});
    }

    void update(Application* app, float deltaTime) override;
    virtual ~FlyingCamera() override;

private:
    bool isDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
};

export class OrbitCamera : public Camera {
public:
    OrbitCamera* setPosition(Vector3 pos) { Node::setPosition(pos); return this; }
    OrbitCamera* setRotation(Vector3 rot) { Node::setRotation(rot); return this; }
    OrbitCamera* setScale(Vector3 scl) { Node::setScale(scl); return this; }
    OrbitCamera* setTransform(const Transform& trns) { Node::setTransform(trns); return this; }

    OrbitCamera* setFov(float newFov) { Camera::setFov(newFov); return this; }
    OrbitCamera* setNear(float newNear) { Camera::setNear(newNear); return this; }
    OrbitCamera* setFar(float newFar) { Camera::setFar(newFar); return this; }

    OrbitCamera* setTarget(Vector3 newTarget) { this->target = newTarget; return this; }
    OrbitCamera* setDistance(float newDistance) { this->distance = newDistance; return this; }

    [[nodiscard]] Vector3 getTarget() const { return target; }
    [[nodiscard]] float getDistance() const { return distance; }

    [[nodiscard]] Matrix4 getViewMatrix() const override {
        float pitchRad = transform.rotationDeg.x * std::numbers::pi_v<float> / 180.0f;
        float yawRad = transform.rotationDeg.y * std::numbers::pi_v<float> / 180.0f;

        Vector3 pos = {
            target.x + distance * std::cos(pitchRad) * std::cos(yawRad),
            target.y + distance * std::sin(pitchRad),
            target.z + distance * std::cos(pitchRad) * std::sin(yawRad)
        };
        return Matrix4::lookAt(pos, target, {0.0f, 1.0f, 0.0f});
    }

    void update(Application* app, float deltaTime) override;
    virtual ~OrbitCamera() override;

private:
    Vector3 target{0.0f, 0.0f, 0.0f};
    float distance = 10.0f;

    bool isDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    bool isPanning = false;
    double lastPanX = 0.0;
    double lastPanY = 0.0;
};
