#pragma once

#include "mna/PortNetwork.h"

namespace amp2203::stage56 {

/// B56 linear network: the phase inverter (CIRCUIT.md §1 B5) and the power stage with the output
/// transformer (§1 B6, §3) in one tree, no J5 junction and R22 wired from nG to the speaker node out
/// (DECISIONS D-041). Supplies bpX, nScr and ct are ideal sources: R18, R21 and the screen resistors are
/// resistive sources; the OT ports that end on ct are stored against ground with the supply offset.
enum Node : int {
    y, g3a, nF, k3, g3b, nG, p3a, p3b,                    // phase inverter
    nGn, nOr, nbias, gA, gB, sA, sB, pA, pA1, pB1, pB, out, sl,  // power stage
    spk1, spk2,                                           // speaker load (D-059)
    kNumNodes
};

enum Element : int {
    eJ4,       // J4 source v_oc(y) behind R_th4 (y – gnd)
    eC9,       // y – g3a
    eR17,      // g3a – nF
    eR16,      // k3 – nF
    eR19,      // nF – g3b
    eC15,      // g3b – nG
    eR20,      // nF – nG
    eR23,      // nG – gnd
    eC17VR6,   // C17 + Presence VR6 in series, stored gnd – nG (chowdsp series adaptor: b = −(b1 + b2))
    eR22,      // NFB resistor nG – out
    eR18,      // bpX behind R18 (p3a – gnd)
    eR21,      // bpX behind R21 (p3b – gnd)
    eCgk3a, eCgp3a, eCpk3a, eCgk3b, eCgp3b, eCpk3b,  // V3a / V3b inter-electrode capacitances
    eC14, eC16, eC18,    // p3a – nGn, p3b – nOr, nGn – nOr
    eR24, eR25,          // nGn – nbias, nbias – nOr
    eC19,                // nbias – gnd
    eBias,               // bias supply Thevenin source (nbias – gnd)
    eRgA, eRgB,          // grid stoppers (pair equivalents)
    eRsA, eRsB,          // nScr behind the screen resistors (pair equivalents)
    eCgkA, eCgpA, eCpkA, eCgkB, eCgpB, eCpkB,  // EL34 pair capacitances
    eRwp1,               // pA – pA1
    eLp1,                // pA1 – ct: stored pA1 – gnd, offset −v(ct)
    eLp2,                // ct – pB1: stored gnd – pB1, offset +v(ct)
    eRwp2,               // pB1 – pB
    eLs,                 // out – sl
    eRws,                // sl – gnd
    eRspkE,              // speaker Re: out – spk1 (D-059)
    eLspkE,              // voice-coil inductance: spk1 – spk2
    eRspkLe,             // its eddy-loss resistance: spk1 – spk2
    eRspkM,              // motional resonance R: spk2 – gnd
    eLspkM,              // motional resonance L: spk2 – gnd
    eCspkM,              // motional resonance C: spk2 – gnd
    kNumElements
};

/// Root: V3a / V3b grid and plate ports against k3, then (g, s, p) of each EL34 pair against ground.
enum RootPort : int { rG3a, rP3a, rG3b, rP3b, rGA, rSA, rPA, rGB, rSB, rPB, rY, kNumRootPorts };  // rY: preamp at y (M6 X1)

inline constexpr mna::PortNetwork<kNumElements, kNumNodes> kElements{{{
    {y, mna::kGround},
    {y, g3a},
    {g3a, nF},
    {k3, nF},
    {nF, g3b},
    {g3b, nG},
    {nF, nG},
    {nG, mna::kGround},
    {mna::kGround, nG},
    {nG, out},
    {p3a, mna::kGround},
    {p3b, mna::kGround},
    {g3a, k3},
    {g3a, p3a},
    {p3a, k3},
    {g3b, k3},
    {g3b, p3b},
    {p3b, k3},
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
    {g3a, k3},
    {p3a, k3},
    {g3b, k3},
    {p3b, k3},
    {gA, mna::kGround},
    {sA, mna::kGround},
    {pA, mna::kGround},
    {gB, mna::kGround},
    {sB, mna::kGround},
    {pB, mna::kGround},
    {y, mna::kGround},
}}};

}  // namespace amp2203::stage56
