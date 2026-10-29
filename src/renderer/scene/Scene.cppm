export module shark:renderer.scene.Scene;

import std;

import :renderer.scene.Node;
import :renderer.scene.node_types.Camera;
import :renderer.scene.node_types.Entity;
import :renderer.scene.node_types.InstancedEntity;
import :renderer.scene.node_types.Light;

export namespace shark {
class Scene {
public:
    static constexpr std::size_t MAX_LIGHTS = 8;

    template <typename T> T* add(std::shared_ptr<T> element) {
        static_assert(std::is_base_of_v<Node, T>, "T must derive from Node");
        if (!element) {
            return nullptr;
        }

        if constexpr (std::is_base_of_v<Light, T>) {
            if (lights.size() >= MAX_LIGHTS) {
                throw std::runtime_error("Scene exceeds Scene::MAX_LIGHTS lights.");
            }
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

    inline void clear() {
        nodes.clear();
        entities.clear();
        instancedEntities.clear();
        lights.clear();
        cameras.clear();
        activeCamera = nullptr;
    }

    inline void               setActiveCamera(Camera* camera) { activeCamera = camera; }
    [[nodiscard]] inline auto getActiveCamera() const -> Camera* { return activeCamera; }

    [[nodiscard]] inline const std::vector<std::shared_ptr<Node>>&            getNodes() const { return nodes; }
    [[nodiscard]] inline const std::vector<std::shared_ptr<Entity>>&          getEntities() const { return entities; }
    [[nodiscard]] inline const std::vector<std::shared_ptr<InstancedEntity>>& getInstancedEntities() const {
        return instancedEntities;
    }
    [[nodiscard]] inline const std::vector<std::shared_ptr<Light>>& getLights() const { return lights; }
    [[nodiscard]] inline std::size_t getRemainingLightSlots() const { return MAX_LIGHTS - lights.size(); }

private:
    std::vector<std::shared_ptr<Node>>            nodes;
    std::vector<std::shared_ptr<Entity>>          entities;
    std::vector<std::shared_ptr<InstancedEntity>> instancedEntities;
    std::vector<std::shared_ptr<Light>>           lights;
    std::vector<std::shared_ptr<Camera>>          cameras;

    Camera* activeCamera = nullptr;
};

} // namespace shark
