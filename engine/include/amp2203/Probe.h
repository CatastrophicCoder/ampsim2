#pragma once

namespace amp2203 {

/// A named circuit quantity (SI units) exposed for the verification harness (SPEC §8). Blocks keep the
/// value in a member they update anyway, so reading probes costs nothing when nobody reads them.
struct Probe {
    const char*   name;   // netlist node name, e.g. "p1a"
    const double* value;
};

}  // namespace amp2203
