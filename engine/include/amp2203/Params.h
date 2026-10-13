#pragma once

namespace amp2203 {

/// Knob rotations, 0.0 .. 1.0 (0 = fully counter-clockwise). SPEC §3.2.
struct Params {
    double preampVolume = 0.6;
    double treble       = 0.5;
    double middle       = 0.5;
    double bass         = 0.5;
    double master       = 0.3;
    double presence     = 0.5;
};

struct EngineConfig {
    double sampleRate = 768000.0;           // the rate process() is called at (already oversampled; D-035)
    int    maxBlockSize = 512;
    double inputVoltsPerFullScale  = 4.361; // 0 dBFS input = +12 dBu at the jack (DECISIONS D-010)
    double outputVoltsPerFullScale = 60.0;  // 60 V peak across 16 Ω = 0 dBFS output
    double paramSmoothingMs        = 20.0;
};

}  // namespace amp2203
