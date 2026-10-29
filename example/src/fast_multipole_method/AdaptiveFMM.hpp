#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

#include "Particle.hpp"
#include "util/Vector.hpp"

struct MultipoleExpansion {
    float                mass = 0.0f;
    float                rMax = 0.0f;
    Vector3              com{0.0f, 0.0f, 0.0f};
    std::array<float, 6> quadrupole{}; // 0.5 * sum m_i dx_i dx_j (xx, yy, zz, xy, yz, zx)

    void clear() noexcept {
        mass = 0.0f;
        rMax = 0.0f;
        com  = {0.0f, 0.0f, 0.0f};
        quadrupole.fill(0.0f);
    }
};

struct LocalExpansion {
    Vector3               F{0.0f, 0.0f, 0.0f}; // 0th-order acceleration at CoM: -grad(Phi)
    std::array<float, 6>  J{};                 // 1st-order field gradient (xx, yy, zz, xy, yz, zx)
    std::array<float, 10> H{}; // 2nd-order field Hessian (xxx, yyy, zzz, xxy, xxz, yyx, yyz, zzx, zzy, xyz)

    void clear() noexcept {
        F = {0.0f, 0.0f, 0.0f};
        J.fill(0.0f);
        H.fill(0.0f);
    }
};

inline constexpr std::uint32_t INVALID_NODE = std::numeric_limits<std::uint32_t>::max();

struct FMMNode {
    Vector3       center{0.0f, 0.0f, 0.0f};
    float         halfWidth       = 0.0f;
    int           depth           = 0;
    bool          isLeaf          = true;
    std::uint8_t  activeMask      = 0;
    std::uint32_t firstChildIndex = INVALID_NODE;
    std::uint32_t begin           = 0;
    std::uint32_t end             = 0;

    MultipoleExpansion M;
    LocalExpansion     L;

    void reset(const Vector3& c, float hw, int d, std::uint32_t b = 0, std::uint32_t e = 0) noexcept {
        center          = c;
        halfWidth       = hw;
        depth           = d;
        isLeaf          = true;
        activeMask      = 0;
        firstChildIndex = INVALID_NODE;
        begin           = b;
        end             = e;
        M.clear();
        L.clear();
    }

    [[nodiscard]] std::uint32_t count() const noexcept { return end - begin; }
};

class AdaptiveFMM {
public:
    struct Config {
        float       theta     = 0.65f;
        std::size_t nCrit     = 48;
        float       softening = 0.35f;
        int         maxDepth  = 12;
    };

    AdaptiveFMM() : AdaptiveFMM(Config{}) {}
    explicit AdaptiveFMM(const Config& cfg);

    void                        setConfig(const Config& cfg) noexcept { config = cfg; }
    [[nodiscard]] const Config& getConfig() const noexcept { return config; }

    void buildTree(std::span<const fmm::Particle> particles, Vector3 center, float halfWidth);
    void execute(std::span<fmm::Particle> particles, float G);

    // Mean relative error (%) against direct summation over evenly spaced sample particles.
    [[nodiscard]] float
    computeObservedError(std::span<const fmm::Particle> particles, float G, std::size_t sampleCount) const;

    [[nodiscard]] std::size_t getNodeCount() const noexcept { return nodeCount; }

private:
    std::uint32_t allocateChildren(const Vector3& parentCenter, float parentHalfWidth, int parentDepth);

    void       buildNodeRecursive(std::uint32_t nodeIdx, std::span<const fmm::Particle> particles);
    void       collectTaskRoots(std::uint32_t nodeIdx, int targetDepth);
    static int getOctant(const Vector3& center, const Vector3& pos) noexcept;

    void upwardPassM2M(std::uint32_t nodeIdx);
    void computeLeafP2M(FMMNode& node) const noexcept;

    void interactTarget(std::uint32_t targetIdx, std::uint32_t sourceIdx);
    void applyM2L(FMMNode& target, const FMMNode& source, const Vector3& R, float r2) const noexcept;
    void directP2P(const FMMNode& target, const FMMNode& source) noexcept;

    void downwardPass(std::uint32_t nodeIdx);
    void evaluateLeafL2P(const FMMNode& node) noexcept;

    Config               config;
    std::vector<FMMNode> nodePool;
    std::size_t          nodeCount = 0;
    float                currentG  = 1.0f;
    float                eps2      = 0.0f;

    // Reused across frames so steady-state steps don't allocate
    std::vector<std::uint32_t> sortedIndices;
    std::vector<std::uint32_t> scratchIndices;
    std::vector<std::uint8_t>  octantScratch;
    std::vector<std::uint32_t> leafNodes;
    std::vector<std::uint32_t> taskRoots;

    std::vector<float> soaPosX;
    std::vector<float> soaPosY;
    std::vector<float> soaPosZ;
    std::vector<float> soaMass;
    std::vector<float> soaAccX;
    std::vector<float> soaAccY;
    std::vector<float> soaAccZ;
};
