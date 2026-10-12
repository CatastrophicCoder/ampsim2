#include "Cli.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <ostream>

#include "Runner.h"
#include "WavIO.h"

namespace ampwav {
namespace {

bool parseDouble(const std::string& s, double& out) {
    if (s.empty())
        return false;
    char* end = nullptr;
    out = std::strtod(s.c_str(), &end);
    return end == s.c_str() + s.size() && std::isfinite(out);
}

double* knob(amp2203::Params& p, const std::string& name) {
    if (name == "preampVolume") return &p.preampVolume;
    if (name == "treble") return &p.treble;
    if (name == "middle") return &p.middle;
    if (name == "bass") return &p.bass;
    if (name == "master") return &p.master;
    if (name == "presence") return &p.presence;
    return nullptr;
}

bool splitPair(const std::string& s, std::string& key, std::string& value) {
    const auto eq = s.find('=');
    if (eq == std::string::npos || eq == 0 || eq + 1 == s.size())
        return false;
    key = s.substr(0, eq);
    value = s.substr(eq + 1);
    return true;
}

void writeStats(const std::string& path, const Options& o, double rate, size_t samples, double seconds,
                const amp2203::Amp2203::Stats& st) {
    std::ofstream f(path);
    f << std::setprecision(17) << "{\n"
      << "  \"block\": \"" << o.block << "\",\n"
      << "  \"sample_rate\": " << rate << ",\n"
      << "  \"samples\": " << samples << ",\n"
      << "  \"process_seconds\": " << seconds << ",\n"
      << "  \"ns_per_sample\": " << (samples ? 1e9 * seconds / static_cast<double>(samples) : 0.0) << ",\n"
      << "  \"realtime_factor\": " << (seconds > 0 ? static_cast<double>(samples) / rate / seconds : 0.0) << ",\n"
      << "  \"newton_failures\": " << st.newtonFailures << ",\n"
      << "  \"max_iterations_used\": " << st.maxIterationsUsed << "\n"
      << "}\n";
}

}  // namespace

void printUsage(std::ostream& os) {
    os << "usage: ampwav --block <id> --in in.wav --out out.wav [--rate Hz]\n"
          "              [--param name=value ...] [--dump-node node=file.wav ...]\n"
          "              [--units volts|fs] [--block-size N] [--stats stats.json] [--list-nodes]\n"
          "\n"
          "  --block       full, b1 .. b7, b56 (M0: every block except full is a pass-through)\n"
          "  --in          mono WAV; 64-bit float is read bit-exact, other formats via 32-bit float\n"
          "  --out         written as mono 64-bit float WAV at the input rate\n"
          "  --rate        engine rate; must equal the input file's rate (no resampling)\n"
          "  --param       knob rotation 0..1: preampVolume treble middle bass master presence\n"
          "  --dump-node   write a probed node voltage (volts) per sample to a WAV\n"
          "  --units       volts (default): samples are volts at the block input/output;\n"
          "                fs: full-scale samples as in the plugin (full amp only)\n"
          "  --block-size  samples per process() call when no nodes are dumped (default 512)\n"
          "  --stats       write timing and Newton statistics as JSON\n"
          "  --list-nodes  print the probe names of the block and exit\n";
}

std::optional<Options> parseArgs(const std::vector<std::string>& args, std::string& error) {
    Options o;
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        auto value = [&](std::string& v) {
            if (i + 1 >= args.size()) {
                error = "missing value for " + a;
                return false;
            }
            v = args[++i];
            return true;
        };
        std::string v;
        if (a == "--help" || a == "-h") {
            o.help = true;
        } else if (a == "--list-nodes") {
            o.listNodes = true;
        } else if (a == "--block") {
            if (!value(o.block)) return std::nullopt;
        } else if (a == "--in") {
            if (!value(o.inPath)) return std::nullopt;
        } else if (a == "--out") {
            if (!value(o.outPath)) return std::nullopt;
        } else if (a == "--stats") {
            if (!value(o.statsPath)) return std::nullopt;
        } else if (a == "--rate") {
            if (!value(v)) return std::nullopt;
            double r = 0.0;
            if (!parseDouble(v, r) || r <= 0.0) {
                error = "invalid --rate '" + v + "'";
                return std::nullopt;
            }
            o.rate = r;
        } else if (a == "--block-size") {
            if (!value(v)) return std::nullopt;
            double b = 0.0;
            if (!parseDouble(v, b) || b < 1.0 || b > 1 << 20 || b != std::floor(b)) {
                error = "invalid --block-size '" + v + "'";
                return std::nullopt;
            }
            o.blockSize = static_cast<int>(b);
        } else if (a == "--units") {
            if (!value(v)) return std::nullopt;
            if (v == "volts") o.units = Units::Volts;
            else if (v == "fs") o.units = Units::FullScale;
            else {
                error = "invalid --units '" + v + "' (volts or fs)";
                return std::nullopt;
            }
        } else if (a == "--param") {
            if (!value(v)) return std::nullopt;
            std::string key, val;
            double x = 0.0;
            if (!splitPair(v, key, val)) {
                error = "--param expects name=value, got '" + v + "'";
                return std::nullopt;
            }
            double* k = knob(o.params, key);
            if (!k) {
                error = "unknown parameter '" + key + "'";
                return std::nullopt;
            }
            if (!parseDouble(val, x) || x < 0.0 || x > 1.0) {
                error = "parameter " + key + " must be a rotation in 0..1, got '" + val + "'";
                return std::nullopt;
            }
            *k = x;
        } else if (a == "--dump-node") {
            if (!value(v)) return std::nullopt;
            NodeDump d;
            if (!splitPair(v, d.node, d.path)) {
                error = "--dump-node expects node=file.wav, got '" + v + "'";
                return std::nullopt;
            }
            o.dumps.push_back(d);
        } else {
            error = "unknown option '" + a + "'";
            return std::nullopt;
        }
    }

    if (o.help)
        return o;
    if (!makeRunner(o.block)) {
        error = "unknown block '" + o.block + "'";
        return std::nullopt;
    }
    if (o.listNodes)
        return o;
    if (o.inPath.empty() || o.outPath.empty()) {
        error = "--in and --out are required";
        return std::nullopt;
    }
    if (o.units == Units::FullScale && o.block != "full") {
        error = "--units fs is only valid with --block full";
        return std::nullopt;
    }
    return o;
}

int run(const Options& o, std::ostream& log) {
    auto runner = makeRunner(o.block);
    if (!runner) {
        log << "ampwav: unknown block '" << o.block << "'\n";
        return 1;
    }

    if (o.listNodes) {
        runner->prepare(o.rate.value_or(384000.0), o.blockSize);
        for (const auto& p : runner->probes())
            log << p.name << "\n";
        return 0;
    }

    std::string err;
    MonoWav in;
    if (!readMonoWav(o.inPath, in, err)) {
        log << "ampwav: " << err << "\n";
        return 1;
    }
    if (o.rate && *o.rate != in.sampleRate) {
        log << "ampwav: --rate " << *o.rate << " differs from the input file's rate " << in.sampleRate
            << " (ampwav does not resample)\n";
        return 1;
    }
    const double rate = in.sampleRate;

    runner->prepare(rate, o.blockSize);
    runner->setParams(o.params);

    // resolve probes before processing so a typo fails fast
    std::vector<const double*> dumpSrc;
    for (const auto& d : o.dumps) {
        const double* src = nullptr;
        for (const auto& p : runner->probes())
            if (d.node == p.name)
                src = p.value;
        if (!src) {
            log << "ampwav: block '" << o.block << "' has no node '" << d.node << "'; available:";
            for (const auto& p : runner->probes())
                log << " " << p.name;
            log << "\n";
            return 1;
        }
        dumpSrc.push_back(src);
    }

    std::vector<double> y = in.samples;
    if (o.units == Units::FullScale)
        for (double& s : y)
            s *= runner->inputVoltsPerFullScale();

    const size_t total = y.size();
    std::vector<std::vector<double>> dumps(o.dumps.size(), std::vector<double>(total));

    const auto t0 = std::chrono::steady_clock::now();
    if (dumps.empty()) {
        for (size_t pos = 0; pos < total; pos += static_cast<size_t>(o.blockSize)) {
            const auto n = static_cast<int>(std::min(static_cast<size_t>(o.blockSize), total - pos));
            runner->processVolts(y.data() + pos, n);
        }
    } else {
        for (size_t n = 0; n < total; ++n) {
            runner->processVolts(y.data() + n, 1);
            for (size_t k = 0; k < dumps.size(); ++k)
                dumps[k][n] = *dumpSrc[k];
        }
    }
    const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

    if (o.units == Units::FullScale)
        for (double& s : y)
            s /= runner->outputVoltsPerFullScale();

    if (!writeMonoWavF64(o.outPath, rate, y, err)) {
        log << "ampwav: " << err << "\n";
        return 1;
    }
    for (size_t k = 0; k < dumps.size(); ++k) {
        if (!writeMonoWavF64(o.dumps[k].path, rate, dumps[k], err)) {
            log << "ampwav: " << err << "\n";
            return 1;
        }
    }

    const auto st = runner->stats();
    const double nsPerSample = total ? 1e9 * seconds / static_cast<double>(total) : 0.0;
    log << "ampwav: block=" << o.block << " samples=" << total << " rate=" << rate << " time=" << seconds
        << " s ns/sample=" << nsPerSample << " newtonFailures=" << st.newtonFailures
        << " maxIterations=" << st.maxIterationsUsed << "\n";
    if (!o.statsPath.empty())
        writeStats(o.statsPath, o, rate, total, seconds, st);
    return 0;
}

}  // namespace ampwav
