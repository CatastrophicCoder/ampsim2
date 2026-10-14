#pragma once

#include <array>
#include <cmath>
#include <numbers>

namespace amp2203::detail {

/// Linear-phase halfband FIR (Kaiser-windowed sinc) for rate changes by 2 inside the engine (M6: the power
/// amp at half the preamp rate). L = 2M + 1 taps, M odd; h[M] = 1/2, h[M ± 2k] = 0 (k ≠ 0), the odd-offset
/// taps scaled to sum to 1/2, so both polyphase branches have unity DC gain. Group delay M samples at the
/// high rate.
template <int M>
struct HalfbandFir {
    static_assert(M % 2 == 1, "M odd: the outermost taps are nonzero");
    static constexpr int kTaps = 2 * M + 1;

    explicit HalfbandFir(double beta) noexcept {
        auto i0 = [](double x) {  // modified Bessel function I0, power series
            double sum = 1.0, term = 1.0;
            for (int k = 1; k < 60; ++k) {
                term *= (x / (2.0 * k)) * (x / (2.0 * k));
                sum += term;
            }
            return sum;
        };
        double odd = 0.0;
        for (int n = 0; n < kTaps; ++n) {
            const int k = n - M;
            if (k == 0 || k % 2 == 0) {
                h[n] = 0.0;
                continue;
            }
            const double x = static_cast<double>(k) / M;
            const double w = i0(beta * std::sqrt(1.0 - x * x)) / i0(beta);
            h[n] = std::sin(0.5 * std::numbers::pi * k) / (std::numbers::pi * k) * w;
            odd += h[n];
        }
        for (int n = 0; n < kTaps; ++n)
            h[n] *= 0.5 / odd;
        h[M] = 0.5;
    }

    std::array<double, kTaps> h{};
};

/// Sliding window of the last kTaps samples, newest last, contiguous for the dot product (each sample is
/// stored twice).
template <int L>
class FirWindow {
public:
    void fill(double v) noexcept { buf_.fill(v); }
    void push(double v) noexcept {
        buf_[pos_] = v;
        buf_[pos_ + L] = v;
        pos_ = pos_ + 1 == L ? 0 : pos_ + 1;
    }
    /// Σ h[n]·x[newest − (L − 1 − n)]: h[0] meets the oldest sample.
    double dot(const std::array<double, L>& h) const noexcept {
        const double* x = buf_.data() + pos_;  // oldest
        double s0 = 0.0, s1 = 0.0;
        int n = 0;
        for (; n + 1 < L; n += 2) {
            s0 += h[n] * x[n];
            s1 += h[n + 1] * x[n + 1];
        }
        for (; n < L; ++n)
            s0 += h[n] * x[n];
        return s0 + s1;
    }

private:
    std::array<double, 2 * L> buf_{};
    int pos_ = 0;
};

}  // namespace amp2203::detail
