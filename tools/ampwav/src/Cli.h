#pragma once

#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

#include "amp2203/Amp2203.h"

namespace ampwav {

enum class Units {
    Volts,      // WAV samples are volts at the block input/output (default; matches SPICE)
    FullScale,  // WAV samples are full-scale normalised, as the plugin sees them (full amp only)
};

struct NodeDump {
    std::string node;
    std::string path;
};

/// Linear ramp of one knob's target rotation (the engine smooths it as for any setParams call).
struct KnobRamp {
    std::string knob;
    double start = 0.0;     // s
    double duration = 0.0;  // s
    double from = 0.0;
    double to = 0.0;
};

struct Options {
    std::string block = "full";
    std::string inPath;
    std::string outPath;
    std::optional<double> rate;  // must equal the input file's rate when given (no resampling, D-007)
    amp2203::Params params;
    std::vector<NodeDump> dumps;
    std::vector<KnobRamp> ramps;  // --knob-ramp; params are then set every sample
    std::vector<NodeDump> supplies;  // --supply node=file.wav: recorded supply voltages, set every sample
    std::string statsPath;       // optional JSON with timing and Newton statistics
    std::optional<double> halfStepTrigger;  // --half-step-trigger V (power amp, D-043)
    std::string nfb;             // --nfb closed | open (full amp only; empty = closed)
    int powerRateDivisor = 1;    // --power-rate-divisor 1 | 2 (full amp, M6)
    std::optional<double> newtonTol;  // --newton-tol V (full amp, M6 study)
    Units units = Units::Volts;
    int blockSize = 512;
    bool listNodes = false;
    bool help = false;
};

/// Parses argv[1..]. Returns std::nullopt and sets `error` on invalid input.
std::optional<Options> parseArgs(const std::vector<std::string>& args, std::string& error);

void printUsage(std::ostream& os);

/// Runs the whole CLI job. Returns the process exit code (0 ok, 1 runtime error).
int run(const Options& opts, std::ostream& log);

}  // namespace ampwav
