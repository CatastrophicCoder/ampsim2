#pragma once

#include "mna/PortNetwork.h"

namespace amp2203::tonestack {

/// B4 port network (CIRCUIT.md §1 B4). Same ports and node order as harness/mna.py B4_PORTS; port 0 is
/// the adapted up-port between k2b and ground (the J3 source side).
enum Node : int { k2b, tt, nD, nE, mw, tw, nbb, y, kNumNodes };
enum Port : int { pUp, pC10, pR15, pC11, pC12, pVR3a, pVR3b, pVR5, pVR4a, pVR4b, pVR2a, pVR2b, kNumPorts };

inline constexpr mna::PortNetwork<kNumPorts, kNumNodes> kNetwork{{{
    {k2b, mna::kGround},  // up-port
    {k2b, tt},            // C10
    {k2b, nD},            // R15
    {nD, nE},             // C11
    {nD, mw},             // C12
    {tt, tw},             // VR3a (Treble, top)
    {tw, nE},             // VR3b (Treble, bottom)
    {nE, nbb},            // VR5 (Bass rheostat)
    {nbb, mw},            // VR4a (Middle, top)
    {mw, mna::kGround},   // VR4b (Middle, bottom)
    {tw, y},              // VR2a (Master, top)
    {y, mna::kGround},    // VR2b (Master, bottom)
}}};

}  // namespace amp2203::tonestack
