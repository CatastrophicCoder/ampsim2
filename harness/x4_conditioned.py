"""M6 experiment X4 (docs/ARCHITECTURE_STUDY.md §6): X3's LSTM-32 with three knobs as extra inputs (Preamp Volume,
Master, Presence; tone stack at 0.5), trained on the engine with the exact J4 coupling (X1), evaluated against SPICE
at CRUNCH, DRIVE and HOT and against the engine at unseen knob settings.

    python -m harness.x4_conditioned render     # teacher segments at random knob settings (768 kHz, --j4 exact)
    python -m harness.x4_conditioned train      # conditioned LSTM-32 at 48 kHz (PyTorch, CPU), resumable
    python -m harness.x4_conditioned evaluate   # vs SPICE (cached 384 kHz references), vs teacher, aliasing, cost

Data: 96 training settings, each knob uniform in [0, 1] with one in five at 0 or 1, each a 20 s segment of a random
DI track (between X3's held-out first 5 s and last 20 s) at a random input gain; 16 validation settings on the last
20 s of a track. Units as X3 (full-scale samples).
"""
import argparse
import datetime
import json
import os
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor

import numpy as np

from . import config, metrics
from . import x3_neural as x3

RATE = x3.RATE
KNOB_NAMES = ('preampVolume', 'master', 'presence')
N_TRAIN, N_VAL = 96, 16
SEG_S, WARM_S = 20.0, 0.5           # scored segment; teacher warm-up before it (dropped)
WORKERS = 6                         # ≈ 1 GB per teacher render at 768 kHz
CACHE = config.HARNESS / 'cache' / 'x4'
EVAL_PRESETS = ('CRUNCH', 'DRIVE', 'HOT')
SPICE_FS = 384000                   # the HOT DI references exist at 384 kHz only (X1)


def _all_knobs(k):
    return {**config.DEFAULT_KNOBS, **dict(zip(KNOB_NAMES, (float(v) for v in k)))}


def _settings():
    """[(split, index, track, start sample, gain dB, knob vector)], deterministic."""
    rng = np.random.default_rng(4)
    tracks = x3._tracks()
    names = sorted(tracks)
    seg, warm = int(SEG_S * RATE), int(WARM_S * RATE)
    h, t = int(x3.HOLD_HEAD_S * RATE), int(x3.HOLD_TAIL_S * RATE)
    out = []
    for split, n in (('train', N_TRAIN), ('val', N_VAL)):
        for i in range(n):
            k = rng.uniform(0.0, 1.0, 3)
            edge = rng.uniform(size=3) < 0.2
            k[edge] = rng.integers(0, 2, edge.sum())
            name = names[rng.integers(len(names))]
            n_all = len(tracks[name])
            if split == 'train':
                start = int(rng.integers(h + warm, n_all - t - seg))
            else:
                start = n_all - t  # the held-out tail; its warm-up reaches back into the training region
            gdb = float(rng.choice(x3.GAINS_DB))
            out.append((split, i, name, start, gdb, np.round(k, 4)))
    return out, tracks


def render():
    CACHE.mkdir(parents=True, exist_ok=True)
    jobs, tracks = _settings()
    warm = int(WARM_S * RATE)
    seg = int(SEG_S * RATE)

    def one(job):
        split, i, name, start, gdb, k = job
        path = CACHE / f'{split}_{i:03d}.npz'
        if path.exists():
            return path, 0.0
        x = tracks[name][start - warm:start + seg] * 10 ** (gdb / 20)
        t0 = time.time()
        y = x3._teacher(x, _all_knobs(k), 'exact')[:len(x)]
        np.savez(path, x=x[warm:].astype(np.float32), y=y[warm:].astype(np.float32), k=k.astype(np.float32),
                 track=name, start=start, gain_db=gdb)
        return path, time.time() - t0

    with ThreadPoolExecutor(max_workers=WORKERS) as ex:
        for path, secs in ex.map(one, jobs):
            print(f'x4: {path.name} ({secs:.0f} s)', flush=True)


def _load(split):
    xs, ys, ks = [], [], []
    for f in sorted(CACHE.glob(f'{split}_*.npz')):
        with np.load(f) as d:
            xs.append(d['x'])
            ys.append(d['y'])
            ks.append(d['k'])
    return xs, ys, ks


def _model():
    import torch

    class Rnn(torch.nn.Module):
        """LSTM-32 on (audio, knobs) + linear output + audio skip (Wright et al. 2020, conditioned)."""

        def __init__(self):
            super().__init__()
            self.rnn = torch.nn.LSTM(1 + len(KNOB_NAMES), x3.HIDDEN, batch_first=True)
            self.lin = torch.nn.Linear(x3.HIDDEN, 1)

        def forward(self, x, k, state=None):
            inp = torch.cat([x, k[:, None, :].expand(-1, x.shape[1], -1)], dim=2)
            h, state = self.rnn(inp, state)
            return self.lin(h) + x, state

    return Rnn()


def validate(model, vx, vy, vk, per_setting=False):
    import torch
    model.eval()
    num = den = 0.0
    rows = []
    with torch.no_grad():
        for x, y, k in zip(vx, vy, vk):
            out, _ = model(torch.tensor(x)[None, :, None], torch.tensor(k)[None])
            o = out[0, :, 0].numpy()
            e, r = np.sum((o[RATE:] - y[RATE:]) ** 2), np.sum(y[RATE:] ** 2)
            num, den = num + e, den + r
            rows.append(10 * np.log10(e / r))
    return (float(num / den), rows) if per_setting else float(num / den)


def train(epochs=2400, seg=RATE // 2, batch=48, tbptt=2048, warm=1000):
    import torch
    torch.manual_seed(0)
    np.random.seed(0)
    torch.set_num_threads(8)
    xs, ys, ks = _load('train')
    vx, vy, vk = _load('val')
    vx, vy = [x[:5 * RATE] for x in vx], [y[:5 * RATE] for y in vy]  # 5 s per validation setting while training
    model = _model()
    opt = torch.optim.Adam(model.parameters(), lr=1e-3)
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
        print(f'x4: resuming at epoch {start}', flush=True)
    for ep in range(start, epochs):
        model.train()
        bx, by, bk = [], [], []
        for _ in range(batch):
            j = np.random.randint(len(xs))
            s = np.random.randint(0, len(xs[j]) - seg)
            bx.append(xs[j][s:s + seg])
            by.append(ys[j][s:s + seg])
            bk.append(ks[j])
        bx = torch.tensor(np.stack(bx))[:, :, None]
        by = torch.tensor(np.stack(by))[:, :, None]
        bk = torch.tensor(np.stack(bk))
        with torch.no_grad():  # warm the state, not scored
            _, state = model(bx[:, :warm], bk)
        total = 0.0
        for s in range(warm, seg, tbptt):
            state = tuple(z.detach() for z in state)
            out, state = model(bx[:, s:s + tbptt], bk, state)
            loss = x3._esr_loss(out, by[:, s:s + tbptt])
            opt.zero_grad()
            loss.backward()
            torch.nn.utils.clip_grad_norm_(model.parameters(), 1.0)
            opt.step()
            total += loss.item()
        if ep % 20 == 19 or ep == epochs - 1:
            v = validate(model, vx, vy, vk)
            sched.step(v)  # patience counts validations (20 epochs each)
            log.append((ep + 1, total, v, time.time() - t0))
            print(f'x4: epoch {ep + 1} train {total:.4f} val ESR {10 * np.log10(v):.1f} dB ({time.time() - t0:.0f} s)',
                  flush=True)
            if v < best:
                best = v
                torch.save(model.state_dict(), CACHE / 'lstm32k.pt')
            torch.save({'model': model.state_dict(), 'opt': opt.state_dict(), 'sched': sched.state_dict(),
                        'best': best, 'log': log, 'ep': ep + 1, 'np_rng': np.random.get_state(),
                        'torch_rng': torch.get_rng_state()}, ckpt)
    (CACHE / 'train_log.json').write_text(json.dumps(log))
    ckpt.unlink(missing_ok=True)


def _load_model():
    import torch
    model = _model()
    model.load_state_dict(torch.load(CACHE / 'lstm32k.pt', map_location='cpu'))
    model.eval()
    return model


def _student(x, k, model=None):
    import torch
    model = model or _load_model()
    with torch.no_grad():
        out, _ = model(torch.tensor(x.astype(np.float32))[None, :, None],
                       torch.tensor(np.asarray(k, np.float32))[None])
    return out[0, :, 0].numpy().astype(np.float64)


def spice_refs():
    """Cached SPICE DI references (full netlist at SPICE_FS, the first 5 s of every track) at EVAL_PRESETS, in the
    48 kHz band, full scale. Run with AMP2203_FS = SPICE_FS (evaluate does). Never renders: a missing reference
    is an error (each takes hours)."""
    from . import cascade, signals, spice
    from .presets import PRESETS
    assert config.FS == SPICE_FS, config.FS
    cached = spice._cached

    def only_cached(key, compute):
        if not (config.CACHE_DIR / f'{key}.npz').exists():
            raise RuntimeError('x4: SPICE reference not cached; refusing to render it here')
        return cached(key, compute)

    spice._cached = only_cached
    texts = cascade._texts()
    out = {}
    for pid in EVAL_PRESETS:
        for sig in signals.di_signals():
            f = cascade.render_f(texts, PRESETS[pid].all_knobs(), sig)
            out[f'{pid}|{sig.id.replace("DI-", "")}'] = x3._band48(f['out'] / 60.0, config.FS)
    CACHE.mkdir(parents=True, exist_ok=True)
    np.savez(CACHE / 'spice_refs.npz', **out)


def evaluate():
    from .presets import PRESETS
    env = {**os.environ, 'AMP2203_FS': str(SPICE_FS)}
    subprocess.run([sys.executable, '-m', 'harness.x4_conditioned', 'spice-refs'], env=env, check=True)
    refs = dict(np.load(CACHE / 'spice_refs.npz'))
    tracks = x3._tracks()
    model = _load_model()
    n5 = int(x3.HOLD_HEAD_S * RATE)
    s = slice(int(0.1 * RATE), n5)  # the students start from a zero state
    rows = []
    for key, ref in refs.items():
        pid, name = key.split('|')
        knobs = PRESETS[pid].all_knobs()
        k = [knobs[n] for n in KNOB_NAMES]
        x = tracks[name][:n5]
        ref = ref[:n5]
        teach = x3._teacher(x, knobs, 'exact')[:n5]
        stud = _student(x, k, model)[:n5]
        x3s = metrics.esr_db(ref[s], x3._student(x)[:n5][s]) if pid == 'CRUNCH' else None
        rows.append((pid, name, metrics.esr_db(ref[s], teach[s]), metrics.esr_db(ref[s], stud[s]),
                     metrics.esr_db(teach[s], stud[s]), x3s))
        print(f'x4: {pid} {name}: teacher vs SPICE {rows[-1][2]:.1f} dB, student vs SPICE {rows[-1][3]:.1f} dB, '
              f'student vs teacher {rows[-1][4]:.1f} dB' + (f', X3 vs SPICE {x3s:.1f} dB' if x3s is not None else ''),
              flush=True)
    vx, vy, vk = _load('val')
    val, val_rows = validate(model, vx, vy, vk, per_setting=True)
    print(f'x4: validation (16 unseen settings) {10 * np.log10(val):.1f} dB', flush=True)
    alias = {}
    for pid in ('CRUNCH', 'HOT'):
        knobs = PRESETS[pid].all_knobs()
        k = [knobs[n] for n in KNOB_NAMES]
        alias[pid] = x3._alias_test(lambda x: _student(x, k, model), lambda x: x3._teacher(x, knobs, 'exact'))
    import torch
    sd = {n: v.clone() for n, v in torch.load(CACHE / 'lstm32k.pt', map_location='cpu').items()}
    knobs = PRESETS['CRUNCH'].all_knobs()
    kc = torch.tensor([knobs[n] for n in KNOB_NAMES])
    w = sd['rnn.weight_ih_l0']
    sd['rnn.bias_ih_l0'] = sd['rnn.bias_ih_l0'] + w[:, 1:] @ kc  # static knobs fold into the bias
    sd['rnn.weight_ih_l0'] = w[:, :1].contiguous()
    ns = x3._cpp_cost(sd)
    log = json.loads((CACHE / 'train_log.json').read_text())
    write_report(rows, val, val_rows, vk, alias, ns, log)


def write_report(rows, val, val_rows, vk, alias, ns, log):
    out = config.REPORT_DIR / 'M6'
    out.mkdir(parents=True, exist_ok=True)
    path = out / f'{datetime.date.today().strftime("%Y%m%d")}_x4_conditioned.md'
    lines = ['# M6 experiment X4: LSTM-32 with Preamp Volume, Master, Presence as inputs, vs SPICE', '',
             f'- Date {datetime.date.today().isoformat()}; teacher `ampwav --block full --j4 exact` at '
             f'{RATE * x3.TEACHER_FACTOR} Hz; tone stack 0.5; {N_TRAIN} training settings ({SEG_S:g} s each, '
             f'random DI track, offset and input gain in {x3.GAINS_DB} dB), {N_VAL} validation settings on the '
             f'held-out last {x3.HOLD_TAIL_S:g} s; student LSTM {x3.HIDDEN} on (audio, 3 knobs) at {RATE} Hz.',
             f'- SPICE: full netlist at {SPICE_FS} Hz on the held-out first {x3.HOLD_HEAD_S:g} s of every DI track. '
             f'Training: {int(log[-1][0])} epochs, {log[-1][3]:.0f} s.',
             '', '## Accuracy vs SPICE (48 kHz band, first 0.1 s excluded)', '',
             '| Preset | Track | Teacher vs SPICE | Student vs SPICE | Student vs teacher | X3 vs SPICE |',
             '|---|---|---|---|---|---|']
    for pid, name, a, b, c, d in rows:
        lines.append(f'| {pid} | {name} | {a:.1f} dB | {b:.1f} dB | {c:.1f} dB | '
                     f'{"—" if d is None else f"{d:.1f} dB"} |')
    lines += ['', f'## Unseen settings: student vs teacher ({N_VAL} validation settings, 20 s, first 1 s excluded)', '',
              f'Overall {10 * np.log10(val):.1f} dB.', '',
              '| Preamp Volume | Master | Presence | ESR |', '|---|---|---|---|']
    for k, e in sorted(zip(vk, val_rows), key=lambda r: r[1]):
        lines.append(f'| {k[0]:.3f} | {k[1]:.3f} | {k[2]:.3f} | {e:.1f} dB |')
    lines += ['', '## Aliasing (sines; power outside the harmonics of f0, relative to the total)', '',
              '| Preset | f0 | amplitude (FS) | teacher (768 kHz → 48 kHz) | student (48 kHz) |', '|---|---|---|---|---|']
    for pid, rs in alias.items():
        for f0, amp, t, s in rs:
            lines.append(f'| {pid} | {f0:.0f} Hz | {amp:g} | {t:.1f} dB | {s:.1f} dB |')
    lines += ['', '## Cost', '',
              f'Static knobs folded into the LSTM bias, same C++ kernel as X3: {ns:.1f} ns per 48 kHz sample = '
              f'{ns / 20833.3 * 100:.2f} % of one core at 48 kHz. A knob change costs one 4·{x3.HIDDEN} × 3 '
              'product.', '', '## Training log (epoch, train loss sum, validation ESR)', '']
    lines += [f'- {int(e)}: {tl:.4f}, {10 * np.log10(v):.1f} dB ({t:.0f} s)' for e, tl, v, t in log[::8] + [log[-1]]]
    path.write_text('\n'.join(lines) + '\n')
    print(f'x4: wrote {path.relative_to(config.ROOT)}')


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('step', choices=('render', 'train', 'evaluate', 'spice-refs'))
    ap.add_argument('--epochs', type=int, default=2400)
    a = ap.parse_args()
    {'render': render, 'train': lambda: train(a.epochs), 'evaluate': evaluate, 'spice-refs': spice_refs}[a.step]()


if __name__ == '__main__':
    sys.exit(main())
