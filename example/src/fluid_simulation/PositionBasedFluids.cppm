export module examples.fluid_simulation.pbf;

import std;

import examples.fluid_simulation.particle;

import shark;

using namespace shark;

export {
// Müller et al. 2003
class SPHKernel {
public:
    SPHKernel() : SPHKernel(1.0f) {}
    explicit SPHKernel(float radius)
        : h(radius),
          hSq(radius * radius),
          poly6Coef(315.0f / (64.0f * std::numbers::pi_v<float> * std::pow(radius, 9.0f))),
          spikyGradCoef(-45.0f / (std::numbers::pi_v<float> * std::pow(radius, 6.0f))) {}

    [[nodiscard]] float radius() const { return h; }
    [[nodiscard]] float radiusSq() const { return hSq; }

    [[nodiscard]] float poly6(float rSq) const {
        if (rSq >= hSq) {
            return 0.0f;
        }
        const float d = hSq - rSq;
        return poly6Coef * d * d * d;
    }

    [[nodiscard]] Vector3 spikyGradient(const Vector3& delta, float r) const {
        if (r >= h) {
            return {};
        }
        return spikyGradientInRange(delta, r);
    }

    [[nodiscard]] Vector3 spikyGradientInRange(const Vector3& delta, float r) const {
        if (r < 1e-6f) {
            return {};
        }
        const float d = h - r;
        return delta * (spikyGradCoef * d * d / r);
    }

    [[nodiscard]] float poly6InRange(float rSq) const {
        const float d = hSq - rSq;
        return poly6Coef * d * d * d;
    }

private:
    float h;
    float hSq;
    float poly6Coef;
    float spikyGradCoef;
};

// Position Based Fluids (Macklin & Müller 2013).
class PositionBasedFluidSolver {
public:
    struct Settings {
        int   iterations      = 4;
        float compliance      = 0.1f;
        float tensileStrength = 0.1f; // Unstable above ~0.2
        float vorticity       = 0.05f;
        float viscosity       = 0.02f;
    };

    static constexpr std::size_t MAX_NEIGHBORS = 64;

    void init(float restSpacing, float kernelRadius) {
        kernel = SPHKernel(kernelRadius);

        const int n       = static_cast<int>(std::ceil(kernelRadius / restSpacing));
        float     density = 0.0f;
        float     gradSq  = 0.0f;
        for (int x = -n; x <= n; ++x) {
            for (int y = -n; y <= n; ++y) {
                for (int z = -n; z <= n; ++z) {
                    const Vector3 offset = Vector3{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)}
                                           * restSpacing;
                    density += kernel.poly6(offset.lengthSq());
                    gradSq += kernel.spikyGradient(offset, offset.length()).lengthSq();
                }
            }
        }

        invRestDensity    = 1.0f / density;
        restGradientSqSum = gradSq * invRestDensity * invRestDensity;

        const float dq = 0.2f * kernelRadius;
        invPoly6AtDq   = 1.0f / kernel.poly6(dq * dq);
    }

    void findNeighbors(const SpatialHashGrid& grid, std::span<const Particle> particles) {
        const std::size_t count = particles.size();
        neighborCounts.resize(count);
        neighbors.resize(count * MAX_NEIGHBORS);
        positions.resize(count);
        for (std::size_t i = 0; i < count; ++i) {
            positions[i] = particles[i].position;
        }

        const float hSq = kernel.radiusSq();
        shark::exec::for_each_index(shark::exec::par_unseq, count, [this, &grid, hSq](std::size_t i) {
            const Vector3  xi    = positions[i];
            std::uint32_t* list  = &neighbors[i * MAX_NEIGHBORS];
            std::uint32_t  found = 0;
            grid.forEachCandidate(xi, [&](std::uint32_t j) {
                if (j != i && found < MAX_NEIGHBORS && (xi - positions[j]).lengthSq() < hSq) {
                    list[found++] = j;
                }
            });
            neighborCounts[i] = found;
        });
    }

    void solveDensity(std::span<Particle> particles, Vector3 bounds, float particleRadius, const Settings& settings) {
        const std::size_t count       = particles.size();
        const float       selfDensity = kernel.poly6(0.0f);
        const float       epsilon     = settings.compliance * restGradientSqSum;
        const float       tensileK    = settings.tensileStrength / restGradientSqSum;
        const Vector3     half        = (bounds * 0.5f) - Vector3{particleRadius, particleRadius, particleRadius};

        lambdas.resize(count);
        positionDeltas.resize(count);

        for (int iter = 0; iter < settings.iterations; ++iter) {
            shark::exec::for_each_index(shark::exec::par_unseq, count, [this, selfDensity, epsilon](std::size_t i) {
                float   density = selfDensity;
                Vector3 gradSum{};
                float   gradSqSum = 0.0f;
                forEachNeighbor(i, [&](std::uint32_t, const Vector3& delta, float rSq, float r) {
                    density += kernel.poly6InRange(rSq);
                    const Vector3 grad = kernel.spikyGradientInRange(delta, r);
                    gradSum += grad;
                    gradSqSum += grad.lengthSq();
                });

                const float invRestDensitySq = invRestDensity * invRestDensity;
                const float constraint       = std::max((density * invRestDensity) - 1.0f, 0.0f);
                lambdas[i] = -constraint / (((gradSum.lengthSq() + gradSqSum) * invRestDensitySq) + epsilon);
            });

            shark::exec::for_each_index(shark::exec::par_unseq, count, [this, tensileK](std::size_t i) {
                const float lambdaI = lambdas[i];
                Vector3     correction{};
                forEachNeighbor(i, [&](std::uint32_t j, const Vector3& delta, float rSq, float r) {
                    const float ratio   = kernel.poly6InRange(rSq) * invPoly6AtDq;
                    const float ratioSq = ratio * ratio;
                    const float sCorr   = -tensileK * ratioSq * ratioSq;
                    correction += kernel.spikyGradientInRange(delta, r) * (lambdaI + lambdas[j] + sCorr);
                });
                positionDeltas[i] = correction * invRestDensity;
            });

            shark::exec::for_each_index(shark::exec::par_unseq, count, [this, half](std::size_t i) {
                positions[i] = clamp(positions[i] + positionDeltas[i], -half, half);
            });
        }

        for (std::size_t i = 0; i < count; ++i) {
            particles[i].position = positions[i];
        }
    }

    void applyVelocityCorrections(std::span<Particle> particles, const Settings& settings, float dt) {
        const std::size_t count = particles.size();
        velocities.resize(count);
        vorticities.resize(count);
        vorticityMagnitudes.resize(count);
        for (std::size_t i = 0; i < count; ++i) {
            velocities[i] = particles[i].velocity;
        }

        const float viscosity = settings.viscosity;
        shark::exec::for_each_index(shark::exec::par_unseq, count, [this, particles, viscosity](std::size_t i) {
            const Vector3 vi = velocities[i];
            Vector3       omega{};
            Vector3       blend{};
            forEachNeighbor(i, [&](std::uint32_t j, const Vector3& delta, float rSq, float r) {
                const Vector3 dv = velocities[j] - vi;
                omega += cross(dv, kernel.spikyGradientInRange(delta, r));
                blend += dv * kernel.poly6InRange(rSq);
            });
            vorticities[i]         = omega * invRestDensity;
            vorticityMagnitudes[i] = vorticities[i].length();
            particles[i].velocity += blend * (invRestDensity * viscosity);
        });

        const float impulse = settings.vorticity * dt;
        if (impulse <= 0.0f) {
            return;
        }
        shark::exec::for_each_index(shark::exec::par_unseq, count, [this, particles, impulse](std::size_t i) {
            const float magnitudeI = vorticityMagnitudes[i];
            Vector3     eta{};
            forEachNeighbor(i, [&](std::uint32_t j, const Vector3& delta, float, float r) {
                eta += kernel.spikyGradientInRange(delta, r) * (vorticityMagnitudes[j] - magnitudeI);
            });
            const float len = eta.length();
            if (len > 1e-6f) {
                particles[i].velocity += cross(eta / len, vorticities[i]) * impulse;
            }
        });
    }

    [[nodiscard]] float kernelRadius() const { return kernel.radius(); }

private:
    template <typename Fn> void forEachNeighbor(std::size_t i, Fn&& fn) const {
        const Vector3        xi    = positions[i];
        const std::uint32_t* list  = &neighbors[i * MAX_NEIGHBORS];
        const std::uint32_t  count = neighborCounts[i];
        const float          hSq   = kernel.radiusSq();
        for (std::uint32_t n = 0; n < count; ++n) {
            const std::uint32_t j     = list[n];
            const Vector3       delta = xi - positions[j];
            const float         rSq   = delta.lengthSq();
            if (rSq < hSq) {
                fn(j, delta, rSq, std::sqrt(rSq));
            }
        }
    }

    SPHKernel kernel;
    float     invRestDensity    = 1.0f;
    float     restGradientSqSum = 1.0f;
    float     invPoly6AtDq      = 1.0f;

    std::vector<std::uint32_t> neighbors;
    std::vector<std::uint32_t> neighborCounts;

    std::vector<Vector3> positions;
    std::vector<Vector3> positionDeltas;
    std::vector<float>   lambdas;
    std::vector<Vector3> velocities;
    std::vector<Vector3> vorticities;
    std::vector<float>   vorticityMagnitudes;
};

} // export
