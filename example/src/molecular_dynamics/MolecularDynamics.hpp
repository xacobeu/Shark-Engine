#pragma once

#include <array>
#include <memory>
#include <vector>

#include "Layer.hpp"
#include "renderer/geometry/Mesh.hpp"
#include "renderer/scene/Scene.hpp"
#include "renderer/scene/node_types/Entity.hpp"
#include "renderer/scene/node_types/OrbitCamera.hpp"
#include "util/Vector.hpp"

class MolecularDynamics : public Layer {
public:
    explicit MolecularDynamics(size_t atomCount = 100);

    void onAttach() override;
    void onDetach() override;
    void onFixedUpdate(float fixedDt) override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

    void                 setAtomCount(size_t count);
    [[nodiscard]] size_t getAtomCount() const { return numAtoms; }

private:
    static constexpr float       SUB_DT     = 0.002f;
    static constexpr std::size_t NUM_COLORS = 18;

    struct ParticleData {
        std::vector<Vector3> positions;
        std::vector<Vector3> velocities;
        std::vector<Vector3> forces;
        std::vector<int>     species;
        std::vector<Vector4> colors;

        void resize(size_t count) {
            positions.resize(count, {.x = 0.0f, .y = 0.0f, .z = 0.0f});
            velocities.resize(count, {.x = 0.0f, .y = 0.0f, .z = 0.0f});
            forces.resize(count, {.x = 0.0f, .y = 0.0f, .z = 0.0f});
            species.resize(count, 0);
            colors.resize(count, {.x = 1.0f, .y = 1.0f, .z = 1.0f, .w = 1.0f});
        }

        [[nodiscard]] size_t size() const { return positions.size(); }
    };

    struct PotentialParams {
        float rCut2;
        float minR2;
        float s6;
    };

    struct CellCoord {
        int x;
        int y;
        int z;
    };

    void                  initEntities();
    void                  resetSimulation();
    void                  computeForces();
    [[nodiscard]] Vector3 pairForce(const Vector3& r, float r2, const PotentialParams& params) const;
    void                  applyPairForce(size_t i, size_t j, const PotentialParams& params);
    void                  buildCellList(int nCells);
    void                  interactSameCell(int i, const PotentialParams& params);
    void interactNeighborCells(CellCoord cell, int nCells, int headAtom, const PotentialParams& params);
    void resolveCellInteractions(int nCells, const PotentialParams& params);
    void computeForcesDirect(const PotentialParams& params);
    void integrate(float dt);
    void renderImGui();

    // Scene & View
    Scene                 scene;
    OrbitCamera*          camera = nullptr;
    std::shared_ptr<Mesh> sphereMesh;
    std::vector<Entity*>  atomEntities;

    // Simulation State
    size_t                                         numAtoms    = 100;
    int                                            uiAtomCount = 100;
    ParticleData                                   particles;
    std::vector<Vector3>                           previousPositions;
    std::vector<int>                               cellHead;
    std::vector<int>                               nextAtom;
    std::array<std::vector<CellCoord>, NUM_COLORS> colorCells;

    // Simulation Parameters
    float epsilon           = 1.0f;
    float sigma             = 1.0f;
    float atomRadius        = 0.25f;
    float boxSize           = 4.5f;
    float targetTemperature = 1.0f;
    float damping           = 0.0f;

    // Runtime Metrics & Timing
    float currentTemperature   = 0.0f;
    float currentKineticEnergy = 0.0f;
    float elapsedTime          = 0.0f;
    bool  isPaused             = false;
};
