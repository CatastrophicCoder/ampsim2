#pragma once

#include <chowdsp_wdf/chowdsp_wdf.h>

namespace amp2203::detail {

/// Bilinear WDF capacitor (WDF_DESIGN §2: R = T / (2C), b[n] = a[n−1]), equations identical to
/// chowdsp::wdft::CapacitorT, plus a settable DC state so that `reset()` can start from the DC operating
/// point (SPEC §3.6, option 1): at DC the current is zero, so a = b = v and the state z = v.
/// tests/test_tonestack.cpp checks it sample for sample against the library's CapacitorT.
class DcCapacitor final : public chowdsp::wdft::BaseWDF {
public:
    explicit DcCapacitor(double capacitance, double fs = 48000.0) : C_(capacitance), fs_(fs) { calcImpedance(); }

    void prepare(double sampleRate) {
        fs_ = sampleRate;
        propagateImpedanceChange();
        z_ = 0.0;
    }

    /// Sets the state for a capacitor voltage v with zero current (DC operating point).
    void setDcVoltage(double v) noexcept {
        z_ = v;
        wdf.a = wdf.b = v;
    }

    void calcImpedance() override {
        wdf.R = 1.0 / (2.0 * C_ * fs_);
        wdf.G = 1.0 / wdf.R;
    }

    inline void incident(double x) noexcept {
        wdf.a = x;
        z_ = x;
    }

    inline double reflected() noexcept {
        wdf.b = z_;
        return wdf.b;
    }

    chowdsp::wdft::WDFMembers<double> wdf;

private:
    double C_;
    double fs_;
    double z_ = 0.0;
};

}  // namespace amp2203::detail
