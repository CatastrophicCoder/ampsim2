"""M6 listening comparison: the same DI excerpts through the engine at several rates / power-amp rates and through
the X3 / X4 networks, as 48 kHz WAVs for a DAW (no cabinet: raw amp output across the 16 Ω load).

    python -m harness.listening        # writes harness/cache/listening/<date>/ (not committed)

Per excerpt and preset every file gets the same gain (the reference's peak at −1 dBFS), so level does not differ.
Folders: named/ (configuration in the file name), blind/ (letters, key in blind/KEY.txt), difference/ (each
configuration minus the reference, same gain). Exact J4, Newton maxIter 32 (D-052, D-053).
"""
import datetime
import random
import sys

import numpy as np
import soundfile as sf

from . import config
from . import x2_rates as x2
from . import x3_neural as x3
from . import x4_conditioned as x4
from .presets import PRESETS

RATE = 48000
EXCERPTS = {'GTR L': 10.0, 'GTR RR': 10.0}  # track: start (s)
SECONDS = 20.0
PRESET_IDS = ('CRUNCH', 'DRIVE', 'HOT')
REFERENCE = 'ref_16x'
ENGINE = {  # name: (factor, options)
    REFERENCE: (16, {}),
    'full_2x': (2, {}),
    'full_4x': (4, {}),
    'full_8x': (8, {}),
    'half_2x': (2, {'divisor': 2}),
    'half_4x': (4, {'divisor': 2}),
}


def write_wav(path, x, rate):
    sf.write(str(path), np.asarray(x, dtype=np.float32), int(rate), subtype='FLOAT')  # 32-bit float: every DAW


OUT = config.HARNESS / 'cache' / 'listening' / datetime.date.today().strftime('%Y%m%d')


def _renders(x, pid):
    knobs = PRESETS[pid].all_knobs()
    out = {name: x2._run(x, knobs, f, o)[0] for name, (f, o) in ENGINE.items()}
    out['net_x4'] = x4._student(x, [knobs[n] for n in x4.KNOB_NAMES])
    if pid == 'CRUNCH':
        out['net_x3'] = x3._student(x)
    return out


def main():
    tracks = x3._tracks()
    rng = random.Random(15)
    for sub in ('named', 'blind', 'difference'):
        (OUT / sub).mkdir(parents=True, exist_ok=True)
    key = []
    for track, start in EXCERPTS.items():
        x = tracks[track][int(start * RATE):int((start + SECONDS) * RATE)]
        for pid in PRESET_IDS:
            r = _renders(x, pid)
            gain = 10 ** (-1 / 20) / np.max(np.abs(r[REFERENCE]))
            tag = f'{track.replace(" ", "")}_{pid}'
            names = sorted(r)
            letters = list('ABCDEFGH'[:len(names)])
            rng.shuffle(letters)
            for name, letter in zip(names, letters):
                y = r[name] * gain
                write_wav(OUT / 'named' / f'{tag}_{name}.wav', y, RATE)
                write_wav(OUT / 'blind' / f'{tag}_{letter}.wav', y, RATE)
                key.append(f'{tag}_{letter}: {name}')
                if name != REFERENCE:
                    write_wav(OUT / 'difference' / f'{tag}_{name}_minus_ref.wav', (r[name] - r[REFERENCE]) * gain, RATE)
            print(f'listening: {tag} ({len(names)} versions, gain {20 * np.log10(gain):+.1f} dB)', flush=True)
    (OUT / 'blind' / 'KEY.txt').write_text('\n'.join(sorted(key)) + '\n')
    print(f'listening: wrote {OUT}')


if __name__ == '__main__':
    sys.exit(main())
