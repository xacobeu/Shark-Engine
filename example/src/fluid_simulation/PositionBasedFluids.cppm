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
        if (r >= h || r < 1e-6f) {
            return {};
        }
        const float d = h - r;
        return delta * (spikyGradCoef * d * d / r);
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
        float tensileStrength = 0.1f;  // Unstable above ~0.2
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

        const float hSq = kernel.radiusSq();
        shark::exec::for_each_index(shark::exec::par_unseq, count, [this, &grid, particles, hSq](std::size_t i) {
            const Vector3  xi    = particles[i].position;
            std::uint32_t* list  = &neighbors[i * MAX_NEIGHBORS];
            std::uint32_t  found = 0;
            grid.forEachCandidate(xi, [&](std::uint32_t j) {
                if (j != i && found < MAX_NEIGHBORS && (xi - particles[j].position).lengthSq() < hSq) {
                    list[found++] = j;
                }
            });
            neighborCounts[i] = found;
        });
    }

    void
    solveDensity(std::span<Particle> particles, Vector3 bounds, float particleRadius, const Settings& settings) const {
        const float selfDensity = kernel.poly6(0.0f);
        const float epsilon     = settings.compliance * restGradientSqSum;
        const float tensileK = settings.tensileStrength / restGradientSqSum;

        for (int iter = 0; iter < settings.iterations; ++iter) {
            shark::exec::for_each_index(
                shark::exec::par_unseq, particles.size(), [this, particles, selfDensity, epsilon](std::size_t i) {
                    float   density = selfDensity;
                    Vector3 gradSum{};
                    float   gradSqSum = 0.0f;
                    forEachNeighbor(i, particles, [&](const Particle&, const Vector3& delta, float rSq) {
                        density += kernel.poly6(rSq);
                        const Vector3 grad = kernel.spikyGradient(delta, std::sqrt(rSq)) * invRestDensity;
                        gradSum += grad;
                        gradSqSum += grad.lengthSq();
                    });

                    const float constraint = std::max((density * invRestDensity) - 1.0f, 0.0f);
                    particles[i].lambda    = -constraint / (gradSum.lengthSq() + gradSqSum + epsilon);
                }
            );

            shark::exec::for_each_index(
                shark::exec::par_unseq, particles.size(), [this, particles, tensileK](std::size_t i) {
                    Particle& p = particles[i];
                    Vector3   correction{};
                    forEachNeighbor(i, particles, [&](const Particle& q, const Vector3& delta, float rSq) {
                        const float ratio   = kernel.poly6(rSq) * invPoly6AtDq;
                        const float ratioSq = ratio * ratio;
                        const float sCorr   = -tensileK * ratioSq * ratioSq;
                        correction += kernel.spikyGradient(delta, std::sqrt(rSq)) * (p.lambda + q.lambda + sCorr);
                    });
                    p.positionDelta = correction * invRestDensity;
                }
            );

            shark::exec::for_each(shark::exec::par_unseq, particles, [bounds, particleRadius](Particle& p) {
                p.position += p.positionDelta;
                p.clampToBounds(bounds, particleRadius);
            });
        }
    }

    void applyVelocityCorrections(std::span<Particle> particles, const Settings& settings, float dt) const {
        shark::exec::for_each_index(shark::exec::par_unseq, particles.size(), [this, particles](std::size_t i) {
            Particle& p = particles[i];
            Vector3   omega{};
            Vector3   blend{};
            forEachNeighbor(i, particles, [&](const Particle& q, const Vector3& delta, float rSq) {
                const Vector3 dv = q.velocity - p.velocity;
                omega += cross(dv, kernel.spikyGradient(delta, std::sqrt(rSq)));
                blend += dv * kernel.poly6(rSq);
            });
            p.vorticity          = omega * invRestDensity;
            p.vorticityMagnitude = p.vorticity.length();
            p.viscosityDelta     = blend * invRestDensity;
        });

        const float impulse   = settings.vorticity * dt;
        const float viscosity = settings.viscosity;
        shark::exec::for_each_index(
            shark::exec::par_unseq, particles.size(), [this, particles, impulse, viscosity](std::size_t i) {
                Particle& p = particles[i];
                p.velocity += p.viscosityDelta * viscosity;

                if (impulse <= 0.0f) {
                    return;
                }
                Vector3 eta{};
                forEachNeighbor(i, particles, [&](const Particle& q, const Vector3& delta, float rSq) {
                    eta += kernel.spikyGradient(delta, std::sqrt(rSq)) * (q.vorticityMagnitude - p.vorticityMagnitude);
                });
                const float len = eta.length();
                if (len > 1e-6f) {
                    p.velocity += cross(eta / len, p.vorticity) * impulse;
                }
            }
        );
    }

    [[nodiscard]] float kernelRadius() const { return kernel.radius(); }

private:
    template <typename Fn> void forEachNeighbor(std::size_t i, std::span<Particle> particles, Fn&& fn) const {
        const Vector3        xi    = particles[i].position;
        const std::uint32_t* list  = &neighbors[i * MAX_NEIGHBORS];
        const std::uint32_t  count = neighborCounts[i];
        for (std::uint32_t n = 0; n < count; ++n) {
            const Particle& q     = particles[list[n]];
            const Vector3   delta = xi - q.position;
            const float     rSq   = delta.lengthSq();
            if (rSq < kernel.radiusSq()) {
                fn(q, delta, rSq);
            }
        }
    }

    SPHKernel kernel;
    float     invRestDensity    = 1.0f;
    float     restGradientSqSum = 1.0f;
    float     invPoly6AtDq      = 1.0f;

    std::vector<std::uint32_t> neighbors;
    std::vector<std::uint32_t> neighborCounts;
};

} // export
