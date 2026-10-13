/* Single translation unit holding the dr_wav implementation (compiled without the project's -Werror). */
#define DR_WAV_IMPLEMENTATION
/* dr_wav rejects files above 384 kHz by default; the engine also runs at 16x oversampling (768 kHz at
   48 kHz, 1.536 MHz at 96 kHz, SPEC 3.4), so ampwav accepts up to 1.536 MHz (DECISIONS D-013). */
#define DRWAV_MAX_SAMPLE_RATE 1536000
#include <dr_wav.h>
