export module examples.fluid_simulation.particle;

import std;

import shark;

using namespace shark;

export {
struct Particle {
    Vector3 position{.x = 0.0f, .y = 0.0f, .z = 0.0f};
    Vector3 prevPosition{.x = 0.0f, .y = 0.0f, .z = 0.0f};
    Vector3 velocity{.x = 0.0f, .y = 0.0f, .z = 0.0f};

    inline void clampToBounds(Vector3 bounds, float radius) {
        const Vector3 half = (bounds * 0.5f) - Vector3{radius, radius, radius};
        position           = clamp(position, -half, half);
    }

    inline void
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

    inline void
    resolveBoundary(Vector3 bounds, float radius, float dt, float restitution = 0.35f, float friction = 0.0f) {
        clampToBounds(bounds, radius);
        resolveBoundaryVelocity(bounds, radius, dt, restitution, friction);
    }
};

class SpatialHashGrid {
public:
    inline void init(Vector3 bounds, float cellSize) {
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
    }

    inline void build(std::vector<Particle>& particles) {
        const std::size_t N = particles.size();
        if (N == 0) {
            return;
        }

        const auto totalCells = static_cast<std::size_t>(nCellsX * nCellsY * nCellsZ);
        shark::exec::fill(shark::exec::par_unseq, cellCount, 0);

        particleCell.resize(N);
        sortedParticles.resize(N);

        for (std::size_t i = 0; i < N; ++i) {
            const int cIdx  = cellIndex(particles[i].position);
            particleCell[i] = cIdx;
            ++cellCount[static_cast<std::size_t>(cIdx)];
        }

        int sum = 0;
        for (std::size_t c = 0; c < totalCells; ++c) {
            cellStart[c] = sum;
            sum += cellCount[c];
        }
        cellStart[totalCells] = sum;

        cellWriteCursor.assign(cellStart.begin(), cellStart.end() - 1);
        for (std::size_t i = 0; i < N; ++i) {
            int cIdx                                          = particleCell[i];
            int dstIdx                                        = cellWriteCursor[static_cast<std::size_t>(cIdx)]++;
            sortedParticles[static_cast<std::size_t>(dstIdx)] = particles[i];
        }

        particles.swap(sortedParticles);
    }

    template <typename Fn> inline void forEachCandidate(const Vector3& position, Fn&& fn) const {
        const int center = cellIndex(position);
        for (int dz = -1; dz <= 1; ++dz) {
            for (int dy = -1; dy <= 1; ++dy) {
                const int rowStart = center + (nCellsX * (dy + (nCellsY * dz))) - 1;
                const int begin    = cellStart[static_cast<std::size_t>(rowStart)];
                const int end      = cellStart[static_cast<std::size_t>(rowStart + 3)];
                for (int j = begin; j < end; ++j) {
                    fn(static_cast<std::uint32_t>(j));
                }
            }
        }
    }

private:
    [[nodiscard]] inline int cellIndex(const Vector3& position) const {
        const int cx = std::clamp(static_cast<int>((position.x + (bounds.x * 0.5f)) * invCellSize) + 1, 1, innerCellsX);
        const int cy = std::clamp(static_cast<int>((position.y + (bounds.y * 0.5f)) * invCellSize) + 1, 1, innerCellsY);
        const int cz = std::clamp(static_cast<int>((position.z + (bounds.z * 0.5f)) * invCellSize) + 1, 1, innerCellsZ);
        return cx + (nCellsX * (cy + (nCellsY * cz)));
    }

    Vector3 bounds{1.0f, 1.0f, 1.0f};
    float   cellSize    = 1.0f;
    float   invCellSize = 1.0f;
    int     innerCellsX = 1, innerCellsY = 1, innerCellsZ = 1;
    int     nCellsX = 3, nCellsY = 3, nCellsZ = 3;

    std::vector<int>      cellStart;
    std::vector<int>      cellCount;
    std::vector<int>      cellWriteCursor;
    std::vector<int>      particleCell;
    std::vector<Particle> sortedParticles;
};

} // export
