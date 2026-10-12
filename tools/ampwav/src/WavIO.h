#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ampwav {

struct MonoWav {
    double sampleRate = 0.0;
    std::vector<double> samples;
};

/// Reads a mono WAV. 64-bit float files are read bit-exact; any other format is converted through
/// 32-bit float (exact for 16/24-bit PCM and 32-bit float). Returns false and sets `error` on failure.
bool readMonoWav(const std::string& path, MonoWav& out, std::string& error);

/// Writes a mono 64-bit IEEE float WAV.
bool writeMonoWavF64(const std::string& path, double sampleRate, const std::vector<double>& samples,
                     std::string& error);

}  // namespace ampwav
