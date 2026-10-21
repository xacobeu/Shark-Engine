#pragma once

#include <memory>
#include <vector>

#include "Node.hpp"
#include "node_types/Camera.hpp"
#include "node_types/Entity.hpp"
#include "node_types/InstancedEntity.hpp"
#include "node_types/Light.hpp"

class Scene {
public:
    template <typename T> T* add(std::shared_ptr<T> element) {
        static_assert(std::is_base_of_v<Node, T>, "T must derive from Node");
        if (!element) {
            return nullptr;
        }

        T* ptr = element.get();
        nodes.push_back(element);

        if constexpr (std::is_base_of_v<InstancedEntity, T>) {
            instancedEntities.push_back(element);
        } else if constexpr (std::is_base_of_v<Entity, T>) {
            entities.push_back(element);
        } else if constexpr (std::is_base_of_v<Light, T>) {
            lights.push_back(element);
        } else if constexpr (std::is_base_of_v<Camera, T>) {
            cameras.push_back(element);
            if (!activeCamera)
                activeCamera = ptr;
        }

        return ptr;
    }

    template <typename T, typename... Args> T* add(Args&&... args) {
        static_assert(std::is_base_of_v<Node, T>, "T must derive from Node");
        return add(std::make_shared<T>(std::forward<Args>(args)...));
    }

    void clear() {
        nodes.clear();
        entities.clear();
        instancedEntities.clear();
        lights.clear();
        cameras.clear();
        activeCamera = nullptr;
    }

    void               setActiveCamera(Camera* camera) { activeCamera = camera; }
    [[nodiscard]] auto getActiveCamera() const -> Camera* { return activeCamera; }

    [[nodiscard]] const std::vector<std::shared_ptr<Node>>&            getNodes() const { return nodes; }
    [[nodiscard]] const std::vector<std::shared_ptr<Entity>>&          getEntities() const { return entities; }
    [[nodiscard]] const std::vector<std::shared_ptr<InstancedEntity>>& getInstancedEntities() const {
        return instancedEntities;
    }
    [[nodiscard]] const std::vector<std::shared_ptr<Light>>& getLights() const { return lights; }

private:
    std::vector<std::shared_ptr<Node>>            nodes;
    std::vector<std::shared_ptr<Entity>>          entities;
    std::vector<std::shared_ptr<InstancedEntity>> instancedEntities;
    std::vector<std::shared_ptr<Light>>           lights;
    std::vector<std::shared_ptr<Camera>>          cameras;

    Camera* activeCamera = nullptr;
};
