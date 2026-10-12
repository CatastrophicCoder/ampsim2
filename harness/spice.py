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


_VREC_RE = re.compile(r'^Vrec_(\w+)\s+(\S+)\s+(\S+)\b.*$', re.IGNORECASE | re.MULTILINE)
PENDING_MARK = '* PENDING: '


def _filesource(name, n_pos, n_neg, file):
    return (f'A{name} %vd([{n_pos} {n_neg}]) {name}_src\n'
            f'.model {name}_src filesource (file="{file}" amploffset=[0] amplscale=[1] '
            'timeoffset=0 timescale=1 timerelative=false amplstep=false)')


def tran(testbench_text, knobs, x, nodes, recorded=None, fs=config.FS, reltol=config.SPICE_RELTOL):
    """Transient on the sample grid at fs. Returns {node: array of len(x)}.
    x: samples (volts) for the testbench's `Vin` source, or None if it has none.
    recorded: {name: samples} for its `Vrec_<name>` sources (recordings from another run).
    reltol: ngspice RELTOL for this run (None = ngspice default)."""
    text = apply_params(testbench_text, knobs)
    if PENDING_MARK in text:
        pending = [ln for ln in text.splitlines() if ln.startswith(PENDING_MARK)]
        raise SpiceError('testbench has pending values: ' + '; '.join(pending))
    recorded = {k.lower(): np.asarray(v, dtype=np.float64) for k, v in (recorded or {}).items()}
    has_vin = bool(_VIN_RE.search(text))
    if has_vin != (x is not None):
        raise SpiceError('x must be given exactly when the testbench has a "Vin" source')
    rec_names = [m.group(1).lower() for m in _VREC_RE.finditer(text)]
    if sorted(rec_names) != sorted(recorded):
        raise SpiceError(f'testbench recorded sources {sorted(rec_names)} != given {sorted(recorded)}')
    inputs = ({'vin': np.asarray(x, dtype=np.float64)} if has_vin else {}) | recorded
    lengths = {len(v) for v in inputs.values()}
    if len(lengths) != 1:
        raise SpiceError(f'input lengths differ: { {k: len(v) for k, v in inputs.items()} }')
    n = lengths.pop()
    dt = 1.0 / fs
    # The filesource does not create breakpoints, so ngspice's internal steps drift off the sample grid
    # and the input is interpolated twice (filesource, then `option interp`), a low-pass of about
    # 1.3e-3 at 5 kHz. A dummy triangle with a corner at every sample forces a time point on every
    # sample instant; the input is then exact (DECISIONS D-014).
    brk = (f'Vharness_brk harness_brk 0 PULSE(0 1 0 {dt:.17g} {dt:.17g} 0 {2 * dt:.17g})\n'
           'Rharness_brk harness_brk 0 1k')
    if has_vin:
        text = _VIN_RE.sub(lambda m: _filesource('harness_in', m.group(1), m.group(2), 'in_vin.txt'), text,
                           count=1)
    text = _VREC_RE.sub(lambda m: _filesource(f'harness_rec_{m.group(1).lower()}', m.group(2), m.group(3),
                                              f'in_{m.group(1).lower()}.txt'), text)
    text = _strip_end(text) + '\n' + brk + '\n'
    if reltol is not None:
        text += f'.options reltol={reltol!r}\n'
    vecs = [_vec(v) for v in nodes]

    def compute():
        t = np.arange(n + 2) * dt          # hold points past the end (the last samples need them)
        files = {}
        for name, v in inputs.items():
            data = np.r_[v, v[-1], v[-1]]
            files[f'in_{name}.txt'] = '\n'.join(f'{a:.17g} {b:.17g}' for a, b in zip(t, data)) + '\n'
        # stop half a sample past the last one so rounding cannot drop the final output point
        control = (f'option interp\ntran {dt:.17g} {(n - 0.5) * dt:.17g} 0 {dt:.17g}\n'
                   f'write tran.raw {" ".join(vecs)}')
        _, raws = _run(text, control, files)
        raw = raws['tran.raw']
        if len(raw['time']) < n or np.max(np.abs(raw['time'][:n] - np.arange(n) * dt)) > 1e-3 * dt:
            raise SpiceError(f'SPICE time grid does not match the {n}-sample input grid')
        return {v: raw[v.lower()][:n] for v in vecs}

    digests = {k: hashlib.sha256(v.tobytes()).hexdigest() for k, v in sorted(inputs.items())}
    key = _key('tran', text, digests, vecs, fs, ngspice_version())
    res = _cached(key, compute)
    return {node: res[v] for node, v in zip(nodes, vecs)}


def op(testbench_text, knobs, nodes):
    """DC operating point. Returns {node: value}."""
    text = apply_params(testbench_text, knobs)
    vecs = [_vec(v) for v in nodes]

    def compute():
        # write every vector (long `write` lines break ngspice's parser); device quantities such as
        # @r4[i] must be saved explicitly before the analysis
        dev = [v for v in vecs if v.startswith('@')]
        save = f'save all {" ".join(dev)}\n' if dev else ''
        _, raws = _run(text, f'{save}op\nwrite op.raw')
        raw = raws['op.raw']
        name = lambda v: f'i({v.lower()})' if v.startswith('@') else v.lower()  # noqa: E731 (raw naming)
        missing = [v for v in vecs if name(v) not in raw]
        if missing:
            raise SpiceError(f'op: vectors not found: {missing}')
        return {v: raw[name(v)] for v in vecs}

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


_AC_RE = re.compile(r'^([VI]\S*\s[^\n]*?)\s+AC\s+\S+', re.IGNORECASE | re.MULTILINE)


def impedance(testbench_text, knobs, node, observe=(), fstart=1e-3, fstop=1e5, ppd=24):
    """Small-signal impedance at `node`: 1 A AC injected into `node` with every other independent
    source AC-grounded, at the DC operating point. Returns (f, {node: Z}) where Z[o] = v(o) / 1 A
    (self impedance for `node`, transimpedance for the `observe` nodes). Frequencies on a decade grid
    starting at `fstart` (powers of ten fall exactly on grid points)."""
    text = _AC_RE.sub(r'\1', apply_params(testbench_text, knobs))
    text = _strip_end(text) + f'\nIinj_zth 0 {node} DC 0 AC 1\n'
    nodes = [node, *observe]
    vecs = ' '.join(_vec(n) for n in nodes)

    def compute():
        _, raws = _run(text, f'ac dec {ppd} {fstart:g} {fstop:g}\nwrite ac.raw {vecs}')
        raw = raws['ac.raw']
        return {'f': raw['frequency'].real, **{n: raw[_vec(n).lower()] for n in nodes}}

    res = _cached(_key('impedance', text, nodes, fstart, fstop, ppd, ngspice_version()), compute)
    return res['f'], {n: res[n] for n in nodes}
