"""ngspice runner for testbenches: transient with a recorded input (XSPICE filesource), operating point
and AC analysis. Results are cached in harness/cache/, keyed by a hash of everything that affects them
(VERIFICATION.md §2).

Testbench convention: the block's input source is a line `Vin <node> <node> DC 0 AC 1`. For transient
runs it is replaced by a filesource driven with the signal; for op and ac it is kept as is.
"""
import functools
import hashlib
import json
import re
import subprocess
import tempfile
from pathlib import Path

import numpy as np

from . import config

_VIN_RE = re.compile(r'^Vin\s+(\S+)\s+(\S+)\b.*$', re.IGNORECASE | re.MULTILINE)


class SpiceError(RuntimeError):
    pass


@functools.lru_cache(maxsize=1)
def ngspice_version():
    out = subprocess.run(['ngspice', '-v'], capture_output=True, text=True).stdout
    m = re.search(r'ngspice-(\S+)', out)
    return m.group(1) if m else 'unknown'


def apply_params(text, knobs):
    """Substitute knob rotations (engine names) into .param lines; knobs a testbench lacks are ignored."""
    for name, value in knobs.items():
        p = config.KNOBS.get(name, name)
        text = re.sub(rf'(\.param[^\n]*\b{p}=)\S+', rf'\g<1>{value!r}', text)
    return text.replace('.include tubes.lib', f'.include {config.TUBES}')


def _strip_end(text):
    text = text.rstrip()
    return text[: -len('.end')].rstrip() if text.lower().endswith('.end') else text


def read_raw(path):
    """Reads an ngspice binary raw file. Returns {vector name: array} (complex for AC)."""
    b = Path(path).read_bytes()
    i = b.index(b'Binary:\n')
    hdr = b[:i].decode(errors='replace').splitlines()
    nvars = npts = None
    names, complex_data = [], False
    for k, line in enumerate(hdr):
        if line.startswith('No. Variables:'):
            nvars = int(line.split(':')[1])
        elif line.startswith('No. Points:'):
            npts = int(line.split(':')[1])
        elif line.startswith('Flags:'):
            complex_data = 'complex' in line
        elif line.startswith('Variables:'):
            for v in hdr[k + 1: k + 1 + nvars]:
                names.append(v.split()[1].lower())
    data = np.frombuffer(b[i + 8:], dtype='<f8')
    if complex_data:
        data = data.reshape(npts, nvars, 2)
        data = data[..., 0] + 1j * data[..., 1]
    else:
        data = data.reshape(npts, nvars)
    return {n: data[:, j].copy() for j, n in enumerate(names)}


def _run(netlist_text, control, files=None):
    """Runs ngspice in a temp dir; returns (stdout, {raw name: parsed raw}) for every *.raw written."""
    with tempfile.TemporaryDirectory(prefix='amp2203_spice_') as d:
        d = Path(d)
        for name, content in (files or {}).items():
            (d / name).write_text(content)
        (d / 'sim.cir').write_text(_strip_end(netlist_text) + '\n.control\nset filetype=binary\n' + control +
                                   '\n.endc\n.end\n')
        r = subprocess.run(['ngspice', '-b', 'sim.cir'], cwd=d, capture_output=True, text=True)
        raws = {p.name: read_raw(p) for p in d.glob('*.raw')}
        if r.returncode != 0 or not raws:
            raise SpiceError(f'ngspice failed (exit {r.returncode}):\n{(r.stdout + r.stderr)[-3000:]}')
        return r.stdout, raws


def _key(*parts):
    h = hashlib.sha256()
    for p in parts:
        h.update(p if isinstance(p, bytes) else json.dumps(p, sort_keys=True).encode())
        h.update(b'\0')
    return h.hexdigest()[:24]


def _cached(key, compute):
    path = config.CACHE_DIR / f'{key}.npz'
    if path.exists():
        with np.load(path) as z:
            return {k: z[k] for k in z.files}
    result = compute()
    config.CACHE_DIR.mkdir(parents=True, exist_ok=True)
    tmp = path.with_suffix('.tmp.npz')
    np.savez(tmp, **result)
    tmp.replace(path)
    return result


def _vec(node):
    return node if '(' in node or '[' in node else f'v({node})'


def tran(testbench_text, knobs, x, nodes, fs=config.FS, reltol=config.SPICE_RELTOL):
    """Transient driven by samples x (volts) at fs. Returns {node: array of len(x)} on the sample grid.
    reltol: ngspice RELTOL for this run (None = ngspice default)."""
    text = apply_params(testbench_text, knobs)
    if not _VIN_RE.search(text):
        raise SpiceError('testbench has no "Vin <n+> <n-> ..." input source')
    x = np.asarray(x, dtype=np.float64)
    n = len(x)
    dt = 1.0 / fs
    # The filesource does not create breakpoints, so ngspice's internal steps drift off the sample grid
    # and the input is interpolated twice (filesource, then `option interp`), a low-pass of about
    # 1.3e-3 at 5 kHz. A dummy triangle with a corner at every sample forces a time point on every
    # sample instant; the input is then exact (DECISIONS D-014).
    text = _VIN_RE.sub(
        lambda m: f'Ain %vd([{m.group(1)} {m.group(2)}]) harness_insrc\n'
                  '.model harness_insrc filesource (file="input.txt" amploffset=[0] amplscale=[1] '
                  'timeoffset=0 timescale=1 timerelative=false amplstep=false)\n'
                  f'Vharness_brk harness_brk 0 PULSE(0 1 0 {dt:.17g} {dt:.17g} 0 {2 * dt:.17g})\n'
                  'Rharness_brk harness_brk 0 1k', text, count=1)
    if reltol is not None:
        text = _strip_end(text) + f'\n.options reltol={reltol!r}\n'
    vecs = [_vec(v) for v in nodes]

    def compute():
        t = np.arange(n + 2) * dt          # hold points past the end (the last samples need them)
        data = np.r_[x, x[-1], x[-1]]
        src = '\n'.join(f'{a:.17g} {b:.17g}' for a, b in zip(t, data)) + '\n'
        # stop half a sample past the last one so rounding cannot drop the final output point
        control = (f'option interp\ntran {dt:.17g} {(n - 0.5) * dt:.17g} 0 {dt:.17g}\n'
                   f'write tran.raw {" ".join(vecs)}')
        _, raws = _run(text, control, {'input.txt': src})
        raw = raws['tran.raw']
        if len(raw['time']) < n or np.max(np.abs(raw['time'][:n] - np.arange(n) * dt)) > 1e-3 * dt:
            raise SpiceError(f'SPICE time grid does not match the {n}-sample input grid')
        return {v: raw[v.lower()][:n] for v in vecs}

    key = _key('tran', text, x.tobytes(), vecs, fs, ngspice_version())
    res = _cached(key, compute)
    return {node: res[v] for node, v in zip(nodes, vecs)}


def op(testbench_text, knobs, nodes):
    """DC operating point. Returns {node: value}."""
    text = apply_params(testbench_text, knobs)
    vecs = [_vec(v) for v in nodes]

    def compute():
        _, raws = _run(text, f'op\nwrite op.raw {" ".join(vecs)}')
        return {v: raws['op.raw'][v.lower()] for v in vecs}

    res = _cached(_key('op', text, vecs, ngspice_version()), compute)
    return {node: float(res[v][0]) for node, v in zip(nodes, vecs)}


def ac(testbench_text, knobs, out_node, in_node, fstart=20.0, fstop=20000.0, ppd=48):
    """Small-signal transfer v(out)/v(in). Returns (f, complex H)."""
    text = apply_params(testbench_text, knobs)
    expr = f'{_vec(out_node)}/{_vec(in_node)}'

    def compute():
        _, raws = _run(text, f'ac dec {ppd} {fstart:g} {fstop:g}\nlet h = {expr}\nwrite ac.raw h')
        raw = raws['ac.raw']
        return {'f': raw['frequency'].real, 'h': raw['h']}

    res = _cached(_key('ac', text, expr, fstart, fstop, ppd, ngspice_version()), compute)
    return res['f'], res['h']
