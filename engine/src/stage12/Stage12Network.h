#pragma once

#include "mna/PortNetwork.h"

namespace amp2203::stage12 {

/// B12 linear network (CIRCUIT.md §1 B12) for the joint 4-port root (DECISIONS D-017, D-028). R2 sits
/// across the ideal input source and changes no node voltage, so it is not part of the network.
enum Node : int { g1a, k1a, p1a, nA, vtop, w1, k1b, p1b, kNumNodes };

/// Element ports (node+, node−). The resistive sources have their + terminal at node+.
enum Element : int {
    eVin,      // input source behind R3 (g1a – gnd)
    eK1a,      // R1 ‖ C1 (k1a – gnd)
    eR4,       // supply bpA behind R4 (p1a – gnd)
    eC3,       // p1a – nA
    eR5C4,     // R5 ‖ C4 (nA – vtop)
    eC5VR1a,   // C5 ‖ VR1a, Preamp Volume top (vtop – w1)
    eVR1b,     // Preamp Volume bottom (w1 – gnd)
    eR6,       // k1b – gnd
    eR7,       // supply bpA behind R7 (p1b – gnd)
    eCgk1a, eCgp1a, eCpk1a,  // V1a inter-electrode capacitances (tubes.lib)
    eCgk1b, eCgp1b, eCpk1b,  // V1b
    kNumElements
};

/// Root ports: grid and plate port of each triode, both against its cathode (WDF_DESIGN §4 port map).
enum RootPort : int { rG1a, rP1a, rG1b, rP1b, kNumRootPorts };

inline constexpr mna::PortNetwork<kNumElements, kNumNodes> kElements{{{
    {g1a, mna::kGround},
    {k1a, mna::kGround},
    {p1a, mna::kGround},
    {p1a, nA},
    {nA, vtop},
    {vtop, w1},
    {w1, mna::kGround},
    {k1b, mna::kGround},
    {p1b, mna::kGround},
    {g1a, k1a},
    {g1a, p1a},
    {p1a, k1a},
    {w1, k1b},
    {w1, p1b},
    {p1b, k1b},
}}};

inline constexpr mna::PortNetwork<kNumRootPorts, kNumNodes> kRoot{{{
    {g1a, k1a},
    {p1a, k1a},
    {w1, k1b},
    {p1b, k1b},
}}};

}  // namespace amp2203::stage12
