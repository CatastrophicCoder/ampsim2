#pragma once

#include "mna/PortNetwork.h"

namespace amp2203::stage3 {

/// B3 linear network (CIRCUIT.md §1 B3) for the joint 4-port root of V2a and V2b. The supply node bpX is an
/// ideal source, not a network node: ports that end on it are stored against ground, and the block adds
/// the exact supply offset (engine/src/stage/RootNetwork.h).
enum Node : int { p1b, nB, g2a, k2a, p2a, k2b, kNumNodes };

/// Element ports (node+, node−). The resistive sources have their + terminal at node+.
enum Element : int {
    eJ2,      // J2 source v_oc(p1b) behind R_th2 (p1b – gnd)
    eC7,      // p1b – nB
    eR10C8,   // R10 ‖ C8 (nB – g2a)
    eR11,     // g2a – gnd
    eR9,      // k2a – gnd
    eR12,     // supply bpX behind R12 (p2a – gnd)
    eR13,     // k2b – gnd
    eCgk2a, eCgp2a, eCpk2a,  // V2a inter-electrode capacitances (tubes.lib)
    eCgk2b,                  // V2b grid–cathode (p2a – k2b)
    eCgp2b,                  // V2b grid–plate (p2a – bpX): stored p2a – gnd, offset −v(bpX)
    eCpk2b,                  // V2b plate–cathode (bpX – k2b): stored gnd – k2b, offset +v(bpX)
    kNumElements
};

/// Root ports: grid and plate port of each triode against its cathode. V2b's plate port is bpX – k2b:
/// stored gnd – k2b, offset +v(bpX).
enum RootPort : int { rG2a, rP2a, rG2b, rP2b, kNumRootPorts };

inline constexpr mna::PortNetwork<kNumElements, kNumNodes> kElements{{{
    {p1b, mna::kGround},
    {p1b, nB},
    {nB, g2a},
    {g2a, mna::kGround},
    {k2a, mna::kGround},
    {p2a, mna::kGround},
    {k2b, mna::kGround},
    {g2a, k2a},
    {g2a, p2a},
    {p2a, k2a},
    {p2a, k2b},
    {p2a, mna::kGround},
    {mna::kGround, k2b},
}}};

inline constexpr mna::PortNetwork<kNumRootPorts, kNumNodes> kRoot{{{
    {g2a, k2a},
    {p2a, k2a},
    {p2a, k2b},
    {mna::kGround, k2b},
}}};

/// Sign of the supply voltage in each port's offset: (supply at node+) − (supply at node−).
inline constexpr int kElementSupplySign[kNumElements] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, +1};
inline constexpr int kRootSupplySign[kNumRootPorts] = {0, 0, 0, +1};

}  // namespace amp2203::stage3
