#pragma once

#include <array>

namespace amp2203::detail {

/// LU factors of a K×K Jacobian as the multiport Newton leaves them (M7): U on and above the diagonal, the
/// multipliers of L below it, and the pivot row of each elimination step. Data only; solved by
/// solver::solveFactored (engine/src/solver/MultiportNewton.h).
template <int K>
struct LuFactors {
    std::array<std::array<double, K>, K> m{};
    std::array<int, K> piv{};
    std::array<std::array<double, K>, K> d{};  // the device ∂i/∂v the Jacobian was built from (device blocks only)
    bool valid = false;
};

}  // namespace amp2203::detail
