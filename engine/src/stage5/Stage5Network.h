#pragma once

#include "mna/PortNetwork.h"

namespace amp2203::stage5 {

/// B5 linear network (CIRCUIT.md §1 B5): the long-tail-pair phase inverter V3a / V3b with the shared
/// cathode k3, the bootstrapped grid leaks, the tail, the feedback node nG with Presence, and the plate
/// loads. bpX is an ideal supply (offsets as in stage3); the NFB node `out` is a resistive source (R22).
enum Node : int { y, g3a, nF, k3, g3b, nG, p3a, p3b, kNumNodes };

enum Element : int {
    eJ4,       // J4 source v_oc(y) behind R_th4 (y – gnd)
    eC9,       // y – g3a
    eR17,      // g3a – nF
    eR16,      // k3 – nF
    eR19,      // nF – g3b
    eC15,      // g3b – nG
    eR20,      // nF – nG
    eR23,      // nG – gnd
    eC17VR6,   // C17 in series with the Presence rheostat VR6 (nG – npr – gnd), stored gnd – nG: a
               // chowdsp series adaptor reflects b = −(b1 + b2), so its port voltage is −(v_C17 + v_VR6)
    eR22,      // NFB: speaker voltage behind R22 (nG – gnd)
    eR18,      // supply bpX behind R18 (p3a – gnd)
    eR21,      // supply bpX behind R21 (p3b – gnd)
    eCgk3a, eCgp3a, eCpk3a,  // V3a inter-electrode capacitances (tubes.lib)
    eCgk3b, eCgp3b, eCpk3b,  // V3b
    kNumElements
};

/// Root ports: grid and plate port of each triode against the shared cathode k3.
enum RootPort : int { rG3a, rP3a, rG3b, rP3b, kNumRootPorts };

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
    {nG, mna::kGround},
    {p3a, mna::kGround},
    {p3b, mna::kGround},
    {g3a, k3},
    {g3a, p3a},
    {p3a, k3},
    {g3b, k3},
    {g3b, p3b},
    {p3b, k3},
}}};

inline constexpr mna::PortNetwork<kNumRootPorts, kNumNodes> kRoot{{{
    {g3a, k3},
    {p3a, k3},
    {g3b, k3},
    {p3b, k3},
}}};

}  // namespace amp2203::stage5
