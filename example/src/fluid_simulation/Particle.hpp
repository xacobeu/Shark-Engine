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
        position           = clamp(position, -half, half);
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
    const Vector3 delta  = a.position - b.position;
    const float   distSq = delta.lengthSq();
    if (distSq >= minDistSq || distSq < 1e-12f) {
        return;
    }

    const float invDist     = 1.0f / std::sqrt(distSq);
    const float dist        = distSq * invDist;
    const float penetration = minDistance - dist;

    const float   scale      = std::min(penetration * 0.5f * relaxation, minDistance * 0.10f) * invDist;
    const Vector3 correction = delta * scale;
    a.position += correction;
    b.position -= correction;
}

class SpatialHashGrid {
public:
    // 3x3x2 coloring: same-colored cells never share a forward neighbor, so a color can run in parallel
    static constexpr std::size_t NUM_COLORS = 18;

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
        for (auto& bucket : colorCells) {
            bucket.reserve(totalCells / (NUM_COLORS * 2));
        }

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
        shark::exec::fill(shark::exec::par_unseq, cellCount, 0);
        for (auto& bucket : colorCells) {
            bucket.clear();
        }

        particleCell.resize(N);
        sortedParticles.resize(N);

        const float halfX = bounds.x * 0.5f;
        const float halfY = bounds.y * 0.5f;
        const float halfZ = bounds.z * 0.5f;

        // Compute cell index for each particle (clamped inside the [1 .. innerCells] padded region)
        for (std::size_t i = 0; i < N; ++i) {
            int cx = std::clamp(static_cast<int>((particles[i].position.x + halfX) * invCellSize) + 1, 1, innerCellsX);
            int cy = std::clamp(static_cast<int>((particles[i].position.y + halfY) * invCellSize) + 1, 1, innerCellsY);
            int cz = std::clamp(static_cast<int>((particles[i].position.z + halfZ) * invCellSize) + 1, 1, innerCellsZ);

            int cIdx        = cx + (nCellsX * (cy + (nCellsY * cz)));
            particleCell[i] = cIdx;

            if (cellCount[static_cast<std::size_t>(cIdx)]++ == 0) {
                const auto color = static_cast<std::size_t>((cx % 3) + (3 * (cy % 3)) + (9 * (cz & 1)));
                colorCells[color].push_back(cIdx);
            }
        }

        // Exclusive prefix sum to find cell start indices
        int sum = 0;
        for (std::size_t c = 0; c < totalCells; ++c) {
            cellStart[c] = sum;
            sum += cellCount[c];
        }
        cellStart[totalCells] = sum;

        // Scatter particles into contiguous sorted order
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

        // Process each conflict-free spatial color in parallel
        for (const auto& bucket : colorCells) {
            shark::exec::for_each(shark::exec::par, bucket, [&](int cIdx) {
                const int startA = cellStart[static_cast<std::size_t>(cIdx)];
                const int endA   = startA + cellCount[static_cast<std::size_t>(cIdx)];

                // Same cell pairs (contiguous in memory)
                for (int a = startA; a < endA; ++a) {
                    for (int b = a + 1; b < endA; ++b) {
                        resolvePairPBD(pData[a], pData[b], minDistance, minDistSq, relaxation);
                    }
                }

                // Neighbor cells (no bounds check needed thanks to 1-cell padding)
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
            });
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
            const Vector3 delta  = a.position - b.position;
            const float   distSq = delta.lengthSq();
            if (distSq >= hSq || distSq < 1e-12f) {
                return;
            }
            const float   dist = std::sqrt(distSq);
            const float   q    = 1.0f - (dist * invH);
            const Vector3 dv   = (b.velocity - a.velocity) * (viscosityCoeff * q);
            a.velocity += dv;
            b.velocity -= dv;
        };

        for (const auto& bucket : colorCells) {
            shark::exec::for_each(shark::exec::par, bucket, [&](int cIdx) {
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
            });
        }
    }

private:
    Vector3 bounds{1.0f, 1.0f, 1.0f};
    float   cellSize    = 1.0f;
    float   invCellSize = 1.0f;
    int     innerCellsX = 1, innerCellsY = 1, innerCellsZ = 1;
    int     nCellsX = 3, nCellsY = 3, nCellsZ = 3;

    std::array<int, 13>                      neighborOffsets{};
    std::vector<int>                         cellStart;
    std::vector<int>                         cellCount;
    std::vector<int>                         cellWriteCursor;
    std::array<std::vector<int>, NUM_COLORS> colorCells;
    std::vector<int>                         particleCell;
    std::vector<Particle>                    sortedParticles;
};
