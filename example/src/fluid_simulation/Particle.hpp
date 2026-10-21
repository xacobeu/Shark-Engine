#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <span>
#include <vector>

#include "SharkEngine.hpp"

struct Particle {
    Vector3 position{.x = 0.0f, .y = 0.0f, .z = 0.0f};
    Vector3 prevPosition{.x = 0.0f, .y = 0.0f, .z = 0.0f};
    Vector3 velocity{.x = 0.0f, .y = 0.0f, .z = 0.0f};

    void clampToBounds(Vector3 bounds, float radius) {
        const Vector3 half = (bounds * 0.5f) - Vector3{radius, radius, radius};
        position.x         = std::clamp(position.x, -half.x, half.x);
        position.y         = std::clamp(position.y, -half.y, half.y);
        position.z         = std::clamp(position.z, -half.z, half.z);
    }

    void
    resolveBoundaryVelocity(Vector3 bounds, float radius, float dt, float restitution = 0.35f, float friction = 0.0f) {
        const Vector3   half             = (bounds * 0.5f) - Vector3{radius, radius, radius};
        constexpr float BOUNCE_THRESHOLD = 0.20f;
        constexpr float EPSILON          = 1e-4f;
        const float     frictionFactor   = std::clamp(1.0f - friction, 0.0f, 1.0f);

        if (position.x >= half.x - EPSILON && velocity.x > 0.0f) {
            velocity.x = -velocity.x * restitution;
            velocity.y *= frictionFactor;
            velocity.z *= frictionFactor;
        } else if (position.x <= -half.x + EPSILON && velocity.x < 0.0f) {
            velocity.x = -velocity.x * restitution;
            velocity.y *= frictionFactor;
            velocity.z *= frictionFactor;
        }

        if (position.z >= half.z - EPSILON && velocity.z > 0.0f) {
            velocity.z = -velocity.z * restitution;
            velocity.x *= frictionFactor;
            velocity.y *= frictionFactor;
        } else if (position.z <= -half.z + EPSILON && velocity.z < 0.0f) {
            velocity.z = -velocity.z * restitution;
            velocity.x *= frictionFactor;
            velocity.y *= frictionFactor;
        }

        if (position.y <= -half.y + EPSILON) {
            if (velocity.y < -BOUNCE_THRESHOLD) {
                velocity.y = -velocity.y * restitution;
            } else if (std::abs(velocity.x) < 0.35f && std::abs(velocity.z) < 0.35f) {
                velocity.y = 0.0f;
            }
            velocity.x *= frictionFactor;
            velocity.z *= frictionFactor;
        } else if (position.y >= half.y - EPSILON && velocity.y > 0.0f) {
            velocity.y = -velocity.y * restitution;
            velocity.x *= frictionFactor;
            velocity.z *= frictionFactor;
        }

        prevPosition = position - (velocity * dt);
    }

    void resolveBoundary(Vector3 bounds, float radius, float dt, float restitution = 0.35f, float friction = 0.0f) {
        clampToBounds(bounds, radius);
        resolveBoundaryVelocity(bounds, radius, dt, restitution, friction);
    }
};

inline void resolvePairPBD(Particle& a, Particle& b, float minDistance, float minDistSq, float relaxation) {
    Vector3 delta  = a.position - b.position;
    float   distSq = (delta.x * delta.x) + (delta.y * delta.y) + (delta.z * delta.z);
    if (distSq >= minDistSq || distSq < 1e-12f) {
        return;
    }

    float invDist     = 1.0f / std::sqrt(distSq);
    float dist        = distSq * invDist;
    float penetration = minDistance - dist;

    float   scale      = std::min(penetration * 0.5f * relaxation, minDistance * 0.10f) * invDist;
    Vector3 correction = delta * scale;
    a.position         = a.position + correction;
    b.position         = b.position - correction;
}

class SpatialHashGrid {
public:
    void init(Vector3 bounds, float cellSize) {
        this->bounds      = bounds;
        this->cellSize    = cellSize;
        this->invCellSize = 1.0f / cellSize;

        innerCellsX = std::max(1, static_cast<int>(std::ceil(bounds.x * invCellSize)));
        innerCellsY = std::max(1, static_cast<int>(std::ceil(bounds.y * invCellSize)));
        innerCellsZ = std::max(1, static_cast<int>(std::ceil(bounds.z * invCellSize)));

        nCellsX = innerCellsX + 2;
        nCellsY = innerCellsY + 2;
        nCellsZ = innerCellsZ + 2;

        const auto totalCells = static_cast<std::size_t>(nCellsX * nCellsY * nCellsZ);
        cellStart.resize(totalCells + 1, 0);
        cellCount.resize(totalCells, 0);
        activeCells.reserve(totalCells / 4);

        // Precompute 1D flat offsets for the 13 forward half-space neighbors
        static constexpr std::array<std::array<int, 3>, 13> OFFSETS_3D = {
            {{1, 0, 0},
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
             {1, 1, 1}}
        };

        for (std::size_t i = 0; i < 13; ++i) {
            neighborOffsets[i] = OFFSETS_3D[i][0] + (nCellsX * (OFFSETS_3D[i][1] + (nCellsY * OFFSETS_3D[i][2])));
        }
    }

    // Counting sort builds a spatially sorted particle array in O(N) time
    void build(std::vector<Particle>& particles) {
        const std::size_t N = particles.size();
        if (N == 0) {
            return;
        }

        const auto totalCells = static_cast<std::size_t>(nCellsX * nCellsY * nCellsZ);
        std::ranges::fill(cellCount, 0);
        activeCells.clear();

        particleCell.resize(N);
        sortedParticles.resize(N);

        const float halfX = bounds.x * 0.5f;
        const float halfY = bounds.y * 0.5f;
        const float halfZ = bounds.z * 0.5f;

        // 1. Compute cell index for each particle (clamped inside the [1 .. innerCells] padded region)
        for (std::size_t i = 0; i < N; ++i) {
            int cx = std::clamp(static_cast<int>((particles[i].position.x + halfX) * invCellSize) + 1, 1, innerCellsX);
            int cy = std::clamp(static_cast<int>((particles[i].position.y + halfY) * invCellSize) + 1, 1, innerCellsY);
            int cz = std::clamp(static_cast<int>((particles[i].position.z + halfZ) * invCellSize) + 1, 1, innerCellsZ);

            int cIdx        = cx + (nCellsX * (cy + (nCellsY * cz)));
            particleCell[i] = cIdx;

            if (cellCount[static_cast<std::size_t>(cIdx)]++ == 0) {
                activeCells.push_back(cIdx);
            }
        }

        // 2. Exclusive prefix sum to find cell start indices
        int sum = 0;
        for (std::size_t c = 0; c < totalCells; ++c) {
            cellStart[c] = sum;
            sum += cellCount[c];
        }
        cellStart[totalCells] = sum;

        // 3. Scatter particles into contiguous sorted order
        // Copy cellStart into cellWriteCursor to place particles
        cellWriteCursor.assign(cellStart.begin(), cellStart.end() - 1);
        for (std::size_t i = 0; i < N; ++i) {
            int cIdx                                          = particleCell[i];
            int dstIdx                                        = cellWriteCursor[static_cast<std::size_t>(cIdx)]++;
            sortedParticles[static_cast<std::size_t>(dstIdx)] = particles[i];
        }

        // Swap sorted buffer into active particles vector (O(1) pointer swap, zero allocation)
        particles.swap(sortedParticles);
    }

    void solveCollisions(std::span<Particle> particles, float minDistance, float relaxation = 0.35f) {
        const float minDistSq      = minDistance * minDistance;
        Particle* __restrict pData = particles.data();

        // Only iterate over cells that actually contain particles!
        for (int cIdx : activeCells) {
            const int startA = cellStart[static_cast<std::size_t>(cIdx)];
            const int endA   = startA + cellCount[static_cast<std::size_t>(cIdx)];

            // 1. Same cell pairs (contiguous in memory!)
            for (int a = startA; a < endA; ++a) {
                for (int b = a + 1; b < endA; ++b) {
                    resolvePairPBD(pData[a], pData[b], minDistance, minDistSq, relaxation);
                }
            }

            // 2. Neighbor cells (no bounds check needed thanks to 1-cell padding)
            for (int offset : neighborOffsets) {
                const int nIdx   = cIdx + offset;
                const int countB = cellCount[static_cast<std::size_t>(nIdx)];
                if (countB == 0) {
                    continue;
                }

                const int startB = cellStart[static_cast<std::size_t>(nIdx)];
                const int endB   = startB + countB;

                for (int a = startA; a < endA; ++a) {
                    for (int b = startB; b < endB; ++b) {
                        resolvePairPBD(pData[a], pData[b], minDistance, minDistSq, relaxation);
                    }
                }
            }
        }
    }

    void applyViscosity(std::span<Particle> particles, float h, float viscosityCoeff) {
        if (viscosityCoeff <= 0.0f) {
            return;
        }

        const float hSq            = h * h;
        const float invH           = 1.0f / h;
        Particle* __restrict pData = particles.data();

        auto interact = [&](Particle& a, Particle& b) {
            Vector3 delta  = a.position - b.position;
            float   distSq = (delta.x * delta.x) + (delta.y * delta.y) + (delta.z * delta.z);
            if (distSq >= hSq || distSq < 1e-12f) {
                return;
            }
            float   dist = std::sqrt(distSq);
            float   q    = 1.0f - (dist * invH);
            Vector3 dv   = (b.velocity - a.velocity) * (viscosityCoeff * q);
            a.velocity   = a.velocity + dv;
            b.velocity   = b.velocity - dv;
        };

        for (int cIdx : activeCells) {
            const int startA = cellStart[static_cast<std::size_t>(cIdx)];
            const int endA   = startA + cellCount[static_cast<std::size_t>(cIdx)];

            for (int a = startA; a < endA; ++a) {
                for (int b = a + 1; b < endA; ++b) {
                    interact(pData[a], pData[b]);
                }
            }

            for (int offset : neighborOffsets) {
                const int nIdx   = cIdx + offset;
                const int countB = cellCount[static_cast<std::size_t>(nIdx)];
                if (countB == 0) {
                    continue;
                }

                const int startB = cellStart[static_cast<std::size_t>(nIdx)];
                const int endB   = startB + countB;

                for (int a = startA; a < endA; ++a) {
                    for (int b = startB; b < endB; ++b) {
                        interact(pData[a], pData[b]);
                    }
                }
            }
        }
    }

private:
    Vector3 bounds{1.0f, 1.0f, 1.0f};
    float   cellSize    = 1.0f;
    float   invCellSize = 1.0f;
    int     innerCellsX = 1, innerCellsY = 1, innerCellsZ = 1;
    int     nCellsX = 3, nCellsY = 3, nCellsZ = 3;

    std::array<int, 13>   neighborOffsets{};
    std::vector<int>      cellStart;
    std::vector<int>      cellCount;
    std::vector<int>      cellWriteCursor;
    std::vector<int>      activeCells;
    std::vector<int>      particleCell;
    std::vector<Particle> sortedParticles;
};