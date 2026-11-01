module;

#include "imgui.h"

export module examples.molecular_dynamics;

import std;

import examples.ui;
import shark;

using namespace shark;

export {
class MolecularDynamicsExample : public Layer {
public:
    explicit MolecularDynamicsExample(std::size_t atomCount = 100);

    void onAttach() override;
    void onDetach() override;
    void onFixedUpdate(float fixedDt) override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

    void                             setAtomCount(std::size_t count);
    [[nodiscard]] inline std::size_t getAtomCount() const { return numAtoms; }

private:
    static constexpr float       SUB_DT     = 0.004f;
    static constexpr std::size_t NUM_COLORS = 18;

    struct ParticleData {
        std::vector<Vector3> positions;
        std::vector<Vector3> velocities;
        std::vector<Vector3> forces;
        std::vector<int>     species;
        std::vector<Vector4> colors;

        inline void resize(std::size_t count) {
            positions.resize(count, {.x = 0.0f, .y = 0.0f, .z = 0.0f});
            velocities.resize(count, {.x = 0.0f, .y = 0.0f, .z = 0.0f});
            forces.resize(count, {.x = 0.0f, .y = 0.0f, .z = 0.0f});
            species.resize(count, 0);
            colors.resize(count, {.x = 1.0f, .y = 1.0f, .z = 1.0f, .w = 1.0f});
        }

        [[nodiscard]] inline std::size_t size() const { return positions.size(); }
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
    void                  applyPairForce(std::size_t i, std::size_t j, const PotentialParams& params);
    void                  buildCellList(int nCells);
    void                  interactSameCell(int i, const PotentialParams& params);
    void interactNeighborCells(CellCoord cell, int nCells, int headAtom, const PotentialParams& params);
    void resolveCellInteractions(int nCells, const PotentialParams& params);
    void computeForcesDirect(const PotentialParams& params);
    void integrate(float dt);
    void renderImGui();

    // Scene & View
    Scene                 scene;
    OrbitCamera*          camera    = nullptr;
    InstancedEntity*      atomGroup = nullptr;
    std::shared_ptr<Mesh> sphereMesh;

    // Simulation State
    std::size_t                                    numAtoms    = 100;
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

} // export

MolecularDynamicsExample::MolecularDynamicsExample(std::size_t atomCount)
    : numAtoms(std::max<std::size_t>(1, atomCount)), uiAtomCount(static_cast<int>(numAtoms)) {}

void MolecularDynamicsExample::setAtomCount(std::size_t count) {
    numAtoms    = std::max<std::size_t>(1, count);
    uiAtomCount = static_cast<int>(numAtoms);
    if (sphereMesh != nullptr) {
        initEntities();
        resetSimulation();
    }
}

void MolecularDynamicsExample::initEntities() {
    Vector3 camTarget   = (camera != nullptr) ? camera->getTarget() : Vector3{.x = 0.0f, .y = 0.0f, .z = 0.0f};
    Vector3 camRot      = (camera != nullptr) ? camera->getRotation() : Vector3{.x = 0.0f, .y = 0.0f, .z = 0.0f};
    float   camDistance = (camera != nullptr) ? camera->getDistance() : 16.0f;

    scene.clear();
    camera    = scene.add<OrbitCamera>()->setTarget(camTarget)->setDistance(camDistance)->setRotation(camRot);
    atomGroup = scene.add<InstancedEntity>()->setMesh(sphereMesh);
    atomGroup->getInstancesMutable().resize(numAtoms);
    particles.resize(numAtoms);
}

void MolecularDynamicsExample::onAttach() {
    getApp()->setScene(&scene);

    sphereMesh = std::make_shared<Mesh>(
        MeshFactory::sphere(8, Vector3{.x = 1.0f, .y = 1.0f, .z = 1.0f}).scale(atomRadius)
    );

    initEntities();
    resetSimulation();
}

void MolecularDynamicsExample::onDetach() {
    if ((getApp() != nullptr) && getApp()->getScene() == &scene) {
        getApp()->setScene(nullptr);
    }
    atomGroup = nullptr;
    camera    = nullptr;
    sphereMesh.reset();
    scene.clear();
}

void MolecularDynamicsExample::resetSimulation() {
    if ((atomGroup == nullptr) || atomGroup->getInstances().size() != numAtoms) {
        initEntities();
    }
    auto& instanceData = atomGroup->getInstancesMutable();
    previousPositions.clear();

    std::random_device                    rd;
    std::mt19937                          rng(rd());
    std::uniform_real_distribution<float> jitter(-0.06f, 0.06f);
    std::normal_distribution<float>       velDist(0.0f, std::sqrt(targetTemperature));

    Vector3 avgVel{.x = 0.0f, .y = 0.0f, .z = 0.0f};

    // Calculate grid dimensions for numAtoms
    int nx = std::max(1, static_cast<int>(std::ceil(std::cbrt(static_cast<float>(numAtoms)))));
    int ny = std::max(1, static_cast<int>(std::ceil(std::sqrt(static_cast<float>(numAtoms) / static_cast<float>(nx)))));
    int nz = std::max(1, static_cast<int>(std::ceil(static_cast<float>(numAtoms) / static_cast<float>(nx * ny))));

    float maxDim  = static_cast<float>(std::max({nx, ny, nz}));
    float maxSpan = std::max(0.2f, (boxSize - atomRadius - 0.2f) * 2.0f);
    float spacing = std::min(1.4f, maxSpan / maxDim);

    float halfX = static_cast<float>(nx - 1) * 0.5f;
    float halfY = static_cast<float>(ny - 1) * 0.5f;
    float halfZ = static_cast<float>(nz - 1) * 0.5f;

    std::size_t idx = 0;
    for (int x = 0; x < nx && idx < numAtoms; ++x) {
        for (int y = 0; y < ny && idx < numAtoms; ++y) {
            for (int z = 0; z < nz && idx < numAtoms; ++z) {
                Vector3 pos = {
                    .x = ((static_cast<float>(x) - halfX) * spacing) + jitter(rng),
                    .y = ((static_cast<float>(y) - halfY) * spacing) + jitter(rng),
                    .z = ((static_cast<float>(z) - halfZ) * spacing) + jitter(rng),
                };

                Vector3 vel = {
                    .x = velDist(rng),
                    .y = velDist(rng),
                    .z = velDist(rng),
                };
                avgVel += vel;

                particles.positions.at(idx)  = pos;
                particles.velocities.at(idx) = vel;
                particles.forces.at(idx)     = {.x = 0.0f, .y = 0.0f, .z = 0.0f};
                particles.species.at(idx)    = (idx % 2 == 0) ? 0 : 1;

                particles.colors.at(idx) = (particles.species.at(idx) == 0)
                                               ? Vector4{.x = 0.2f, .y = 0.75f, .z = 0.95f, .w = 1.0f}  // Cyan
                                               : Vector4{.x = 1.0f, .y = 0.55f, .z = 0.15f, .w = 1.0f}; // Amber

                instanceData.at(idx).color       = particles.colors.at(idx);
                instanceData.at(idx).modelMatrix = Matrix4::translate(pos);

                ++idx;
            }
        }
    }

    if (numAtoms > 1) {
        avgVel /= static_cast<float>(numAtoms);
        for (std::size_t i = 0; i < numAtoms; ++i) {
            particles.velocities.at(i) -= avgVel;
        }
    }

    computeForces();
}

Vector3 MolecularDynamicsExample::pairForce(const Vector3& r, float r2, const PotentialParams& params) const {
    const float invR2   = 1.0f / std::max(r2, params.minR2);
    const float s6invR6 = params.s6 * invR2 * invR2 * invR2;
    return r * (24.0f * epsilon * invR2 * s6invR6 * ((2.0f * s6invR6) - 1.0f));
}

void MolecularDynamicsExample::applyPairForce(std::size_t i, std::size_t j, const PotentialParams& params) {
    Vector3 r  = particles.positions.at(i) - particles.positions.at(j);
    float   r2 = r.lengthSq();
    if (r2 > params.rCut2) {
        return;
    }

    const Vector3 f = pairForce(r, r2, params);
    particles.forces.at(i) += f;
    particles.forces.at(j) -= f;
}

void MolecularDynamicsExample::buildCellList(int nCells) {
    const int totalCells = nCells * nCells * nCells;
    cellHead.assign(static_cast<std::size_t>(totalCells), -1);
    nextAtom.resize(numAtoms, -1);
    for (auto& bucket : colorCells) {
        bucket.clear();
    }

    const float invCellSize = static_cast<float>(nCells) / (2.0f * boxSize);

    for (std::size_t i = 0; i < numAtoms; ++i) {
        int cx = std::clamp(static_cast<int>((particles.positions.at(i).x + boxSize) * invCellSize), 0, nCells - 1);
        int cy = std::clamp(static_cast<int>((particles.positions.at(i).y + boxSize) * invCellSize), 0, nCells - 1);
        int cz = std::clamp(static_cast<int>((particles.positions.at(i).z + boxSize) * invCellSize), 0, nCells - 1);

        int cIdx       = cx + (nCells * (cy + (nCells * cz)));
        int prevHead   = cellHead.at(static_cast<std::size_t>(cIdx));
        nextAtom.at(i) = prevHead;
        if (prevHead == -1) {
            // 3x3x2 coloring: same-colored cells never share a forward neighbor, so a color can run in parallel
            const int color = (cx % 3) + (3 * (cy % 3)) + (9 * (cz & 1));
            colorCells[static_cast<std::size_t>(color)].push_back({.x = cx, .y = cy, .z = cz});
        }
        cellHead.at(static_cast<std::size_t>(cIdx)) = static_cast<int>(i);
    }
}

void MolecularDynamicsExample::interactSameCell(int i, const PotentialParams& params) {
    for (int a = i; a != -1; a = nextAtom.at(static_cast<std::size_t>(a))) {
        for (int b = nextAtom.at(static_cast<std::size_t>(a)); b != -1; b = nextAtom.at(static_cast<std::size_t>(b))) {
            applyPairForce(static_cast<std::size_t>(a), static_cast<std::size_t>(b), params);
        }
    }
}

void MolecularDynamicsExample::interactNeighborCells(
    CellCoord cell, int nCells, int headAtom, const PotentialParams& params
) {
    static constexpr std::array<std::array<int, 3>, 13> NEIGHBOR_OFFSETS = {
        {
            {1, 0, 0},
            {-1, 1, 0},
            {0, 1, 0},
            {1, 1, 0},
            {-1, -1, 1},
            {0, -1, 1},
            {1, -1, 1},
            {-1, 0, 1},
            {0, 0, 1},
            {1, 0, 1},
            {-1, 1, 1},
            {0, 1, 1},
            {1, 1, 1},
        },
    };

    for (const auto& off : NEIGHBOR_OFFSETS) {
        int nx = cell.x + off.at(0);
        int ny = cell.y + off.at(1);
        int nz = cell.z + off.at(2);

        if (nx < 0 || nx >= nCells || ny < 0 || ny >= nCells || nz < 0 || nz >= nCells) {
            continue;
        }

        int nIdx = nx + (nCells * (ny + (nCells * nz)));
        int j    = cellHead.at(static_cast<std::size_t>(nIdx));
        if (j == -1) {
            continue;
        }

        for (int a = headAtom; a != -1; a = nextAtom.at(static_cast<std::size_t>(a))) {
            for (int b = j; b != -1; b = nextAtom.at(static_cast<std::size_t>(b))) {
                applyPairForce(static_cast<std::size_t>(a), static_cast<std::size_t>(b), params);
            }
        }
    }
}

void MolecularDynamicsExample::resolveCellInteractions(int nCells, const PotentialParams& params) {
    for (const auto& bucket : colorCells) {
        shark::exec::for_each(shark::exec::par, bucket, [this, nCells, &params](const CellCoord& cell) {
            int cIdx = cell.x + (nCells * (cell.y + (nCells * cell.z)));
            int i    = cellHead.at(static_cast<std::size_t>(cIdx));
            if (i != -1) {
                interactSameCell(i, params);
                interactNeighborCells(cell, nCells, i, params);
            }
        });
    }
}

void MolecularDynamicsExample::computeForcesDirect(const PotentialParams& params) {
    shark::exec::for_each_index(shark::exec::par, numAtoms, [this, &params](std::size_t i) {
        Vector3       accForce{.x = 0.0f, .y = 0.0f, .z = 0.0f};
        const Vector3 posI = particles.positions[i];

        for (std::size_t j = 0; j < numAtoms; ++j) {
            if (i == j) {
                continue;
            }
            Vector3 r  = posI - particles.positions[j];
            float   r2 = r.lengthSq();
            if (r2 > params.rCut2) {
                continue;
            }
            accForce += pairForce(r, r2, params);
        }

        particles.forces[i] = accForce;
    });
}

void MolecularDynamicsExample::computeForces() {
    const float rCut = 2.5f * sigma;
    const float s2   = sigma * sigma;

    const PotentialParams params = {
        .rCut2 = rCut * rCut,
        .minR2 = 0.04f,
        .s6    = s2 * s2 * s2,
    };

    int nCells = static_cast<int>((2.0f * boxSize) / rCut);
    if (numAtoms >= 64 && nCells >= 3) {
        shark::exec::fill(shark::exec::par_unseq, particles.forces, Vector3{.x = 0.0f, .y = 0.0f, .z = 0.0f});
        nCells = std::min(nCells, 12);
        buildCellList(nCells);
        resolveCellInteractions(nCells, params);
    } else {
        computeForcesDirect(params);
    }
}

void MolecularDynamicsExample::integrate(float dt) {
    const float halfDt     = 0.5f * dt;
    const float limit      = boxSize - atomRadius;
    const float dampFactor = (damping > 0.0f) ? std::max(0.0f, 1.0f - (damping * dt)) : 1.0f;

    // Velocity Verlet: half kick + drift
    shark::exec::for_each_index(shark::exec::par_unseq, numAtoms, [this, halfDt, dt, limit, dampFactor](std::size_t i) {
        particles.velocities[i] += particles.forces[i] * halfDt;
        particles.velocities[i] *= dampFactor;
        particles.positions[i] += particles.velocities[i] * dt;

        // Reflective boundary checks
        if (particles.positions[i].x > limit) {
            particles.positions[i].x  = limit;
            particles.velocities[i].x = -std::abs(particles.velocities[i].x);
        } else if (particles.positions[i].x < -limit) {
            particles.positions[i].x  = -limit;
            particles.velocities[i].x = std::abs(particles.velocities[i].x);
        }

        if (particles.positions[i].y > limit) {
            particles.positions[i].y  = limit;
            particles.velocities[i].y = -std::abs(particles.velocities[i].y);
        } else if (particles.positions[i].y < -limit) {
            particles.positions[i].y  = -limit;
            particles.velocities[i].y = std::abs(particles.velocities[i].y);
        }

        if (particles.positions[i].z > limit) {
            particles.positions[i].z  = limit;
            particles.velocities[i].z = -std::abs(particles.velocities[i].z);
        } else if (particles.positions[i].z < -limit) {
            particles.positions[i].z  = -limit;
            particles.velocities[i].z = std::abs(particles.velocities[i].z);
        }
    });

    computeForces();

    // Second half kick with the new forces
    shark::exec::for_each_index(shark::exec::par_unseq, numAtoms, [this, halfDt](std::size_t i) {
        particles.velocities[i] += particles.forces[i] * halfDt;
    });
}

void MolecularDynamicsExample::onFixedUpdate(float /*fixedDt*/) {
    if (isPaused) {
        return;
    }

    previousPositions = particles.positions;

    const int subSteps = (numAtoms > 300) ? 2 : 4;
    for (auto _ : std::views::iota(0, subSteps)) {
        integrate(SUB_DT);
    }
}

void MolecularDynamicsExample::onUpdate(float deltaTime) {
    if (isPaused) {
        return;
    }

    elapsedTime += deltaTime;

    const float totalKinetic = shark::exec::transform_reduce_index(
        shark::exec::par_unseq, numAtoms, 0.0f, std::plus<>{},
        [this](std::size_t i) { return 0.5f * particles.velocities[i].lengthSq(); }
    );

    currentKineticEnergy = totalKinetic;
    currentTemperature   = (numAtoms > 0) ? (2.0f * totalKinetic) / (3.0f * static_cast<float>(numAtoms)) : 0.0f;

    const bool  canInterpolate = previousPositions.size() == numAtoms;
    const float alpha          = getApp()->getFixedAlpha();
    auto&       instanceData   = atomGroup->getInstancesMutable();
    shark::exec::for_each_index(
        shark::exec::par_unseq, numAtoms, [this, &instanceData, canInterpolate, alpha](std::size_t i) {
            const Vector3 pos           = canInterpolate ? lerp(previousPositions[i], particles.positions[i], alpha)
                                                         : particles.positions[i];
            instanceData[i].modelMatrix = Matrix4::translate(pos);
        }
    );
}

void MolecularDynamicsExample::renderImGui() {
    if (!examples::beginControls(ImVec2(340.0f, 360.0f))) {
        return;
    }

    if (ImGui::SliderInt("Particle Count", &uiAtomCount, 1, 2000)) {
        uiAtomCount = std::clamp(uiAtomCount, 1, 2000);
    }

    if (ImGui::Button(isPaused ? " Resume " : " Pause ")) {
        isPaused = !isPaused;
    }

    ImGui::SameLine();
    if (ImGui::Button("Rescale Velocities") && currentTemperature > 0.001f) {
        float factor = std::sqrt(targetTemperature / currentTemperature);
        shark::exec::for_each(shark::exec::par_unseq, particles.velocities, [factor](Vector3& v) { v *= factor; });
    }

    ImGui::Separator();
    if (ImGui::IsItemDeactivatedAfterEdit() && uiAtomCount > 0 && std::cmp_not_equal(uiAtomCount, numAtoms)) {
        numAtoms = static_cast<std::size_t>(uiAtomCount);
        initEntities();
        resetSimulation();
    }

    ImGui::SliderFloat("Target Temp", &targetTemperature, 0.1f, 5.0f);

    ImGui::Separator();
    ImGui::SliderFloat("Epsilon (Well Depth)", &epsilon, 0.2f, 8.0f);
    ImGui::SliderFloat("Sigma (Atom Size)", &sigma, 0.4f, 1.4f);
    ImGui::SliderFloat("Damping", &damping, 0.0f, 0.2f);
    ImGui::SliderFloat("Box Size", &boxSize, 2.5f, 12.0f);

    ImGui::End();
}

void MolecularDynamicsExample::onRender(Renderer& renderer) {
    renderer.render(scene, getApp()->getWidth(), getApp()->getHeight());
    renderImGui();
}
