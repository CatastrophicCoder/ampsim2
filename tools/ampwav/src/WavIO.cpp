#include "WavIO.h"

#include <dr_wav.h>

namespace ampwav {

bool readMonoWav(const std::string& path, MonoWav& out, std::string& error) {
    drwav wav;
    if (!drwav_init_file(&wav, path.c_str(), nullptr)) {
        error = "cannot open WAV file '" + path + "'";
        return false;
    }
    if (wav.channels != 1) {
        error = "'" + path + "' has " + std::to_string(wav.channels) + " channels; ampwav needs mono";
        drwav_uninit(&wav);
        return false;
    }

    const auto frames = static_cast<size_t>(wav.totalPCMFrameCount);
    out.sampleRate = static_cast<double>(wav.sampleRate);
    out.samples.assign(frames, 0.0);

    drwav_uint64 read = 0;
    if (wav.translatedFormatTag == DR_WAVE_FORMAT_IEEE_FLOAT && wav.bitsPerSample == 64) {
        read = drwav_read_pcm_frames_le(&wav, frames, out.samples.data());
    } else {
        std::vector<float> tmp(frames);
        read = drwav_read_pcm_frames_f32(&wav, frames, tmp.data());
        for (size_t i = 0; i < frames; ++i)
            out.samples[i] = static_cast<double>(tmp[i]);
    }
    drwav_uninit(&wav);

    if (read != frames) {
        error = "'" + path + "': read " + std::to_string(read) + " of " + std::to_string(frames) + " frames";
        return false;
    }
    return true;
}

bool writeMonoWavF64(const std::string& path, double sampleRate, const std::vector<double>& samples,
                     std::string& error) {
    drwav_data_format format;
    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    format.channels = 1;
    format.sampleRate = static_cast<drwav_uint32>(sampleRate);
    format.bitsPerSample = 64;

    drwav wav;
    if (!drwav_init_file_write(&wav, path.c_str(), &format, nullptr)) {
        error = "cannot create WAV file '" + path + "'";
        return false;
    }
    const drwav_uint64 written = drwav_write_pcm_frames(&wav, samples.size(), samples.data());
    drwav_uninit(&wav);

    if (written != samples.size()) {
        error = "'" + path + "': wrote " + std::to_string(written) + " of " + std::to_string(samples.size()) +
                " frames";
        return false;
    }
    return true;
}

}  // namespace ampwav
