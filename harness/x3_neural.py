"""M6 experiment X3 (docs/ARCHITECTURE_STUDY.md §6): a small recurrent network trained on the engine's output at
one knob setting (CRUNCH), evaluated against SPICE, timed in C++, checked for aliasing.

    python -m harness.x3_neural render     # teacher: ampwav full amp at 768 kHz on the DI tracks, 4 input gains
    python -m harness.x3_neural train      # LSTM-32 at 48 kHz (PyTorch, MPS or CPU)
    python -m harness.x3_neural evaluate   # vs SPICE CRUNCH DI references, aliasing, C++ cost; writes the report

Data: the four DI tracks resampled to 48 kHz (the student's rate) and from there to 768 kHz for the teacher, so
both see the same band-limited input. Teacher output decimated to 48 kHz. Held out: the first 5 s of every track
(the SPICE CRUNCH DI references) and the last 20 s (validation). Units: full-scale samples as in the plugin
(input × 4.361 V, output / 60 V).
"""
import argparse
import datetime
import json
import os
import resource
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor
from math import gcd
from pathlib import Path

import numpy as np
from scipy.signal import resample_poly

from . import config, metrics
from .wavio import read_wav, write_wav

RATE = 48000
TEACHER_FACTOR = 16
KNOBS = {'preampVolume': 0.6, 'treble': 0.5, 'middle': 0.5, 'bass': 0.5, 'master': 0.3, 'presence': 0.5}  # CRUNCH
GAINS_DB = (-6.0, 0.0, 6.0, 12.0)
HOLD_HEAD_S, HOLD_TAIL_S = 5.0, 20.0
CACHE = config.HARNESS / 'cache' / 'x3'
AMPWAV = config.ROOT / 'build' / 'tools' / 'ampwav' / 'ampwav'
HIDDEN = 32


def _tracks():
    out = {}
    for f in sorted(Path(config.DI_DIR).glob('*.wav')):
        x, rate = read_wav(f, mono=True)
        g = gcd(int(rate), RATE)
        out[f.stem] = resample_poly(x, RATE // g, int(rate) // g)
    return out


def _teacher(x48, knobs=KNOBS, j4=None):
    """Full amp at 768 kHz on a 48 kHz full-scale input; output decimated to 48 kHz (full scale)."""
    hi = resample_poly(x48, TEACHER_FACTOR, 1)
    with tempfile.TemporaryDirectory(prefix='amp2203_x3_') as d:
        d = Path(d)
        write_wav(d / 'in.wav', hi, RATE * TEACHER_FACTOR)
        args = [str(AMPWAV), '--block', 'full', '--in', str(d / 'in.wav'), '--out', str(d / 'out.wav'),
                '--units', 'fs', '--rate', str(RATE * TEACHER_FACTOR)]
        args += [a for k, v in knobs.items() for a in ('--param', f'{k}={v!r}')]
        args += ['--j4', j4] if j4 else []
        r = subprocess.run(args, capture_output=True, text=True)
        if r.returncode != 0:
            raise RuntimeError(r.stderr)
        y, _ = read_wav(d / 'out.wav')
    return resample_poly(y, 1, TEACHER_FACTOR)


def render():
    CACHE.mkdir(parents=True, exist_ok=True)
    tracks = _tracks()
    jobs = [(name, gdb) for name in tracks for gdb in GAINS_DB]

    def one(job):
        name, gdb = job
        path = CACHE / f'{name}_{gdb:+.0f}dB.npz'
        if path.exists():
            return path, 0.0
        x = tracks[name] * 10 ** (gdb / 20)
        t = time.time()
        y = _teacher(x)
        np.savez(path, x=x, y=y[:len(x)])
        return path, time.time() - t

    with ThreadPoolExecutor(max_workers=os.cpu_count() - 2) as ex:
        for path, secs in ex.map(one, jobs):
            print(f'x3: {path.name} ({secs:.0f} s)')


def _load(split):
    """Arrays (x, y) for 'train' (between the held-out head and tail) or 'val' (the tail)."""
    xs, ys = [], []
    h, t = int(HOLD_HEAD_S * RATE), int(HOLD_TAIL_S * RATE)
    for f in sorted(CACHE.glob('*dB.npz')):
        d = np.load(f)
        x, y = d['x'], d['y']
        s = slice(h, len(x) - t) if split == 'train' else slice(len(x) - t, len(x))
        xs.append(x[s].astype(np.float32))
        ys.append(y[s].astype(np.float32))
    return xs, ys


def _model():
    import torch

    class Rnn(torch.nn.Module):
        """LSTM-32 + linear output + input skip (Wright et al. 2020)."""

        def __init__(self):
            super().__init__()
            self.rnn = torch.nn.LSTM(1, HIDDEN, batch_first=True)
            self.lin = torch.nn.Linear(HIDDEN, 1)

        def forward(self, x, state=None):
            h, state = self.rnn(x, state)
            return self.lin(h) + x, state

    return Rnn()


def _esr_loss(y, t):
    import torch
    # pre-emphasis 1 − 0.85 z⁻¹ (Wright et al.) plus a DC term
    pe = lambda s: s[:, 1:] - 0.85 * s[:, :-1]
    e = torch.sum((pe(t) - pe(y)) ** 2) / (torch.sum(pe(t) ** 2) + 1e-10)
    dc = torch.mean(torch.mean(t - y, dim=1) ** 2) / (torch.mean(t ** 2) + 1e-10)
    return e + dc


def train(epochs=1200, seg=RATE // 2, batch=48, tbptt=2048, warm=1000):
    import torch
    torch.manual_seed(0)
    np.random.seed(0)
    dev = 'cpu'  # an LSTM-32 trains ≈ 17× faster per step on the CPU than on MPS (0.18 vs ≈ 3 s, batch 48 × 2048)
    torch.set_num_threads(8)
    xs, ys = _load('train')
    vx, vy = _load('val')
    vx, vy = [x[:5 * RATE] for x in vx], [y[:5 * RATE] for y in vy]  # 5 s per validation track while training
    model = _model().to(dev)
    opt = torch.optim.Adam(model.parameters(), lr=1e-3)  # 5e-3 diverged at epoch ≈ 80
    sched = torch.optim.lr_scheduler.ReduceLROnPlateau(opt, factor=0.5, patience=3)
    best, log, start = float('inf'), [], 0
    t0 = time.time()
    ckpt = CACHE / 'train_state.pt'
    if ckpt.exists():  # resume after an interruption (written at every validation)
        st = torch.load(ckpt, weights_only=False)
        model.load_state_dict(st['model'])
        opt.load_state_dict(st['opt'])
        sched.load_state_dict(st['sched'])
        best, log, start = st['best'], st['log'], st['ep']
        np.random.set_state(st['np_rng'])
        torch.set_rng_state(st['torch_rng'])
        t0 -= log[-1][3] if log else 0.0
        print(f'x3: resuming at epoch {start}', flush=True)
    for ep in range(start, epochs):
        model.train()
        bx, by = [], []
        for _ in range(batch):
            k = np.random.randint(len(xs))
            s = np.random.randint(0, len(xs[k]) - seg)
            bx.append(xs[k][s:s + seg])
            by.append(ys[k][s:s + seg])
        bx = torch.tensor(np.stack(bx))[:, :, None].to(dev)
        by = torch.tensor(np.stack(by))[:, :, None].to(dev)
        with torch.no_grad():  # warm the state, not scored
            _, state = model(bx[:, :warm])
        total = 0.0
        for s in range(warm, seg, tbptt):
            state = tuple(z.detach() for z in state)
            out, state = model(bx[:, s:s + tbptt], state)
            loss = _esr_loss(out, by[:, s:s + tbptt])
            opt.zero_grad()
            loss.backward()
            torch.nn.utils.clip_grad_norm_(model.parameters(), 1.0)
            opt.step()
            total += loss.item()
        if ep % 20 == 19 or ep == epochs - 1:
            v = float(validate(model, vx, vy, dev))
            sched.step(v)  # patience counts validations (20 epochs each)
            log.append((ep + 1, total, v, time.time() - t0))
            rss = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss / 2 ** 30  # bytes on macOS
            print(f'x3: epoch {ep + 1} train {total:.4f} val ESR {10 * np.log10(v):.1f} dB ({time.time() - t0:.0f} s, '
                  f'peak RSS {rss:.1f} GB)', flush=True)
            if v < best:
                best = v
                torch.save(model.state_dict(), CACHE / 'lstm32.pt')
            torch.save({'model': model.state_dict(), 'opt': opt.state_dict(), 'sched': sched.state_dict(),
                        'best': best, 'log': log, 'ep': ep + 1, 'np_rng': np.random.get_state(),
                        'torch_rng': torch.get_rng_state()}, ckpt)
    (CACHE / 'train_log.json').write_text(json.dumps(log))
    ckpt.unlink(missing_ok=True)


def validate(model, vx, vy, dev):
    import torch
    model.eval()
    num = den = 0.0
    with torch.no_grad():
        for x, y in zip(vx, vy):
            out, _ = model(torch.tensor(x)[None, :, None].to(dev))
            o = out[0, :, 0].cpu().numpy()
            num += np.sum((o[RATE:] - y[RATE:]) ** 2)
            den += np.sum(y[RATE:] ** 2)
    return num / den


def _student(x):
    import torch
    model = _model()
    model.load_state_dict(torch.load(CACHE / 'lstm32.pt', map_location='cpu'))
    model.eval()
    with torch.no_grad():
        out, _ = model(torch.tensor(x.astype(np.float32))[None, :, None])
    return out[0, :, 0].numpy().astype(np.float64)


def _band48(y, rate):
    g = gcd(int(rate), RATE)
    return resample_poly(y, RATE // g, int(rate) // g)


def _spice_crunch_di():
    """SPICE CRUNCH DI references (5 s, full netlist at config.FS) as {track: (x48 full scale, out48 full scale)}."""
    from . import cascade, signals
    from .blocks import BLOCKS
    from .presets import PRESETS
    texts = cascade._texts()
    knobs = PRESETS['CRUNCH'].all_knobs()
    out = {}
    for sig in signals.di_signals():
        f = cascade.render_f(texts, knobs, sig)
        name = sig.id.replace('DI-', '')
        out[name] = (f['out'] / 60.0)
    return out


def _alias_test(student_fn, teacher_fn=_teacher):
    """5 kHz and 9 kHz sines at two levels: power at non-harmonic frequencies relative to the total (48 kHz),
    student vs teacher (768 kHz, decimated)."""
    rows = []
    n = RATE
    t = np.arange(2 * n) / RATE
    for f0 in (5000.0, 9000.0):
        for amp in (0.1, 0.4):
            x = amp * np.sin(2 * np.pi * f0 * t)
            ys = student_fn(x)[n:]
            yt = teacher_fn(x)[n:2 * n]

            def nonharm(y):
                w = np.hanning(len(y))
                Y = np.abs(np.fft.rfft(y * w)) ** 2
                fr = np.fft.rfftfreq(len(y), 1 / RATE)
                harm = np.zeros_like(Y, bool)
                for k in range(0, int(RATE / 2 / f0) + 1):
                    harm |= np.abs(fr - k * f0) < 30
                return 10 * np.log10(Y[~harm].sum() / Y.sum())
            rows.append((f0, amp, nonharm(yt), nonharm(ys)))
    return rows


def _cpp_cost(sd=None):
    """ns per sample of a hand-written LSTM-32 (same weights; one audio input) in C++ (-O3), single thread."""
    import torch
    sd = torch.load(CACHE / 'lstm32.pt', map_location='cpu') if sd is None else sd
    w = np.concatenate([sd[k].numpy().ravel() for k in
                        ('rnn.weight_ih_l0', 'rnn.weight_hh_l0', 'rnn.bias_ih_l0', 'rnn.bias_hh_l0', 'lin.weight',
                         'lin.bias')]).astype(np.float32)
    src = CACHE / 'lstm_bench.cpp'
    src.write_text(r'''
#include <chrono>
#include <cmath>
#include <cstdio>
#include <vector>
constexpr int H = %d;
int main(int, char** argv) {
    std::vector<float> w(4 * H + 4 * H * H + 8 * H + H + 1);
    FILE* f = std::fopen(argv[1], "rb"); std::fread(w.data(), 4, w.size(), f); std::fclose(f);
    const float* wih = w.data(); const float* whh = wih + 4 * H; const float* bih = whh + 4 * H * H;
    const float* bhh = bih + 4 * H; const float* lw = bhh + 4 * H; const float lb = lw[H];
    float b[4 * H]; for (int k = 0; k < 4 * H; ++k) b[k] = bih[k] + bhh[k];
    float h[H] = {}, c[H] = {}, g[4 * H];
    const int n = 48000 * 20; double sink = 0;
    auto t0 = std::chrono::steady_clock::now();
    for (int s = 0; s < n; ++s) {
        const float x = 0.3f * std::sin(0.01f * s);
        for (int k = 0; k < 4 * H; ++k) {
            float a = b[k] + wih[k] * x;
            const float* r = whh + k * H;
            for (int j = 0; j < H; ++j) a += r[j] * h[j];
            g[k] = a;
        }
        float y = lb + x;
        for (int j = 0; j < H; ++j) {
            const float i = 1 / (1 + std::exp(-g[j])), fg = 1 / (1 + std::exp(-g[H + j]));
            const float gg = std::tanh(g[2 * H + j]), o = 1 / (1 + std::exp(-g[3 * H + j]));
            c[j] = fg * c[j] + i * gg;
            h[j] = o * std::tanh(c[j]);
            y += lw[j] * h[j];
        }
        sink += y;
    }
    const double ns = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count() * 1e9 / n;
    std::printf("%%.1f %%g\n", ns, sink);
}
''' % HIDDEN)
    wb = CACHE / 'lstm32.bin'
    w.tofile(wb)
    exe = CACHE / 'lstm_bench'
    subprocess.run(['clang++', '-std=c++20', '-O3', '-ffast-math', str(src), '-o', str(exe)], check=True)
    ns = min(float(subprocess.run([str(exe), str(wb)], capture_output=True, text=True).stdout.split()[0])
             for _ in range(3))
    return ns


def evaluate():
    tracks = _tracks()
    spice = _spice_crunch_di()
    rows = []
    for name, ref_hi in spice.items():
        n5 = int(HOLD_HEAD_S * RATE)
        x = tracks[name][:n5]
        ref = _band48(ref_hi, config.FS)[:n5]
        teach = _teacher(x)[:n5]
        stud = _student(x)[:n5]
        s = slice(int(0.1 * RATE), n5)  # the student starts from a zero state
        rows.append((name, metrics.esr_db(ref[s], teach[s]), metrics.esr_db(ref[s], stud[s]),
                     metrics.esr_db(teach[s], stud[s])))
        print(f'x3: {name}: teacher vs SPICE {rows[-1][1]:.1f} dB, student vs SPICE {rows[-1][2]:.1f} dB, '
              f'student vs teacher {rows[-1][3]:.1f} dB', flush=True)
    vx, vy = _load('val')
    import torch
    model = _model()
    model.load_state_dict(torch.load(CACHE / 'lstm32.pt', map_location='cpu'))
    val = validate(model, vx, vy, 'cpu')
    alias = _alias_test(_student)
    ns = _cpp_cost()
    log = json.loads((CACHE / 'train_log.json').read_text())
    write_report(rows, val, alias, ns, log)


def write_report(rows, val, alias, ns, log):
    out = config.REPORT_DIR / 'M6'
    out.mkdir(parents=True, exist_ok=True)
    path = out / f'{datetime.date.today().strftime("%Y%m%d")}_x3_neural.md'
    lines = ['# M6 experiment X3: LSTM-32 trained on the engine (CRUNCH), vs SPICE', '',
             f'- Date {datetime.date.today().isoformat()}; teacher `ampwav --block full` at {RATE * TEACHER_FACTOR} Hz, '
             f'knobs CRUNCH {KNOBS}; DI tracks at input gains {GAINS_DB} dB; student at {RATE} Hz '
             f'(LSTM {HIDDEN} + linear + input skip, PyTorch, pre-emphasised ESR loss).',
             f'- Held out: first {HOLD_HEAD_S:g} s (SPICE references, full netlist at {config.FS} Hz) and last '
             f'{HOLD_TAIL_S:g} s of every track (validation). Training: {log[-1][0]} epochs, {log[-1][3]:.0f} s.',
             '', '## Accuracy (48 kHz band, first 0.1 s excluded)', '',
             '| Track | Teacher vs SPICE | Student vs SPICE | Student vs teacher |', '|---|---|---|---|']
    for name, a, b, c in rows:
        lines.append(f'| {name} | {a:.1f} dB | {b:.1f} dB | {c:.1f} dB |')
    lines += ['', f'Validation (last {HOLD_TAIL_S:g} s of every track, 4 gains): student vs teacher ESR '
              f'{10 * np.log10(val):.1f} dB.', '',
              '## Aliasing (sines; power outside the harmonics of f0, relative to the total)', '',
              '| f0 | amplitude (FS) | teacher (768 kHz → 48 kHz) | student (48 kHz) |', '|---|---|---|---|']
    for f0, amp, t, s in alias:
        lines.append(f'| {f0:.0f} Hz | {amp:g} | {t:.1f} dB | {s:.1f} dB |')
    lines += ['', '## Cost', '',
              f'Hand-written LSTM-{HIDDEN} in C++ (float, -O3 -ffast-math, one core of the M4 Pro): {ns:.1f} ns per '
              f'48 kHz sample = {ns / 20833.3 * 100:.2f} % of one core at 48 kHz (no oversampling). The engine at 8x: '
              '≈ 86 % (D-049 baseline).', '', '## Training log (epoch, train loss sum, validation ESR)', '']
    lines += [f'- {e}: {tl:.4f}, {10 * np.log10(v):.1f} dB ({t:.0f} s)' for e, tl, v, t in log[::4] + [log[-1]]]
    path.write_text('\n'.join(lines) + '\n')
    print(f'x3: wrote {path.relative_to(config.ROOT)}')


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('step', choices=('render', 'train', 'evaluate'))
    ap.add_argument('--epochs', type=int, default=1200)
    a = ap.parse_args()
    if a.step == 'render':
        render()
    elif a.step == 'train':
        train(a.epochs)
    else:
        evaluate()


if __name__ == '__main__':
    sys.exit(main())
