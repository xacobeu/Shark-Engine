#include "MolecularDynamics.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <print>
#include <random>
#include <utility>

#include "imgui.h"

#include "Application.hpp"
#include "renderer/geometry/MeshFactory.hpp"

MolecularDynamics::MolecularDynamics(size_t atomCount)
    : numAtoms(std::max<size_t>(1, atomCount)), uiAtomCount(static_cast<int>(numAtoms)) {}

void MolecularDynamics::setAtomCount(size_t count) {
    numAtoms    = std::max<size_t>(1, count);
    uiAtomCount = static_cast<int>(numAtoms);
    if (sphereMesh != nullptr) {
        initEntities();
        resetSimulation();
    }
}

void MolecularDynamics::initEntities() {
    Vector3 camTarget   = (camera != nullptr) ? camera->getTarget() : Vector3{.x = 0.0f, .y = 0.0f, .z = 0.0f};
    Vector3 camRot      = (camera != nullptr) ? camera->getRotation() : Vector3{.x = 0.0f, .y = 0.0f, .z = 0.0f};
    float   camDistance = (camera != nullptr) ? camera->getDistance() : 16.0f;

    scene.clear();
    camera = scene.add<OrbitCamera>()->setTarget(camTarget)->setDistance(camDistance)->setRotation(camRot);

    atomEntities.clear();
    atomEntities.reserve(numAtoms);
    particles.resize(numAtoms);

    for (size_t i = 0; i < numAtoms; ++i) {
        auto* entity =
            scene.add<Entity>()->setMesh(sphereMesh)->setScale({.x = atomRadius, .y = atomRadius, .z = atomRadius});
        atomEntities.push_back(entity);
    }
}

void MolecularDynamics::onAttach() {
    std::println("[DEBUG] Molecular Dynamics layer attached with {} atoms", numAtoms);

    getApp()->setScene(&scene);

    sphereMesh = std::make_shared<Mesh>(MeshFactory::sphere(8, Vector3{.x = 1.0f, .y = 1.0f, .z = 1.0f}));

    initEntities();
    resetSimulation();
}

void MolecularDynamics::onDetach() {
    if ((getApp() != nullptr) && getApp()->getScene() == &scene) {
        getApp()->setScene(nullptr);
    }
    atomEntities.clear();
    camera = nullptr;
    sphereMesh.reset();
    scene.clear();
}

void MolecularDynamics::resetSimulation() {
    if (atomEntities.size() != numAtoms) {
        initEntities();
    }

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

    size_t idx = 0;
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
                avgVel = avgVel + vel;

                particles.positions.at(idx)  = pos;
                particles.velocities.at(idx) = vel;
                particles.forces.at(idx)     = {.x = 0.0f, .y = 0.0f, .z = 0.0f};
                particles.species.at(idx)    = (idx % 2 == 0) ? 0 : 1;

                particles.colors.at(idx) = (particles.species.at(idx) == 0)
                                               ? Vector4{.x = 0.2f, .y = 0.75f, .z = 0.95f, .w = 1.0f}  // Cyan
                                               : Vector4{.x = 1.0f, .y = 0.55f, .z = 0.15f, .w = 1.0f}; // Amber

                atomEntities.at(idx)->setColor(particles.colors.at(idx));
                atomEntities.at(idx)->setPosition(pos);

                ++idx;
            }
        }
    }

    if (numAtoms > 1) {
        avgVel = avgVel / static_cast<float>(numAtoms);
        for (size_t i = 0; i < numAtoms; ++i) {
            particles.velocities.at(i) = particles.velocities.at(i) - avgVel;
        }
    }

    computeForces();
}

void MolecularDynamics::applyPairForce(size_t i, size_t j, const PotentialParams& params) {
    Vector3 r  = particles.positions.at(i) - particles.positions.at(j);
    float   r2 = r.dot(r);
    if (r2 > params.rCut2) {
        return;
    }

    float safeR2  = std::max(r2, params.minR2);
    float invR2   = 1.0f / safeR2;
    float invR6   = invR2 * invR2 * invR2;
    float s6invR6 = params.s6 * invR6;

    float fScalar = 24.0f * epsilon * invR2 * s6invR6 * ((2.0f * s6invR6) - 1.0f);

    Vector3 f              = r * fScalar;
    particles.forces.at(i) = particles.forces.at(i) + f;
    particles.forces.at(j) = particles.forces.at(j) - f;
}

void MolecularDynamics::buildCellList(int nCells) {
    const int totalCells = nCells * nCells * nCells;
    cellHead.assign(static_cast<size_t>(totalCells), -1);
    nextAtom.resize(numAtoms, -1);

    const float invCellSize = static_cast<float>(nCells) / (2.0f * boxSize);

    for (size_t i = 0; i < numAtoms; ++i) {
        int cx = std::clamp(static_cast<int>((particles.positions.at(i).x + boxSize) * invCellSize), 0, nCells - 1);
        int cy = std::clamp(static_cast<int>((particles.positions.at(i).y + boxSize) * invCellSize), 0, nCells - 1);
        int cz = std::clamp(static_cast<int>((particles.positions.at(i).z + boxSize) * invCellSize), 0, nCells - 1);

        int cIdx                               = cx + (nCells * (cy + (nCells * cz)));
        nextAtom.at(i)                         = cellHead.at(static_cast<size_t>(cIdx));
        cellHead.at(static_cast<size_t>(cIdx)) = static_cast<int>(i);
    }
}

void MolecularDynamics::interactSameCell(int i, const PotentialParams& params) {
    for (int a = i; a != -1; a = nextAtom.at(static_cast<size_t>(a))) {
        for (int b = nextAtom.at(static_cast<size_t>(a)); b != -1; b = nextAtom.at(static_cast<size_t>(b))) {
            applyPairForce(static_cast<size_t>(a), static_cast<size_t>(b), params);
        }
    }
}

void MolecularDynamics::interactNeighborCells(CellCoord cell, int nCells, int headAtom, const PotentialParams& params) {
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
        int j    = cellHead.at(static_cast<size_t>(nIdx));
        if (j == -1) {
            continue;
        }

        for (int a = headAtom; a != -1; a = nextAtom.at(static_cast<size_t>(a))) {
            for (int b = j; b != -1; b = nextAtom.at(static_cast<size_t>(b))) {
                applyPairForce(static_cast<size_t>(a), static_cast<size_t>(b), params);
            }
        }
    }
}

void MolecularDynamics::resolveCellInteractions(int nCells, const PotentialParams& params) {
    for (int cz = 0; cz < nCells; ++cz) {
        for (int cy = 0; cy < nCells; ++cy) {
            for (int cx = 0; cx < nCells; ++cx) {
                int cIdx = cx + (nCells * (cy + (nCells * cz)));
                int i    = cellHead.at(static_cast<size_t>(cIdx));
                if (i != -1) {
                    interactSameCell(i, params);
                    interactNeighborCells({.x = cx, .y = cy, .z = cz}, nCells, i, params);
                }
            }
        }
    }
}

void MolecularDynamics::computeForcesDirect(const PotentialParams& params) {
    for (size_t i = 0; i < numAtoms; ++i) {
        for (size_t j = i + 1; j < numAtoms; ++j) {
            applyPairForce(i, j, params);
        }
    }
}

void MolecularDynamics::computeForces() {
    for (size_t i = 0; i < numAtoms; ++i) {
        particles.forces.at(i) = {.x = 0.0f, .y = 0.0f, .z = 0.0f};
    }

    const float rCut = 2.5f * sigma;
    const float s2   = sigma * sigma;

    const PotentialParams params = {
        .rCut2 = rCut * rCut,
        .minR2 = 0.04f,
        .s6    = s2 * s2 * s2,
    };

    int nCells = static_cast<int>((2.0f * boxSize) / rCut);
    if (numAtoms >= 64 && nCells >= 3) {
        nCells = std::min(nCells, 12);
        buildCellList(nCells);
        resolveCellInteractions(nCells, params);
    } else {
        computeForcesDirect(params);
    }
}

void MolecularDynamics::integrate(float dt) {
    const float halfDt = 0.5f * dt;
    const float limit  = boxSize - atomRadius;

    // Step 1: Half-step velocity update & Full-step position drift
    for (size_t i = 0; i < numAtoms; ++i) {
        particles.velocities.at(i) = particles.velocities.at(i) + (particles.forces.at(i) * halfDt);

        if (damping > 0.0f) {
            particles.velocities.at(i) = particles.velocities.at(i) * std::max(0.0f, 1.0f - (damping * dt));
        }

        particles.positions.at(i) = particles.positions.at(i) + (particles.velocities.at(i) * dt);

        // Reflective boundary checks
        if (particles.positions.at(i).x > limit) {
            particles.positions.at(i).x  = limit;
            particles.velocities.at(i).x = -std::abs(particles.velocities.at(i).x);
        } else if (particles.positions.at(i).x < -limit) {
            particles.positions.at(i).x  = -limit;
            particles.velocities.at(i).x = std::abs(particles.velocities.at(i).x);
        }

        if (particles.positions.at(i).y > limit) {
            particles.positions.at(i).y  = limit;
            particles.velocities.at(i).y = -std::abs(particles.velocities.at(i).y);
        } else if (particles.positions.at(i).y < -limit) {
            particles.positions.at(i).y  = -limit;
            particles.velocities.at(i).y = std::abs(particles.velocities.at(i).y);
        }

        if (particles.positions.at(i).z > limit) {
            particles.positions.at(i).z  = limit;
            particles.velocities.at(i).z = -std::abs(particles.velocities.at(i).z);
        } else if (particles.positions.at(i).z < -limit) {
            particles.positions.at(i).z  = -limit;
            particles.velocities.at(i).z = std::abs(particles.velocities.at(i).z);
        }
    }

    // Step 2: Compute forces at updated positions
    computeForces();

    // Step 3: Complete velocity step
    for (size_t i = 0; i < numAtoms; ++i) {
        particles.velocities.at(i) = particles.velocities.at(i) + (particles.forces.at(i) * halfDt);
    }
}

void MolecularDynamics::onUpdate(float deltaTime) {
    if (isPaused) {
        return;
    }

    elapsedTime += deltaTime;

    constexpr float SUB_DT      = 0.004f;
    const int       maxSubSteps = (numAtoms > 300) ? 2 : 4;

    timeAccumulator += std::min(deltaTime, 0.02f);

    int subSteps = 0;
    while (timeAccumulator >= SUB_DT && subSteps < maxSubSteps) {
        integrate(SUB_DT);
        timeAccumulator -= SUB_DT;
        ++subSteps;
    }

    if (timeAccumulator > (SUB_DT * static_cast<float>(maxSubSteps))) {
        timeAccumulator = 0.0f;
    }

    float totalKinetic = 0.0f;
    for (size_t i = 0; i < numAtoms; ++i) {
        totalKinetic += 0.5f * particles.velocities.at(i).dot(particles.velocities.at(i));
    }

    currentKineticEnergy = totalKinetic;
    currentTemperature   = (numAtoms > 0) ? (2.0f * totalKinetic) / (3.0f * static_cast<float>(numAtoms)) : 0.0f;

    for (size_t i = 0; i < numAtoms; ++i) {
        atomEntities.at(i)->setPosition(particles.positions.at(i));
    }
}

void MolecularDynamics::renderImGui() {
    ImGui::SetNextWindowPos(ImVec2(10.0f, 28.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(340, 360), ImGuiCond_FirstUseEver);
    ImGui::Begin("Controls");

    if (ImGui::SliderInt("Particle Count", &uiAtomCount, 1, 100)) {
        uiAtomCount = std::clamp(uiAtomCount, 1, 100);
    }

    if (ImGui::Button(isPaused ? " Resume " : " Pause ")) {
        isPaused = !isPaused;
    }

    ImGui::SameLine();
    if (ImGui::Button("Rescale Velocities") && currentTemperature > 0.001f) {
        float factor = std::sqrt(targetTemperature / currentTemperature);
        for (size_t i = 0; i < numAtoms; ++i) {
            particles.velocities.at(i) = particles.velocities.at(i) * factor;
        }
    }

    ImGui::Separator();
    if (ImGui::IsItemDeactivatedAfterEdit() && uiAtomCount > 0 && std::cmp_not_equal(uiAtomCount, numAtoms)) {
        numAtoms = static_cast<size_t>(uiAtomCount);
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

void MolecularDynamics::onRender(Renderer& renderer) {
    renderer.render(scene, getApp()->getWidth(), getApp()->getHeight());
    renderImGui();
}