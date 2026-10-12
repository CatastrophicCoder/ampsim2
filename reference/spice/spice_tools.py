"""Helpers for driving ngspice on the JCM800 2203 reference netlist.

Usage examples:
    from spice_tools import op, ac, tran, thd_power
    op(['v(p1a)', 'v(ct)'], vol=0.8)                        # DC operating point
    f, h = ac('db(v(tw)/v(k2b))', bass=1.0)                 # AC sweep of any expression
    t, (vin, vout) = tran(0.3, 220, nodes=('v(in)', 'v(out)'), vol=0.8, master=0.3)

Knob/model parameters (see .param lines in jcm800_2203.cir) are passed as keyword
arguments and substituted textually into a temporary copy of the netlist.
Requires ngspice (macOS: `brew install ngspice`) and numpy.
"""
import os
import re
import subprocess
import tempfile

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
NETLIST = os.path.join(HERE, 'jcm800_2203.cir')
TUBES = os.path.join(HERE, 'tubes.lib')


def netlist(base=None, **params):
    """Return netlist text with .param values overridden and the tube library path made absolute."""
    txt = open(NETLIST).read() if base is None else base
    txt = txt.replace('.include tubes.lib', f'.include {TUBES}')
    for k, v in params.items():
        txt, n = re.subn(rf'(\.param[^\n]*\b{k}=)\S+', rf'\g<1>{v}', txt)
        if n == 0:
            raise KeyError(f'unknown .param {k}')
    return txt


def run(control, base=None, **params):
    """Run ngspice in batch mode with the given .control block body; return stdout+stderr."""
    body = netlist(base, **params).rstrip()
    if body.endswith('.end'):
        body = body[: -len('.end')]
    with tempfile.TemporaryDirectory() as d:
        path = os.path.join(d, 'sim.cir')
        with open(path, 'w') as fh:
            fh.write(body + '\n.control\n' + control + '\n.endc\n.end\n')
        r = subprocess.run(['ngspice', '-b', path], capture_output=True, text=True, cwd=d)
    return r.stdout + r.stderr


def _wrdata(control, cols, base=None, **params):
    with tempfile.TemporaryDirectory() as d:
        f = os.path.join(d, 'out.dat')
        out = run(control + f'\nwrdata {f} ' + ' '.join(cols), base, **params)
        try:
            return np.loadtxt(f)
        except Exception as e:
            raise RuntimeError(out[-3000:]) from e


def op(nodes, base=None, **params):
    """DC operating point. nodes: list like ['v(p1a)', 'i(vht)', '@r35[i]']. Returns dict."""
    out = run('op\n' + '\n'.join(f'print {n}' for n in nodes), base, **params)
    vals = {}
    for line in out.splitlines():
        m = re.match(r'\s*(\S+)\s*=\s*([-+0-9.eE]+)', line)
        if m:
            vals[m.group(1)] = float(m.group(2))
    if not vals:
        raise RuntimeError(out[-3000:])
    return vals


def ac(expr, fstart=10, fstop=40e3, ppd=40, base=None, **params):
    """Small-signal AC sweep. expr is an ngspice expression, e.g. 'db(v(out)/v(in))'."""
    d = _wrdata(f'ac dec {ppd} {fstart} {fstop}\nlet h={expr}', ['h'], base, **params)
    return d[:, 0], d[:, 1]


def tran(amp, freq, tstop=0.12, step='4u', nodes=('v(in)', 'v(out)'), base=None, **params):
    """Transient with a sine input of amplitude amp (V peak) at freq (Hz).
    Uses a fixed output timestep via 'linearize'. Returns t, [column arrays]."""
    txt = open(NETLIST).read() if base is None else base
    txt = txt.replace('Vin in 0 DC 0 AC 1', f'Vin in 0 DC 0 AC 1 SIN(0 {amp} {freq})')
    d = _wrdata(f'option interp\ntran {step} {tstop}\nlinearize', list(nodes), txt, **params)
    t = d[:, 0]
    return t, [d[:, 2 * i + 1] for i in range(len(nodes))]


def thd_power(t, y, freq, tlast=0.04, rl=16.0):
    """Mean power into rl and THD (%) over the last tlast seconds (Hann-windowed FFT, 10 harmonics)."""
    m = t > t[-1] - tlast
    t, y = t[m], y[m] - y[m].mean()
    w = np.hanning(len(y))
    Y = np.abs(np.fft.rfft(y * w))
    fr = np.fft.rfftfreq(len(y), t[1] - t[0])
    h = []
    for k in range(1, 11):
        i = int(np.argmin(np.abs(fr - k * freq)))
        h.append(Y[max(i - 2, 0): i + 3].max())
    thd = np.sqrt(sum(x * x for x in h[1:])) / h[0]
    return float(np.mean(y * y) / rl), float(thd * 100)


if __name__ == '__main__':
    nodes = ['v(ct)', 'v(nscr)', 'v(bpx)', 'v(bpa)', 'v(nbias)', 'v(p1a)', 'v(k1a)', 'v(p1b)', 'v(k1b)',
             'v(p2a)', 'v(k2a)', 'v(k2b)', 'v(p3a)', 'v(p3b)', 'v(k3)', 'v(nf)', 'v(ng)']
    for k, v in op(nodes).items():
        print(f'{k:10s} {v:10.3f} V')
