"""Test signals at 384 kHz (VERIFICATION.md §3). All signals start at 0 V so that SPICE and the engine
both start from the DC operating point.

    python -m harness.signals --levels b1 --out harness/renders/signals   # writes the WAVs
"""
import argparse
from dataclasses import dataclass, field
from pathlib import Path

import numpy as np

from . import config
from .wavio import read_wav, write_wav

SIN_FREQS = (100.0, 1000.0, 5000.0)
SIN_SECONDS = 0.150
SWEEP_SECONDS = 2.0
SWEEP_TAIL = 0.050
TT_SECONDS = 0.300
DI_SECONDS = 5.0
LEVEL_NAMES = ('small', 'nominal', 'hot', 'overdrive')


@dataclass
class Signal:
    id: str
    kind: str               # SIN, SWP-lo, SWP-hi, TT, BURST, STEP, DI
    x: np.ndarray           # volts at the block input, sampled at config.FS
    meta: dict = field(default_factory=dict)


def _t(seconds):
    return np.arange(int(round(seconds * config.FS))) / config.FS


def _fmt(v):
    return f'{v:g}'


def sine(freq, amp, seconds=SIN_SECONDS):
    x = amp * np.sin(2 * np.pi * freq * _t(seconds))
    return Signal(f'SIN-{_fmt(freq)}Hz-{_fmt(amp)}V', 'SIN', x, {'freq': freq, 'amp': amp})


def sweep(amp, kind, f0=20.0, f1=20000.0, seconds=SWEEP_SECONDS, tail=SWEEP_TAIL):
    """Exponential (Farina) sweep, phase 0 at t = 0, followed by `tail` seconds of silence so the
    response decays inside the record (needed for an exact Y/X magnitude estimate)."""
    t = _t(seconds)
    k = np.log(f1 / f0)
    x = amp * np.sin(2 * np.pi * f0 * seconds / k * (np.exp(t * k / seconds) - 1.0))
    x = np.r_[x, np.zeros(int(round(tail * config.FS)))]
    return Signal(f'{kind}-{_fmt(amp)}V', kind, x,
                  {'amp': amp, 'f0': f0, 'f1': f1, 'seconds': seconds, 'tail': tail})


def two_tone(amp, f1=220.0, f2=330.0, seconds=TT_SECONDS):
    t = _t(seconds)
    x = amp * (np.sin(2 * np.pi * f1 * t) + np.sin(2 * np.pi * f2 * t))
    return Signal(f'TT-{_fmt(amp)}V', 'TT', x, {'amp': amp, 'f1': f1, 'f2': f2})


def burst(amp_high, freq=1000.0):
    """1 kHz: 50 ms low, 300 ms high (+30 dB), 300 ms low."""
    amp_low = amp_high * 10 ** (-30 / 20)
    t = _t(0.650)
    env = np.where((t >= 0.050) & (t < 0.350), amp_high, amp_low)
    x = env * np.sin(2 * np.pi * freq * t)
    return Signal(f'BURST-{_fmt(amp_high)}V', 'BURST', x, {'amp_high': amp_high, 'amp_low': amp_low, 'freq': freq})


def step(amp, period=0.005, seconds=0.100):
    """DC-free pulse train: +amp, 0, -amp, 0, each `period` long."""
    t = _t(seconds)
    phase = np.floor(t / period).astype(int) % 4
    x = np.choose(phase, [amp, 0.0, -amp, 0.0]).astype(float)
    x[0] = 0.0
    return Signal(f'STEP-{_fmt(amp)}V', 'STEP', x, {'amp': amp, 'period': period})


def di_signals(volts_per_fs=config.INPUT_VOLTS_PER_FS, seconds=DI_SECONDS, di_dir=config.DI_DIR):
    """DI recordings from reference/di/, resampled to 384 kHz and scaled to volts at the jack.
    Returns [] (and says so) when there are none."""
    from math import gcd
    from scipy.signal import resample_poly

    files = sorted(Path(di_dir).glob('*.wav'))
    if not files:
        print(f'signals: no DI recordings in {di_dir}; DI tests skipped (see reference/di/README.md)')
        return []
    out = []
    for f in files:
        x, rate = read_wav(f, mono=True)
        x = x[: int(seconds * rate)]
        g = gcd(int(rate), config.FS)
        y = resample_poly(x, config.FS // g, int(rate) // g) * volts_per_fs
        ramp = min(len(y), int(0.002 * config.FS))       # 2 ms fade-in so the signal starts at 0 V
        y[:ramp] *= np.linspace(0.0, 1.0, ramp)
        out.append(Signal(f'DI-{f.stem}', 'DI', y, {'file': f.name, 'volts_per_fs': volts_per_fs}))
    return out


def standard_set(levels, include=('SIN', 'SWP-lo', 'SWP-hi', 'TT', 'BURST', 'STEP', 'DI')):
    """The standard signal set for a block, given its level table {level name: volts peak or None}."""
    lv = {k: v for k, v in levels.items() if v is not None}
    nominal = lv['nominal']
    small = lv.get('small', nominal)
    hot = lv.get('hot', nominal)
    sigs = []
    if 'SIN' in include:
        sigs += [sine(f, a) for a in lv.values() for f in SIN_FREQS]
    if 'SWP-lo' in include:
        sigs.append(sweep(small, 'SWP-lo'))
    if 'SWP-hi' in include and hot != small:
        sigs.append(sweep(nominal, 'SWP-hi'))
    if 'TT' in include:
        sigs.append(two_tone(nominal / 2))
    if 'BURST' in include:
        sigs.append(burst(hot))
    if 'STEP' in include:
        sigs.append(step(nominal))
    if 'DI' in include:
        sigs += di_signals()
    return sigs


def main():
    from .blocks import BLOCKS
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--levels', default='full', help='block id whose level table to use')
    ap.add_argument('--out', default=str(config.RENDER_DIR / 'signals'))
    a = ap.parse_args()
    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    for s in standard_set(BLOCKS[a.levels].levels):
        write_wav(out / f'{s.id}.wav', s.x)
        print(f'{s.id:28s} {len(s.x) / config.FS:6.3f} s  peak {np.max(np.abs(s.x)):.4g} V')


if __name__ == '__main__':
    main()
