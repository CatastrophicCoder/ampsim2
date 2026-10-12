// ampwav — WAV in -> engine block -> WAV out (SPEC §7 M0.2). See `ampwav --help`.
#include <iostream>
#include <string>
#include <vector>

#include "src/Cli.h"

int main(int argc, char** argv) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    std::string error;
    const auto opts = ampwav::parseArgs(args, error);
    if (!opts) {
        std::cerr << "ampwav: " << error << "\n\n";
        ampwav::printUsage(std::cerr);
        return 2;
    }
    if (opts->help) {
        ampwav::printUsage(std::cout);
        return 0;
    }
    return ampwav::run(*opts, opts->listNodes ? std::cout : std::cerr);
}
