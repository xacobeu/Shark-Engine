#include "AdaptiveFMM.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <functional>
#include <numbers>
#include <type_traits>

#include "util/Execution.hpp"
#include "util/SIMD.hpp"

namespace {

// Lane masks for the final partial SIMD block of a row:
// load8(&TAIL_MASK[8 - n]) yields n leading 1.0f lanes followed by (8 - n) 0.0f lanes.
alignas(32) constexpr std::array<float, 16> TAIL_MASK = {
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
};

struct P2PSoAView {
    const float* posX;
    const float* posY;
    const float* posZ;
    const float* mass;
    float*       accX;
    float*       accY;
    float*       accZ;
};

struct ErrorSampleAccum {
    float       relErrorSum = 0.0f;
    std::size_t validCount  = 0;

    [[nodiscard]] constexpr ErrorSampleAccum operator+(const ErrorSampleAccum& other) const noexcept {
        return {.relErrorSum = relErrorSum + other.relErrorSum, .validCount = validCount + other.validCount};
    }
};

// Accumulates sources [jBegin, jEnd) into target i only, so disjoint targets can run in parallel.
inline void evalP2PRow(
    const P2PSoAView& soa, std::uint32_t i, std::uint32_t jBegin, std::uint32_t jEnd, float g,
    const shark::simd::float8& eps2_8
) noexcept {
    using namespace shark::simd;

    const float8 ax8(soa.posX[i]);
    const float8 ay8(soa.posY[i]);
    const float8 az8(soa.posZ[i]);
    const float8 g8(g);

    float8 accX8(0.0f);
    float8 accY8(0.0f);
    float8 accZ8(0.0f);

    const auto block = [&](std::uint32_t j, auto maskedTag) {
        constexpr bool kMasked = decltype(maskedTag)::value;

        const float8 dx = load8(soa.posX + j) - ax8;
        const float8 dy = load8(soa.posY + j) - ay8;
        const float8 dz = load8(soa.posZ + j) - az8;
        const float8 bm = load8(soa.mass + j);

        float8 r2 = fma(dx, dx, fma(dy, dy, fma(dz, dz, eps2_8)));
        float8 laneMask(1.0f);
        if constexpr (kMasked) {
            laneMask = load8(&TAIL_MASK[8 - (jEnd - j)]);
            // Keep r2 > 0 in inactive lanes so rsqrt stays finite (0 * inf would give NaN)
            r2 = r2 + (float8(1.0f) - laneMask);
        }

        const float8 invR  = rsqrt(r2);
        float8       invR3 = invR * invR * invR;
        if constexpr (kMasked) {
            invR3 = invR3 * laneMask;
        }

        const float8 factor = (g8 * bm) * invR3;

        accX8 = fma(dx, factor, accX8);
        accY8 = fma(dy, factor, accY8);
        accZ8 = fma(dz, factor, accZ8);
    };

    std::uint32_t j = jBegin;
    for (; j + 8 <= jEnd; j += 8) {
        block(j, std::false_type{});
    }
    if (j < jEnd) {
        block(j, std::true_type{});
    }

    const auto [fx, fy, fz] = reduce_add3(accX8, accY8, accZ8);
    soa.accX[i] += fx;
    soa.accY[i] += fy;
    soa.accZ[i] += fz;
}

} // namespace

AdaptiveFMM::AdaptiveFMM(const Config& cfg) : config(cfg), nodePool(16384) {}

std::uint32_t AdaptiveFMM::allocateChildren(const Vector3& parentCenter, float parentHalfWidth, int parentDepth) {
    const float quarterWidth = parentHalfWidth * 0.5f;
    const int   childDepth   = parentDepth + 1;

    if (nodeCount + 8 > nodePool.size()) {
        nodePool.resize(std::max(nodePool.size() * 2, nodeCount + 8));
    }

    const auto firstChild = static_cast<std::uint32_t>(nodeCount);
    nodeCount += 8;

    for (int i = 0; i < 8; ++i) {
        const Vector3 childCenter = {
            parentCenter.x + (((i & 1) != 0) ? quarterWidth : -quarterWidth),
            parentCenter.y + (((i & 2) != 0) ? quarterWidth : -quarterWidth),
            parentCenter.z + (((i & 4) != 0) ? quarterWidth : -quarterWidth),
        };
        nodePool[firstChild + i].reset(childCenter, quarterWidth, childDepth);
    }
    return firstChild;
}

int AdaptiveFMM::getOctant(const Vector3& center, const Vector3& pos) noexcept {
    int oct = 0;
    if (pos.x >= center.x)
        oct |= 1;
    if (pos.y >= center.y)
        oct |= 2;
    if (pos.z >= center.z)
        oct |= 4;
    return oct;
}

void AdaptiveFMM::buildNodeRecursive(std::uint32_t nodeIdx, std::span<const fmm::Particle> particles) {
    const std::uint32_t begin = nodePool[nodeIdx].begin;
    const std::uint32_t end   = nodePool[nodeIdx].end;
    const std::uint32_t count = end - begin;

    if (count <= static_cast<std::uint32_t>(config.nCrit) || nodePool[nodeIdx].depth >= config.maxDepth) {
        nodePool[nodeIdx].isLeaf = true;
        leafNodes.push_back(nodeIdx);
        return;
    }

    const Vector3 center    = nodePool[nodeIdx].center;
    const float   halfWidth = nodePool[nodeIdx].halfWidth;
    const int     depth     = nodePool[nodeIdx].depth;

    std::array<std::uint32_t, 8> counts{};
    for (std::uint32_t i = begin; i < end; ++i) {
        const std::uint32_t pIdx = sortedIndices[i];
        const int           oct  = getOctant(center, particles[pIdx].position);
        octantScratch[i - begin] = static_cast<std::uint8_t>(oct);
        ++counts[oct];
    }

    std::array<std::uint32_t, 8> offsets{};
    std::array<std::uint32_t, 8> writeCursor{};
    std::uint32_t                running = begin;
    std::uint8_t                 mask    = 0;

    for (int oct = 0; oct < 8; ++oct) {
        offsets[oct]     = running;
        writeCursor[oct] = running - begin;
        if (counts[oct] > 0) {
            mask |= static_cast<std::uint8_t>(1U << oct);
        }
        running += counts[oct];
    }

    for (std::uint32_t i = 0; i < count; ++i) {
        const std::uint8_t oct             = octantScratch[i];
        scratchIndices[writeCursor[oct]++] = sortedIndices[begin + i];
    }
    std::copy_n(scratchIndices.data(), count, sortedIndices.data() + begin);

    const std::uint32_t firstChild    = allocateChildren(center, halfWidth, depth);
    nodePool[nodeIdx].firstChildIndex = firstChild;
    nodePool[nodeIdx].isLeaf          = false;
    nodePool[nodeIdx].activeMask      = mask;

    for (int oct = 0; oct < 8; ++oct) {
        const std::uint32_t childIdx   = firstChild + static_cast<std::uint32_t>(oct);
        const std::uint32_t childBegin = offsets[oct];
        const std::uint32_t childEnd   = childBegin + counts[oct];
        nodePool[childIdx].begin       = childBegin;
        nodePool[childIdx].end         = childEnd;

        if (counts[oct] > 0) {
            buildNodeRecursive(childIdx, particles);
        }
    }
}

void AdaptiveFMM::collectTaskRoots(std::uint32_t nodeIdx, int targetDepth) {
    const auto& node = nodePool[nodeIdx];
    if (node.count() == 0) {
        return;
    }
    if (node.isLeaf || node.depth >= targetDepth) {
        taskRoots.push_back(nodeIdx);
        return;
    }

    const std::uint32_t firstChild = node.firstChildIndex;
    const std::uint8_t  mask       = node.activeMask;
    for (int oct = 0; oct < 8; ++oct) {
        if ((mask & (1U << oct)) != 0) {
            collectTaskRoots(firstChild + static_cast<std::uint32_t>(oct), targetDepth);
        }
    }
}

void AdaptiveFMM::buildTree(std::span<const fmm::Particle> particles, Vector3 center, float halfWidth) {
    nodeCount = 0;
    leafNodes.clear();
    taskRoots.clear();

    const std::size_t count = particles.size();
    if (count == 0) {
        return;
    }

    sortedIndices.resize(count);
    scratchIndices.resize(count);
    octantScratch.resize(count);

    shark::exec::for_each_index(shark::exec::par_unseq, static_cast<std::uint32_t>(count), [this](std::uint32_t i) {
        sortedIndices[i] = i;
    });

    nodeCount = 1;
    nodePool[0].reset(center, halfWidth, 0, 0, static_cast<std::uint32_t>(count));
    buildNodeRecursive(0, particles);

    // Subtrees at depth 3 (up to 512) are the units of parallel work in execute()
    collectTaskRoots(0, 3);

    // SoA copy in tree order, padded by one SIMD block
    const std::size_t paddedSize = count + 8;
    soaPosX.resize(paddedSize);
    soaPosY.resize(paddedSize);
    soaPosZ.resize(paddedSize);
    soaMass.resize(paddedSize);
    soaAccX.resize(paddedSize);
    soaAccY.resize(paddedSize);
    soaAccZ.resize(paddedSize);

    shark::exec::for_each_index(shark::exec::par_unseq, count, [this, particles](std::size_t i) {
        const std::uint32_t origIdx = sortedIndices[i];
        const auto&         p       = particles[origIdx];
        soaPosX[i]                  = p.position.x;
        soaPosY[i]                  = p.position.y;
        soaPosZ[i]                  = p.position.z;
        soaMass[i]                  = p.mass;
    });

    for (std::size_t i = count; i < paddedSize; ++i) {
        soaPosX[i] = 0.0f;
        soaPosY[i] = 0.0f;
        soaPosZ[i] = 0.0f;
        soaMass[i] = 0.0f;
        soaAccX[i] = 0.0f;
        soaAccY[i] = 0.0f;
        soaAccZ[i] = 0.0f;
    }
}

void AdaptiveFMM::computeLeafP2M(FMMNode& node) const noexcept {
    using namespace shark::simd;

    node.M.clear();
    node.L.clear();

    const std::uint32_t begin = node.begin;
    const std::uint32_t end   = node.end;
    if (begin >= end) {
        return;
    }

    // Total mass and center of mass
    float8 sumM8(0.0f);
    float8 sumWX8(0.0f);
    float8 sumWY8(0.0f);
    float8 sumWZ8(0.0f);

    std::uint32_t i = begin;
    for (; i + 8 <= end; i += 8) {
        const float8 px = load8(&soaPosX[i]);
        const float8 py = load8(&soaPosY[i]);
        const float8 pz = load8(&soaPosZ[i]);
        const float8 pm = load8(&soaMass[i]);

        sumM8 += pm;
        sumWX8 += pm * px;
        sumWY8 += pm * py;
        sumWZ8 += pm * pz;
    }

    float totalMass = reduce_add(sumM8);
    float wx        = reduce_add(sumWX8);
    float wy        = reduce_add(sumWY8);
    float wz        = reduce_add(sumWZ8);

    for (; i < end; ++i) {
        const float pm = soaMass[i];
        totalMass += pm;
        wx += pm * soaPosX[i];
        wy += pm * soaPosY[i];
        wz += pm * soaPosZ[i];
    }

    node.M.mass = totalMass;
    if (totalMass <= 0.0f) {
        node.M.com = node.center;
        return;
    }

    const float   invMass = 1.0f / totalMass;
    const Vector3 com     = {wx * invMass, wy * invMass, wz * invMass};
    node.M.com            = com;

    // Quadrupole Q_ij = 0.5 * sum(m * dx_i * dx_j) and bounding radius about the CoM
    const float8 comX8(com.x);
    const float8 comY8(com.y);
    const float8 comZ8(com.z);
    const float8 half8(0.5f);

    float8 qxx8(0.0f);
    float8 qyy8(0.0f);
    float8 qzz8(0.0f);
    float8 qxy8(0.0f);
    float8 qyz8(0.0f);
    float8 qzx8(0.0f);
    float8 maxR2_8(0.0f);

    i = begin;
    for (; i + 8 <= end; i += 8) {
        const float8 dx = load8(&soaPosX[i]) - comX8;
        const float8 dy = load8(&soaPosY[i]) - comY8;
        const float8 dz = load8(&soaPosZ[i]) - comZ8;
        const float8 hm = load8(&soaMass[i]) * half8;

        const float8 dxx = dx * dx;
        const float8 dyy = dy * dy;
        const float8 dzz = dz * dz;
        const float8 r2  = dxx + dyy + dzz;

        maxR2_8 = max(maxR2_8, r2);

        qxx8 += hm * dxx;
        qyy8 += hm * dyy;
        qzz8 += hm * dzz;
        qxy8 += hm * (dx * dy);
        qyz8 += hm * (dy * dz);
        qzx8 += hm * (dz * dx);
    }

    float maxR2 = reduce_max(maxR2_8);
    float qxx   = reduce_add(qxx8);
    float qyy   = reduce_add(qyy8);
    float qzz   = reduce_add(qzz8);
    float qxy   = reduce_add(qxy8);
    float qyz   = reduce_add(qyz8);
    float qzx   = reduce_add(qzx8);

    for (; i < end; ++i) {
        const float dx  = soaPosX[i] - com.x;
        const float dy  = soaPosY[i] - com.y;
        const float dz  = soaPosZ[i] - com.z;
        const float hm  = soaMass[i] * 0.5f;
        const float dxx = dx * dx;
        const float dyy = dy * dy;
        const float dzz = dz * dz;

        maxR2 = std::max(maxR2, dxx + dyy + dzz);
        qxx += hm * dxx;
        qyy += hm * dyy;
        qzz += hm * dzz;
        qxy += hm * dx * dy;
        qyz += hm * dy * dz;
        qzx += hm * dz * dx;
    }

    node.M.quadrupole[0] = qxx;
    node.M.quadrupole[1] = qyy;
    node.M.quadrupole[2] = qzz;
    node.M.quadrupole[3] = qxy;
    node.M.quadrupole[4] = qyz;
    node.M.quadrupole[5] = qzx;

    node.M.rMax = std::sqrt(maxR2);
}

void AdaptiveFMM::upwardPassM2M(std::uint32_t nodeIdx) {
    auto& node = nodePool[nodeIdx];
    if (node.isLeaf) {
        return;
    }

    node.M.clear();
    node.L.clear();

    const std::uint32_t firstChild = node.firstChildIndex;
    const std::uint8_t  mask       = node.activeMask;

    float   totalMass = 0.0f;
    Vector3 weightedCom{0.0f, 0.0f, 0.0f};

    for (int oct = 0; oct < 8; ++oct) {
        if ((mask & (1U << oct)) == 0) {
            continue;
        }
        const std::uint32_t childIdx = firstChild + static_cast<std::uint32_t>(oct);
        upwardPassM2M(childIdx);

        const auto& childM = nodePool[childIdx].M;
        totalMass += childM.mass;
        weightedCom += childM.com * childM.mass;
    }

    auto& pNode  = nodePool[nodeIdx];
    pNode.M.mass = totalMass;
    if (totalMass <= 0.0f) {
        pNode.M.com = pNode.center;
        return;
    }

    const Vector3 com = weightedCom * (1.0f / totalMass);
    pNode.M.com       = com;

    float maxChildR = 0.0f;
    for (int oct = 0; oct < 8; ++oct) {
        if ((mask & (1U << oct)) == 0) {
            continue;
        }
        const auto&   childM = nodePool[firstChild + static_cast<std::uint32_t>(oct)].M;
        const Vector3 delta  = childM.com - com;
        const float   dist   = delta.length();
        maxChildR            = std::max(maxChildR, dist + childM.rMax);

        const float hm = 0.5f * childM.mass;
        const auto& qc = childM.quadrupole;
        auto&       qp = pNode.M.quadrupole;

        // Exact M2M shift around child CoM (child dipole = 0 identically)
        qp[0] += qc[0] + (hm * delta.x * delta.x);
        qp[1] += qc[1] + (hm * delta.y * delta.y);
        qp[2] += qc[2] + (hm * delta.z * delta.z);
        qp[3] += qc[3] + (hm * delta.x * delta.y);
        qp[4] += qc[4] + (hm * delta.y * delta.z);
        qp[5] += qc[5] + (hm * delta.z * delta.x);
    }

    // Children's spheres, capped by the cell's own bounding sphere
    const float boxBoundR = (pNode.center - com).length() + (pNode.halfWidth * std::numbers::sqrt3_v<float>);
    pNode.M.rMax          = std::min(maxChildR, boxBoundR);
}

void AdaptiveFMM::applyM2L(FMMNode& target, const FMMNode& source, const Vector3& R, float r2) const noexcept {
    // Fast reciprocal sqrt (hardware estimate + one Newton step) avoids the div + sqrt latency chain
    const float invR  = shark::simd::rsqrt(shark::simd::float4(r2 + eps2))[0];
    const float invR2 = invR * invR;

    // Radial kernel derivatives: d_k = (2k-1)!! * G / (r^2 + eps^2)^{(2k+1)/2}
    const float d1 = currentG * invR * invR2;
    const float d2 = 3.0f * d1 * invR2;
    const float d3 = 5.0f * d2 * invR2;
    const float d4 = 7.0f * d3 * invR2;

    const float rx  = R.x;
    const float ry  = R.y;
    const float rz  = R.z;
    const float rxx = rx * rx;
    const float ryy = ry * ry;
    const float rzz = rz * rz;
    const float rxy = rx * ry;
    const float ryz = ry * rz;
    const float rzx = rz * rx;

    // Order-1 tensor T1_i = d1 * R_i (odd parity)
    const float t1x = d1 * rx;
    const float t1y = d1 * ry;
    const float t1z = d1 * rz;

    // Order-2 symmetric tensor T2_ij = d1 * delta_ij - d2 * R_i * R_j (even parity)
    const float t2xx = d1 - (d2 * rxx);
    const float t2yy = d1 - (d2 * ryy);
    const float t2zz = d1 - (d2 * rzz);
    const float t2xy = -d2 * rxy;
    const float t2yz = -d2 * ryz;
    const float t2zx = -d2 * rzx;

    // Order-3 symmetric tensor T3_ijk (10 components, odd parity)
    const std::array<float, 10> t3 = {
        (-3.0f * d2 * rx) + (d3 * rxx * rx), // 0: xxx
        (-3.0f * d2 * ry) + (d3 * ryy * ry), // 1: yyy
        (-3.0f * d2 * rz) + (d3 * rzz * rz), // 2: zzz
        (-d2 * ry) + (d3 * rxx * ry),        // 3: xxy
        (-d2 * rz) + (d3 * rxx * rz),        // 4: xxz
        (-d2 * rx) + (d3 * ryy * rx),        // 5: yyx
        (-d2 * rz) + (d3 * ryy * rz),        // 6: yyz
        (-d2 * rx) + (d3 * rzz * rx),        // 7: zzx
        (-d2 * ry) + (d3 * rzz * ry),        // 8: zzy
        d3 * rx * ry * rz,                   // 9: xyz
    };

    const float mB  = source.M.mass;
    const auto& qB  = source.M.quadrupole;
    const float trB = qB[0] + qB[1] + qB[2];

    // qVecB = Q_B * R
    const float qBx = (qB[0] * rx) + (qB[3] * ry) + (qB[5] * rz);
    const float qBy = (qB[3] * rx) + (qB[1] * ry) + (qB[4] * rz);
    const float qBz = (qB[5] * rx) + (qB[4] * ry) + (qB[2] * rz);
    const float qRR = (qBx * rx) + (qBy * ry) + (qBz * rz);

    // Monopole + Quadrupole -> F_target
    const float radialB = (d3 * qRR) - (d2 * trB);
    target.L.F.x += (mB * t1x) - (2.0f * d2 * qBx) + (radialB * rx);
    target.L.F.y += (mB * t1y) - (2.0f * d2 * qBy) + (radialB * ry);
    target.L.F.z += (mB * t1z) - (2.0f * d2 * qBz) + (radialB * rz);

    // Monopole + Quadrupole -> J_target (J_target -= mB * T2 + T4 : Q_B)
    const float cDiagB = radialB;
    const float cRR_B  = (d3 * trB) - (d4 * qRR);

    target.L.J[0] -= (mB * t2xx) + cDiagB - (2.0f * d2 * qB[0]) + (4.0f * d3 * rx * qBx) + (cRR_B * rxx);
    target.L.J[1] -= (mB * t2yy) + cDiagB - (2.0f * d2 * qB[1]) + (4.0f * d3 * ry * qBy) + (cRR_B * ryy);
    target.L.J[2] -= (mB * t2zz) + cDiagB - (2.0f * d2 * qB[2]) + (4.0f * d3 * rz * qBz) + (cRR_B * rzz);
    target.L.J[3] -= (mB * t2xy) - (2.0f * d2 * qB[3]) + (2.0f * d3 * ((rx * qBy) + (ry * qBx))) + (cRR_B * rxy);
    target.L.J[4] -= (mB * t2yz) - (2.0f * d2 * qB[4]) + (2.0f * d3 * ((ry * qBz) + (rz * qBy))) + (cRR_B * ryz);
    target.L.J[5] -= (mB * t2zx) - (2.0f * d2 * qB[5]) + (2.0f * d3 * ((rz * qBx) + (rx * qBz))) + (cRR_B * rzx);

    // Monopole -> H_target
    for (std::size_t k = 0; k < 10; ++k) {
        target.L.H[k] += mB * t3[k];
    }
}

void AdaptiveFMM::directP2P(const FMMNode& target, const FMMNode& source) noexcept {
    const P2PSoAView soa{
        soaPosX.data(), soaPosY.data(), soaPosZ.data(), soaMass.data(), soaAccX.data(), soaAccY.data(), soaAccZ.data(),
    };
    const shark::simd::float8 eps2_8(eps2);

    for (std::uint32_t i = target.begin; i < target.end; ++i) {
        evalP2PRow(soa, i, source.begin, source.end, currentG, eps2_8);
    }
}

void AdaptiveFMM::interactTarget(std::uint32_t targetIdx, std::uint32_t sourceIdx) {
    auto&       nodeA = nodePool[targetIdx];
    const auto& nodeB = nodePool[sourceIdx];

    if (nodeA.M.mass <= 0.0f || nodeB.M.mass <= 0.0f) {
        return;
    }

    // Self-interaction: recurse into the subtree's own children
    if (targetIdx == sourceIdx) {
        if (nodeA.count() <= 1) {
            return;
        }
        if (nodeA.isLeaf) {
            directP2P(nodeA, nodeA);
            return;
        }
        const std::uint32_t firstA = nodeA.firstChildIndex;
        const std::uint8_t  maskA  = nodeA.activeMask;
        for (int i = 0; i < 8; ++i) {
            if ((maskA & (1U << i)) != 0) {
                interactTarget(firstA + static_cast<std::uint32_t>(i), sourceIdx);
            }
        }
        return;
    }

    // Source is an ancestor of target: split the source
    if (nodeB.begin <= nodeA.begin && nodeA.end <= nodeB.end) {
        const std::uint32_t firstB = nodeB.firstChildIndex;
        const std::uint8_t  maskB  = nodeB.activeMask;
        for (int j = 0; j < 8; ++j) {
            if ((maskB & (1U << j)) != 0) {
                interactTarget(targetIdx, firstB + static_cast<std::uint32_t>(j));
            }
        }
        return;
    }

    const Vector3 R      = nodeB.M.com - nodeA.M.com;
    const float   distSq = R.lengthSq();
    const float   sumR   = nodeA.M.rMax + nodeB.M.rMax;

    // theta capped at 0.9: above that the expansions don't converge
    const float effTheta = std::min(config.theta, 0.90f);
    if (distSq > 0.0f && (sumR * sumR) < (effTheta * effTheta * distSq)) {
        applyM2L(nodeA, nodeB, R, distSq);
        return;
    }

    if (nodeA.isLeaf && nodeB.isLeaf) {
        directP2P(nodeA, nodeB);
        return;
    }

    // Split the node with the larger bounding radius
    if (nodeB.isLeaf || (!nodeA.isLeaf && nodeA.M.rMax >= nodeB.M.rMax)) {
        const std::uint32_t firstA = nodeA.firstChildIndex;
        const std::uint8_t  maskA  = nodeA.activeMask;
        for (int i = 0; i < 8; ++i) {
            if ((maskA & (1U << i)) != 0) {
                interactTarget(firstA + static_cast<std::uint32_t>(i), sourceIdx);
            }
        }
    } else {
        const std::uint32_t firstB = nodeB.firstChildIndex;
        const std::uint8_t  maskB  = nodeB.activeMask;
        for (int j = 0; j < 8; ++j) {
            if ((maskB & (1U << j)) != 0) {
                interactTarget(targetIdx, firstB + static_cast<std::uint32_t>(j));
            }
        }
    }
}

void AdaptiveFMM::evaluateLeafL2P(const FMMNode& node) noexcept {
    using namespace shark::simd;

    const std::uint32_t begin = node.begin;
    const std::uint32_t end   = node.end;
    if (begin >= end) {
        return;
    }

    const Vector3 com = node.M.com;
    const auto&   L   = node.L;

    const float8 comX(com.x);
    const float8 comY(com.y);
    const float8 comZ(com.z);
    const float8 half8(0.5f);

    const float8 fx(L.F.x);
    const float8 fy(L.F.y);
    const float8 fz(L.F.z);

    const float8 j0(L.J[0]);
    const float8 j1(L.J[1]);
    const float8 j2(L.J[2]);
    const float8 j3(L.J[3]);
    const float8 j4(L.J[4]);
    const float8 j5(L.J[5]);

    const float8 h0(L.H[0]);
    const float8 h1(L.H[1]);
    const float8 h2(L.H[2]);
    const float8 h3(L.H[3]);
    const float8 h4(L.H[4]);
    const float8 h5(L.H[5]);
    const float8 h6(L.H[6]);
    const float8 h7(L.H[7]);
    const float8 h8(L.H[8]);
    const float8 h9(L.H[9]);

    std::uint32_t i = begin;
    for (; i + 8 <= end; i += 8) {
        const float8 dx = load8(&soaPosX[i]) - comX;
        const float8 dy = load8(&soaPosY[i]) - comY;
        const float8 dz = load8(&soaPosZ[i]) - comZ;

        const float8 dxx = (dx * dx) * half8;
        const float8 dyy = (dy * dy) * half8;
        const float8 dzz = (dz * dz) * half8;
        const float8 dxy = dx * dy;
        const float8 dyz = dy * dz;
        const float8 dzx = dz * dx;

        const float8 ax = fx + ((j0 * dx) + (j3 * dy) + (j5 * dz))
                          + ((h0 * dxx) + (h5 * dyy) + (h7 * dzz) + (h3 * dxy) + (h9 * dyz) + (h4 * dzx));
        const float8 ay = fy + ((j3 * dx) + (j1 * dy) + (j4 * dz))
                          + ((h3 * dxx) + (h1 * dyy) + (h8 * dzz) + (h5 * dxy) + (h6 * dyz) + (h9 * dzx));
        const float8 az = fz + ((j5 * dx) + (j4 * dy) + (j2 * dz))
                          + ((h4 * dxx) + (h6 * dyy) + (h2 * dzz) + (h9 * dxy) + (h8 * dyz) + (h7 * dzx));

        store8(&soaAccX[i], load8(&soaAccX[i]) + ax);
        store8(&soaAccY[i], load8(&soaAccY[i]) + ay);
        store8(&soaAccZ[i], load8(&soaAccZ[i]) + az);
    }

    for (; i < end; ++i) {
        const float dx  = soaPosX[i] - com.x;
        const float dy  = soaPosY[i] - com.y;
        const float dz  = soaPosZ[i] - com.z;
        const float dxx = 0.5f * dx * dx;
        const float dyy = 0.5f * dy * dy;
        const float dzz = 0.5f * dz * dz;
        const float dxy = dx * dy;
        const float dyz = dy * dz;
        const float dzx = dz * dx;

        soaAccX[i] += L.F.x + ((L.J[0] * dx) + (L.J[3] * dy) + (L.J[5] * dz))
                      + ((L.H[0] * dxx) + (L.H[5] * dyy) + (L.H[7] * dzz) + (L.H[3] * dxy) + (L.H[9] * dyz)
                         + (L.H[4] * dzx));
        soaAccY[i] += L.F.y + ((L.J[3] * dx) + (L.J[1] * dy) + (L.J[4] * dz))
                      + ((L.H[3] * dxx) + (L.H[1] * dyy) + (L.H[8] * dzz) + (L.H[5] * dxy) + (L.H[6] * dyz)
                         + (L.H[9] * dzx));
        soaAccZ[i] += L.F.z + ((L.J[5] * dx) + (L.J[4] * dy) + (L.J[2] * dz))
                      + ((L.H[4] * dxx) + (L.H[6] * dyy) + (L.H[2] * dzz) + (L.H[9] * dxy) + (L.H[8] * dyz)
                         + (L.H[7] * dzx));
    }
}

void AdaptiveFMM::downwardPass(std::uint32_t nodeIdx) {
    const auto& node = nodePool[nodeIdx];
    if (node.M.mass <= 0.0f || node.count() == 0) {
        return;
    }

    if (node.isLeaf) {
        evaluateLeafL2P(node);
        return;
    }

    const std::uint32_t firstChild = node.firstChildIndex;
    const std::uint8_t  mask       = node.activeMask;
    const Vector3       pCom       = node.M.com;
    const auto          pL         = node.L;

    for (int oct = 0; oct < 8; ++oct) {
        if ((mask & (1U << oct)) == 0) {
            continue;
        }
        const std::uint32_t childIdx = firstChild + static_cast<std::uint32_t>(oct);
        auto&               child    = nodePool[childIdx];

        // L2L shift from parent CoM to child CoM: delta = z_c - z_p
        const Vector3 d   = child.M.com - pCom;
        const float   dx  = d.x;
        const float   dy  = d.y;
        const float   dz  = d.z;
        const float   dxx = 0.5f * dx * dx;
        const float   dyy = 0.5f * dy * dy;
        const float   dzz = 0.5f * dz * dz;
        const float   dxy = dx * dy;
        const float   dyz = dy * dz;
        const float   dzx = dz * dx;

        // F_c += F_p + J_p * d + 0.5 * H_p : (d x d)
        child.L.F.x += pL.F.x + ((pL.J[0] * dx) + (pL.J[3] * dy) + (pL.J[5] * dz))
                       + ((pL.H[0] * dxx) + (pL.H[5] * dyy) + (pL.H[7] * dzz) + (pL.H[3] * dxy) + (pL.H[9] * dyz)
                          + (pL.H[4] * dzx));
        child.L.F.y += pL.F.y + ((pL.J[3] * dx) + (pL.J[1] * dy) + (pL.J[4] * dz))
                       + ((pL.H[3] * dxx) + (pL.H[1] * dyy) + (pL.H[8] * dzz) + (pL.H[5] * dxy) + (pL.H[6] * dyz)
                          + (pL.H[9] * dzx));
        child.L.F.z += pL.F.z + ((pL.J[5] * dx) + (pL.J[4] * dy) + (pL.J[2] * dz))
                       + ((pL.H[4] * dxx) + (pL.H[6] * dyy) + (pL.H[2] * dzz) + (pL.H[9] * dxy) + (pL.H[8] * dyz)
                          + (pL.H[7] * dzx));

        // J_c += J_p + H_p * d
        child.L.J[0] += pL.J[0] + (pL.H[0] * dx) + (pL.H[3] * dy) + (pL.H[4] * dz);
        child.L.J[1] += pL.J[1] + (pL.H[5] * dx) + (pL.H[1] * dy) + (pL.H[6] * dz);
        child.L.J[2] += pL.J[2] + (pL.H[7] * dx) + (pL.H[8] * dy) + (pL.H[2] * dz);
        child.L.J[3] += pL.J[3] + (pL.H[3] * dx) + (pL.H[5] * dy) + (pL.H[9] * dz);
        child.L.J[4] += pL.J[4] + (pL.H[9] * dx) + (pL.H[6] * dy) + (pL.H[8] * dz);
        child.L.J[5] += pL.J[5] + (pL.H[4] * dx) + (pL.H[9] * dy) + (pL.H[7] * dz);

        // H_c += H_p
        for (std::size_t k = 0; k < 10; ++k) {
            child.L.H[k] += pL.H[k];
        }

        downwardPass(childIdx);
    }
}

void AdaptiveFMM::execute(std::span<fmm::Particle> particles, float G) {
    if (nodeCount == 0 || particles.empty()) {
        return;
    }
    currentG = G;
    eps2     = std::max(config.softening * config.softening, 1e-12f);

    const std::size_t count = particles.size();
    shark::exec::fill_n(shark::exec::par_unseq, soaAccX.data(), count, 0.0f);
    shark::exec::fill_n(shark::exec::par_unseq, soaAccY.data(), count, 0.0f);
    shark::exec::fill_n(shark::exec::par_unseq, soaAccZ.data(), count, 0.0f);

    // Upward pass: P2M on leaves in parallel, then M2M up the tree
    shark::exec::for_each(shark::exec::par, leafNodes, [this](std::uint32_t leafIdx) {
        computeLeafP2M(nodePool[leafIdx]);
    });
    upwardPassM2M(0);

    // Each task root only writes to its own subtree, so interaction and downward passes run in parallel
    shark::exec::for_each(shark::exec::par, taskRoots, [this](std::uint32_t subRootIdx) {
        interactTarget(subRootIdx, 0);
        downwardPass(subRootIdx);
    });

    shark::exec::for_each_index(shark::exec::par_unseq, count, [this, particles](std::size_t i) {
        const std::uint32_t origIdx     = sortedIndices[i];
        particles[origIdx].acceleration = {soaAccX[i], soaAccY[i], soaAccZ[i]};
    });
}

float AdaptiveFMM::computeObservedError(
    std::span<const fmm::Particle> particles, float G, std::size_t sampleCount
) const {
    using namespace shark::simd;

    const std::size_t totalCount = particles.size();
    if (totalCount == 0 || sampleCount == 0 || soaPosX.size() < totalCount) {
        return 0.0f;
    }

    const std::size_t samples    = std::min(sampleCount, totalCount);
    const float       eps2Scalar = config.softening * config.softening;
    const float8      g8(G);
    const float8      eps2_8(eps2Scalar);

    const ErrorSampleAccum accum = shark::exec::transform_reduce_index(
        shark::exec::par, samples, ErrorSampleAccum{}, std::plus<>{}, [&](std::size_t s) -> ErrorSampleAccum {
            const std::size_t idxA = (s * totalCount) / samples;
            const auto&       pA   = particles[idxA];

            const float8 ax8(pA.position.x);
            const float8 ay8(pA.position.y);
            const float8 az8(pA.position.z);

            float8 accX8(0.0f);
            float8 accY8(0.0f);
            float8 accZ8(0.0f);

            std::size_t j = 0;
            for (; j + 8 <= totalCount; j += 8) {
                const float8 dx = load8(&soaPosX[j]) - ax8;
                const float8 dy = load8(&soaPosY[j]) - ay8;
                const float8 dz = load8(&soaPosZ[j]) - az8;
                const float8 bm = load8(&soaMass[j]);

                const float8 r2     = (dx * dx) + (dy * dy) + (dz * dz) + eps2_8;
                const float8 invR   = rsqrt(r2);
                const float8 factor = (g8 * bm) * (invR * invR * invR);

                accX8 += dx * factor;
                accY8 += dy * factor;
                accZ8 += dz * factor;
            }

            Vector3 directAcc = {reduce_add(accX8), reduce_add(accY8), reduce_add(accZ8)};

            for (; j < totalCount; ++j) {
                const float dx     = soaPosX[j] - pA.position.x;
                const float dy     = soaPosY[j] - pA.position.y;
                const float dz     = soaPosZ[j] - pA.position.z;
                const float r2Val  = (dx * dx) + (dy * dy) + (dz * dz) + eps2Scalar;
                const float invR   = 1.0f / std::sqrt(r2Val);
                const float factor = (G * soaMass[j]) * (invR * invR * invR);

                directAcc.x += dx * factor;
                directAcc.y += dy * factor;
                directAcc.z += dz * factor;
            }

            const float directMag = directAcc.length();
            if (directMag > 1e-6f) {
                const Vector3 diffAcc = pA.acceleration - directAcc;
                const float   diffMag = diffAcc.length();
                return {.relErrorSum = diffMag / directMag, .validCount = 1};
            }
            return {};
        }
    );

    return accum.validCount > 0 ? (accum.relErrorSum / static_cast<float>(accum.validCount)) * 100.0f : 0.0f;
}
