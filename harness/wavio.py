"""64-bit float mono WAV I/O (same format as ampwav, DECISIONS D-013)."""
import numpy as np
import soundfile as sf

from . import config


def write_wav(path, x, rate=config.FS):
    sf.write(str(path), np.asarray(x, dtype=np.float64), int(rate), subtype='DOUBLE')


def read_wav(path, mono=False):
    x, rate = sf.read(str(path), dtype='float64', always_2d=True)
    if x.shape[1] != 1:
        if not mono:
            raise ValueError(f'{path}: expected mono, got {x.shape[1]} channels')
        x = x[:, :1]
    return x[:, 0], rate
