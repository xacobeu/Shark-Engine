#pragma once

#include "Node.hpp"
#include "node_types/Camera.hpp"
#include "node_types/OrbitCamera.hpp"
#include "node_types/FlyingCamera.hpp"
#include "node_types/Entity.hpp"
#include "node_types/Light.hpp"

#include <memory>
#include <vector>

class Scene {
public:
    template<typename T, typename... Args>
    T* add(Args&&... args) {
        static_assert(std::is_base_of_v<Node, T>, "T must derive from Node");

        auto element = std::make_shared<T>(std::forward<Args>(args)...);
        T* ptr = element.get();

        nodes.push_back(element);

        if constexpr (std::is_base_of_v<Entity, T>) {
            entities.push_back(element);
        } else if constexpr (std::is_base_of_v<Light, T>) {
            lights.push_back(element);
        } else if constexpr (std::is_base_of_v<Camera, T>) {
            cameras.push_back(element);
            if (!activeCamera) activeCamera = ptr; 
        }

        return ptr;
    }

    void setActiveCamera(Camera* camera) { activeCamera = camera; }
    [[nodiscard]] Camera* getActiveCamera() const { return activeCamera; }

    [[nodiscard]] const std::vector<std::shared_ptr<Node>>& getNodes() const { return nodes; }
    [[nodiscard]] const std::vector<std::shared_ptr<Entity>>& getEntities() const { return entities; }
    [[nodiscard]] const std::vector<std::shared_ptr<Light>>& getLights() const { return lights; }

private:
    std::vector<std::shared_ptr<Node>> nodes;
    std::vector<std::shared_ptr<Entity>> entities;
    std::vector<std::shared_ptr<Light>> lights;
    std::vector<std::shared_ptr<Camera>> cameras;

    Camera* activeCamera = nullptr;
};
