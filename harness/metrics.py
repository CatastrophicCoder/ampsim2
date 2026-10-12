"""Comparison metrics (VERIFICATION.md §6). `ref` is the SPICE reference, `y` the signal under test."""
import numpy as np

from . import config

TINY = 1e-300


def esr_db(ref, y):
    """10·log10( Σ(y−ŷ)² / Σy² ), with y = reference and ŷ = signal under test."""
    ref, y = np.asarray(ref), np.asarray(y)
    return float(10 * np.log10((np.sum((ref - y) ** 2) + TINY) / (np.sum(ref ** 2) + TINY)))


def best_lag(ref, y, max_lag=2):
    """Integer lag in [-max_lag, max_lag] minimising ESR; positive lag = y is late. Returns (lag, esr_db)."""
    best = (0, esr_db(ref, y))
    n = len(ref)
    for lag in range(-max_lag, max_lag + 1):
        if lag == 0:
            continue
        if lag > 0:
            e = esr_db(ref[: n - lag], y[lag:])
        else:
            e = esr_db(ref[-lag:], y[: n + lag])
        if e < best[1]:
            best = (lag, e)
    return best


def harmonics(x, f0, fs=config.FS, tlast=0.040, count=10):
    """Peak amplitudes (V) of harmonics 1..count over the last `tlast` seconds (Hann window)."""
    seg = np.asarray(x[-int(round(tlast * fs)):], dtype=float)
    seg = seg - seg.mean()
    w = np.hanning(len(seg))
    spec = np.abs(np.fft.rfft(seg * w)) * 2.0 / np.sum(w)
    fr = np.fft.rfftfreq(len(seg), 1.0 / fs)
    out = []
    for k in range(1, count + 1):
        i = int(np.argmin(np.abs(fr - k * f0)))
        out.append(float(spec[max(i - 2, 0): i + 3].max()))
    return np.array(out)


def thd_percent(h):
    """THD (%) from harmonic amplitudes h[0] = fundamental."""
    return float(np.sqrt(np.sum(h[1:] ** 2)) / max(h[0], TINY) * 100.0)


def harmonic_diff_db(h_ref, h, floor_dbc=-80.0):
    """Per-harmonic level difference (dB) and a mask of harmonics scored (reference above floor_dbc)."""
    ref_db = 20 * np.log10(np.maximum(h_ref, TINY))
    db = 20 * np.log10(np.maximum(h, TINY))
    scored = ref_db - ref_db[0] > floor_dbc
    return db - ref_db, scored


def sweep_response_db(x, y, freqs, fs=config.FS):
    """|Y/X| in dB at `freqs` from an excitation x and response y: per-bin FFT ratio, complex-interpolated
    to `freqs`, no smoothing. Exact for a linear system when the response has decayed by the end of the
    record (sweeps carry a silent tail). Only meaningful inside the band the excitation covers."""
    n = len(x)
    nfft = 1 << int(np.ceil(np.log2(2 * n)))
    X = np.fft.rfft(x, nfft)
    Y = np.fft.rfft(y, nfft)
    fr = np.fft.rfftfreq(nfft, 1.0 / fs)
    H = Y / np.where(np.abs(X) > TINY, X, TINY)
    h = np.interp(freqs, fr, H.real) + 1j * np.interp(freqs, fr, H.imag)
    return 20 * np.log10(np.maximum(np.abs(h), TINY))


def mean_power(v, r_load, fs=config.FS, tlast=0.040):
    seg = np.asarray(v[-int(round(tlast * fs)):])
    return float(np.mean(seg ** 2) / r_load)
