#pragma once

#include "mna/PortNetwork.h"

namespace amp2203::stage6 {

/// B6 linear network (CIRCUIT.md §1 B6, §3): J5 sources, coupling to the EL34 grids, bias network, the two
/// pair equivalents' grid stoppers, screen resistors and inter-electrode capacitances, the output
/// transformer and the 16 Ω load. The OT centre tap ct and the screen supply nScr are ideal supplies:
/// ports that end on ct are stored against ground with the supply offset (stage/RootNetwork.h); the
/// screen resistors are resistive sources.
enum Node : int { p3a, p3b, nGn, nOr, nbias, gA, gB, sA, sB, pA, pA1, pB1, pB, out, sl, spk1, spk2, kNumNodes };

enum Element : int {
    eJ5a, eJ5b,          // J5 sources v_oc(p3a), v_oc(p3b) behind R_thA / R_thB (p3x – gnd)
    eC14, eC16, eC18,    // p3a – nGn, p3b – nOr, nGn – nOr
    eR24, eR25,          // nGn – nbias, nbias – nOr
    eC19,                // nbias – gnd
    eBias,               // bias supply Vc20 / R27 / R26 + RRV1 as its Thevenin source (nbias – gnd)
    eRgA, eRgB,          // grid stoppers, pair equivalents R31 ‖ R33, R32 ‖ R34 (nGn – gA, nOr – gB)
    eRsA, eRsB,          // screen resistors, pair equivalents R35 ‖ R37, R36 ‖ R38: nScr behind them (sX – gnd)
    eCgkA, eCgpA, eCpkA,  // pair A inter-electrode capacitances (2 × tubes.lib EL34)
    eCgkB, eCgpB, eCpkB,  // pair B
    eRwp1,               // pA – pA1
    eLp1,                // OT half-primary pA1 – ct: stored pA1 – gnd, offset −v(ct)
    eLp2,                // OT half-primary ct – pB1: stored gnd – pB1, offset +v(ct)
    eRwp2,               // pB1 – pB
    eLs,                 // OT secondary out – sl
    eRws,                // sl – gnd
    eRspkE,              // speaker Re: out – spk1 (D-059)
    eLspkE,              // voice-coil inductance: spk1 – spk2
    eRspkLe,             // its eddy-loss resistance: spk1 – spk2
    eRspkM,              // motional resonance R: spk2 – gnd
    eLspkM,              // motional resonance L: spk2 – gnd
    eCspkM,              // motional resonance C: spk2 – gnd
    kNumElements
};

enum RootPort : int { rGA, rSA, rPA, rGB, rSB, rPB, kNumRootPorts };

inline constexpr mna::PortNetwork<kNumElements, kNumNodes> kElements{{{
    {p3a, mna::kGround},
    {p3b, mna::kGround},
    {p3a, nGn},
    {p3b, nOr},
    {nGn, nOr},
    {nGn, nbias},
    {nbias, nOr},
    {nbias, mna::kGround},
    {nbias, mna::kGround},
    {nGn, gA},
    {nOr, gB},
    {sA, mna::kGround},
    {sB, mna::kGround},
    {gA, mna::kGround},
    {gA, pA},
    {pA, mna::kGround},
    {gB, mna::kGround},
    {gB, pB},
    {pB, mna::kGround},
    {pA, pA1},
    {pA1, mna::kGround},
    {mna::kGround, pB1},
    {pB1, pB},
    {out, sl},
    {sl, mna::kGround},
    {out, spk1},
    {spk1, spk2},
    {spk1, spk2},
    {spk2, mna::kGround},
    {spk2, mna::kGround},
    {spk2, mna::kGround},
}}};

inline constexpr mna::PortNetwork<kNumRootPorts, kNumNodes> kRoot{{{
    {gA, mna::kGround},
    {sA, mna::kGround},
    {pA, mna::kGround},
    {gB, mna::kGround},
    {sB, mna::kGround},
    {pB, mna::kGround},
}}};

/// Sign of v(ct) in each element port's offset: (supply at node+) − (supply at node−).
inline constexpr int kElementCtSign[kNumElements] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0,
                                                     0, 0, 0, 0, -1, +1, 0, 0, 0, 0, 0, 0, 0, 0, 0};

}  // namespace amp2203::stage6
