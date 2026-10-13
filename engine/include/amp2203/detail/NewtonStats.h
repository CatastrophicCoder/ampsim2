#pragma once

#include <array>
#include <cstdint>

namespace amp2203::detail {

/// Newton statistics of one multiport root (WDF_DESIGN §4): an iteration histogram and the number of
/// solves that did not converge within the iteration limit.
struct NewtonStats {
    static constexpr int kMaxTracked = 64;  // iteration counts above this land in the last bin

    std::array<uint64_t, kMaxTracked + 1> histogram{};
    uint64_t solves = 0;
    uint64_t failures = 0;
    int maxIterations = 0;

    void record(int iterations, bool converged) noexcept {
        ++histogram[static_cast<size_t>(iterations < kMaxTracked ? iterations : kMaxTracked)];
        ++solves;
        failures += converged ? 0 : 1;
        maxIterations = iterations > maxIterations ? iterations : maxIterations;
    }

    void clear() noexcept { *this = NewtonStats{}; }

    /// Median number of iterations (lower median), 0 if there were no solves.
    int medianIterations() const noexcept {
        uint64_t seen = 0;
        for (size_t k = 0; k < histogram.size(); ++k) {
            seen += histogram[k];
            if (2 * seen >= solves && solves > 0)
                return static_cast<int>(k);
        }
        return 0;
    }
};

}  // namespace amp2203::detail
