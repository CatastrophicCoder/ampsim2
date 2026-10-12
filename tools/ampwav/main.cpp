// ampwav — WAV in -> engine block -> WAV out. Placeholder; the command line (SPEC §7 M0.2) is
// implemented in M0 step 2.
#include <cstdio>

int main() {
    std::fprintf(stderr,
                 "usage: ampwav --block <name|full> --in in.wav --out out.wav --rate 384000 "
                 "[--param name=value ...] [--dump-node name=file.wav ...]\n"
                 "ampwav: not implemented yet (M0 step 2)\n");
    return 2;
}
