#pragma once

#include "mna/PortNetwork.h"

namespace amp2203::stage1234 {

/// B1234 linear network: B12, B3 and B4 (CIRCUIT.md §1) in one tree under the 8-port root of V1a, V1b,
/// V2a and V2b, without the J2 and J3 junctions (DECISIONS D-038). The element and root ports are those of
/// engine/src/stage12 and engine/src/stage34 without B34's J2 source. bpA appears only behind R4 / R7
/// (resistive sources); bpX behind R12 and as supply offsets on V2b's plate side (Cgp, Cpk, plate port).
enum Node : int { g1a, k1a, p1a, nA, vtop, w1, k1b, p1b, nB, g2a, k2a, p2a, k2b, tt, nD, nE, mw, tw, nbb, y, kNumNodes };

enum Element : int {
    // B12
    eVin, eK1a, eR4, eC3, eR5C4, eC5VR1a, eVR1b, eR6, eR7, eCgk1a, eCgp1a, eCpk1a, eCgk1b, eCgp1b, eCpk1b,
    // B3
    eC7, eR10C8, eR11, eR9, eR12, eR13, eCgk2a, eCgp2a, eCpk2a, eCgk2b, eCgp2b, eCpk2b,
    // B4
    eC10, eR15, eC11, eC12, eVR3a, eVR3b, eVR5, eVR4a, eVR4b, eVR2a, eVR2b,
    kNumElements
};

/// Grid and plate port of each triode against its cathode. V2b's plate port is bpX − k2b: stored
/// gnd − k2b, offset +v(bpX).
enum RootPort : int { rG1a, rP1a, rG1b, rP1b, rG2a, rP2a, rG2b, rP2b, kNumRootPorts };

inline constexpr mna::PortNetwork<kNumElements, kNumNodes> kElements{{{
    {g1a, mna::kGround},   // Vin behind R3
    {k1a, mna::kGround},   // R1 ‖ C1
    {p1a, mna::kGround},   // bpA behind R4
    {p1a, nA},             // C3
    {nA, vtop},            // R5 ‖ C4
    {vtop, w1},            // C5 ‖ VR1a
    {w1, mna::kGround},    // VR1b
    {k1b, mna::kGround},   // R6
    {p1b, mna::kGround},   // bpA behind R7
    {g1a, k1a},            // Cgk1a
    {g1a, p1a},            // Cgp1a
    {p1a, k1a},            // Cpk1a
    {w1, k1b},             // Cgk1b
    {w1, p1b},             // Cgp1b
    {p1b, k1b},            // Cpk1b
    {p1b, nB},             // C7
    {nB, g2a},             // R10 ‖ C8
    {g2a, mna::kGround},   // R11
    {k2a, mna::kGround},   // R9
    {p2a, mna::kGround},   // bpX behind R12
    {k2b, mna::kGround},   // R13
    {g2a, k2a},            // Cgk2a
    {g2a, p2a},            // Cgp2a
    {p2a, k2a},            // Cpk2a
    {p2a, k2b},            // Cgk2b
    {p2a, mna::kGround},   // Cgp2b (p2a – bpX)
    {mna::kGround, k2b},   // Cpk2b (bpX – k2b)
    {k2b, tt},             // C10
    {k2b, nD},             // R15
    {nD, nE},              // C11
    {nD, mw},              // C12
    {tt, tw},              // VR3a
    {tw, nE},              // VR3b
    {nE, nbb},             // VR5
    {nbb, mw},             // VR4a
    {mw, mna::kGround},    // VR4b
    {tw, y},               // VR2a
    {y, mna::kGround},     // VR2b
}}};

inline constexpr mna::PortNetwork<kNumRootPorts, kNumNodes> kRoot{{{
    {g1a, k1a},
    {p1a, k1a},
    {w1, k1b},
    {p1b, k1b},
    {g2a, k2a},
    {p2a, k2a},
    {p2a, k2b},
    {mna::kGround, k2b},
}}};

/// Sign of v(bpX) in each port's offset: (supply at node+) − (supply at node−).
inline constexpr int kElementSupplySign[kNumElements] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // B12
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, +1,          // B3
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};              // B4
inline constexpr int kRootSupplySign[kNumRootPorts] = {0, 0, 0, 0, 0, 0, 0, +1};

}  // namespace amp2203::stage1234
