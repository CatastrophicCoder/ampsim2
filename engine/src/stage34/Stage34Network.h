#pragma once

#include "mna/PortNetwork.h"

namespace amp2203::stage34 {

/// B34 linear network: B3 (CIRCUIT.md §1 B3) and B4 (tone stack and master) in one tree under the V2a/V2b
/// 4-port root, without the J3 junction (DECISIONS D-033). bpX is an ideal supply, as in
/// engine/src/stage3/Stage3Network.h: ports that end on it are stored against ground with an offset.
enum Node : int { p1b, nB, g2a, k2a, p2a, k2b, tt, nD, nE, mw, tw, nbb, y, kNumNodes };

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
    eC10,     // k2b – tt (treble cap)
    eR15,     // k2b – nD (slope resistor)
    eC11,     // nD – nE (bass cap)
    eC12,     // nD – mw (mid cap, to the Middle wiper as drawn)
    eVR3a,    // Treble top (tt – tw)
    eVR3b,    // Treble bottom (tw – nE)
    eVR5,     // Bass rheostat (nE – nbb)
    eVR4a,    // Middle top (nbb – mw)
    eVR4b,    // Middle bottom (mw – gnd)
    eVR2a,    // Master top (tw – y)
    eVR2b,    // Master bottom (y – gnd)
    kNumElements
};

/// Root ports as in B3: V2a (g2a − k2a, p2a − k2a), V2b (p2a − k2b, bpX − k2b: stored gnd − k2b, +v(bpX)).
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
    {k2b, tt},
    {k2b, nD},
    {nD, nE},
    {nD, mw},
    {tt, tw},
    {tw, nE},
    {nE, nbb},
    {nbb, mw},
    {mw, mna::kGround},
    {tw, y},
    {y, mna::kGround},
}}};

inline constexpr mna::PortNetwork<kNumRootPorts, kNumNodes> kRoot{{{
    {g2a, k2a},
    {p2a, k2a},
    {p2a, k2b},
    {mna::kGround, k2b},
}}};

/// Sign of the supply voltage in each port's offset: (supply at node+) − (supply at node−).
inline constexpr int kElementSupplySign[kNumElements] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, +1,
                                                         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
inline constexpr int kRootSupplySign[kNumRootPorts] = {0, 0, 0, +1};

}  // namespace amp2203::stage34
