/* Single translation unit holding the dr_wav implementation (compiled without the project's -Werror). */
#define DR_WAV_IMPLEMENTATION
/* dr_wav rejects files above 384 kHz by default; the engine runs at 768 kHz (D-035) and the
   discretisation checks at up to 4x that, so ampwav accepts up to 3.072 MHz (DECISIONS D-013). */
#define DRWAV_MAX_SAMPLE_RATE 3072000
#include <dr_wav.h>
