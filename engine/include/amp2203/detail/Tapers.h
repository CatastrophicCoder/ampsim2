#pragma once

#include <algorithm>
#include <cmath>

namespace amp2203::detail {

/// Potentiometer tapers exactly as in the reference netlist (SPEC §6.1).
inline double atap(double x) noexcept { return std::max((std::pow(81.0, x) - 1.0) / 80.0, 1e-4); }
inline double ltap(double x) noexcept { return std::clamp(x, 1e-4, 0.9999); }

/// One-pole smoother on a knob rotation (SPEC §6.2); `moving()` until within kEpsilon of the target.
class KnobSmoother {
public:
    static constexpr double kEpsilon = 1e-7;

    void prepare(double sampleRate, double timeConstantMs) noexcept {
        coeff_ = timeConstantMs > 0.0 ? std::exp(-1.0 / (timeConstantMs * 1e-3 * sampleRate)) : 0.0;
    }
    void setTarget(double t) noexcept { target_ = t; }
    void snap() noexcept { value_ = target_; }
    /// Advances one sample; returns true while the value is still moving.
    bool step() noexcept {
        if (value_ == target_)
            return false;
        value_ = target_ + coeff_ * (value_ - target_);
        if (std::fabs(value_ - target_) < kEpsilon)
            value_ = target_;
        return true;
    }
    double value() const noexcept { return value_; }
    double target() const noexcept { return target_; }

private:
    double coeff_ = 0.0;
    double target_ = 0.0;
    double value_ = 0.0;
};

}  // namespace amp2203::detail
