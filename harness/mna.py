"""MNA reference for R-type adaptors (WDF_DESIGN §5), the numpy counterpart of engine/src/mna/.

A port network is a list of ports (node+, node−) between named nodes ('0' is ground). Each port n of an
adaptor is modelled as a Thevenin source (voltage a_n in series with R_n, + terminal at node+), with
v_n = v(node+) − v(node−) and b_n = 2·v_n − a_n.

The linear solves use exact rational arithmetic (`fractions`), so every result is the correctly rounded
value of the exact one: the reference for the engine's double-double implementation (PROGRESS Q11).

    python -m harness.mna --fixtures     # write tests/data/rtype_fixtures.txt for the C++ cross-check
    python -m harness.mna --check        # exit 1 if the committed fixtures are out of date
"""
import argparse
import sys
from dataclasses import dataclass
from fractions import Fraction

import numpy as np

from . import config

FIXTURES = config.ROOT / 'tests' / 'data' / 'rtype_fixtures.txt'


@dataclass(frozen=True)
class PortNetwork:
    ports: tuple            # ((node+, node−), ...)

    @property
    def nodes(self):
        """Non-ground nodes in order of first appearance."""
        seen = []
        for p in self.ports:
            for n in p:
                if n != '0' and n not in seen:
                    seen.append(n)
        return seen

    def incidence(self):
        """A[k, n] = +1 at node+ and −1 at node− of port k (ground column dropped)."""
        idx = {n: i for i, n in enumerate(self.nodes)}
        a = np.zeros((len(self.ports), len(idx)))
        for k, (p, m) in enumerate(self.ports):
            if p != '0':
                a[k, idx[p]] += 1.0
            if m != '0':
                a[k, idx[m]] -= 1.0
        return a


def _solve_exact(Y, rhs):
    """Gaussian elimination in rationals; Y is a list of lists, rhs a list of columns (lists)."""
    n = len(Y)
    M = [row[:] + [col[i] for col in rhs] for i, row in enumerate(Y)]
    for k in range(n):
        p = next((i for i in range(k, n) if M[i][k] != 0), None)
        if p is None:
            raise np.linalg.LinAlgError('singular nodal matrix (floating node?)')
        M[k], M[p] = M[p], M[k]
        for i in range(k + 1, n):
            f = M[i][k] / M[k][k]
            if f:
                M[i] = [x - f * y for x, y in zip(M[i], M[k])]
    X = [[Fraction(0)] * n for _ in rhs]
    for c in range(len(rhs)):
        for i in range(n - 1, -1, -1):
            s = M[i][n + c] - sum(M[i][j] * X[c][j] for j in range(i + 1, n))
            X[c][i] = s / M[i][i]
    return X


def _nodal(net, G):
    """Nodal matrix Y = Aᵀ·diag(G)·A in rationals (G: list of Fractions)."""
    A = net.incidence().astype(int).tolist()
    n = len(net.nodes)
    Y = [[Fraction(0)] * n for _ in range(n)]
    for k, row in enumerate(A):
        nz = [(i, x) for i, x in enumerate(row) if x]
        for i, xi in nz:
            for j, xj in nz:
                Y[i][j] += G[k] * xi * xj
    return A, Y


def _port_voltages_exact(net, R, excitations):
    """Exact port voltages for each excitation vector a (every port a Thevenin source (a_k, R_k))."""
    G = [1 / Fraction(float(r)) for r in R]
    A, Y = _nodal(net, G)
    rhs = [[sum(A[k][i] * G[k] * Fraction(float(a[k])) for k in range(len(A))) for i in range(len(Y))]
           for a in excitations]
    return [[sum(A[k][i] * v[i] for i in range(len(v))) for k in range(len(A))]
            for v in _solve_exact(Y, rhs)]


def port_voltages(net, R, a):
    """Port voltages with every port a Thevenin source (a_k, R_k): nodal analysis of the Norton form."""
    return np.array([float(x) for x in _port_voltages_exact(net, R, [a])[0]])


def scattering(net, R):
    """Definition-based scattering matrix (WDF_DESIGN §5.1): column j from a = e_j."""
    n = len(net.ports)
    cols = _port_voltages_exact(net, R, [[1 if k == j else 0 for k in range(n)] for j in range(n)])
    return np.array([[float(2 * cols[j][i] - (1 if i == j else 0)) for j in range(n)] for i in range(n)])


def up_port_resistance(net, R, up):
    """Resistance seen into the network at port `up` with the other ports as their resistances
    (WDF_DESIGN §5.2): 1 A injected at node+ of `up`, its own port resistance removed."""
    G = [Fraction(0) if k == up else 1 / Fraction(float(r)) for k, r in enumerate(R)]
    A, Y = _nodal(net, G)
    v = _solve_exact(Y, [[Fraction(A[up][i]) for i in range(len(Y))]])[0]
    return float(sum(A[up][i] * v[i] for i in range(len(v))))


def adapted(net, R, up):
    """Port resistances with the up-port made reflection-free, and the scattering matrix."""
    R = np.asarray(R, dtype=float).copy()
    R[up] = up_port_resistance(net, R, up)
    return R, scattering(net, R)


# ---- B4 tone stack (CIRCUIT.md §1 B4, netlist lines C10 .. RVR2b) ----------------------------------------

B4_PORTS = (('k2b', '0'),                    # 0: up-port (J3 source side, adapted)
            ('k2b', 'tt'), ('k2b', 'nD'),    # 1: C10, 2: R15
            ('nD', 'nE'), ('nD', 'mw'),      # 3: C11, 4: C12
            ('tt', 'tw'), ('tw', 'nE'),      # 5: VR3a (treble top), 6: VR3b
            ('nE', 'nbb'),                   # 7: VR5 (bass rheostat)
            ('nbb', 'mw'), ('mw', '0'),      # 8: VR4a (middle top), 9: VR4b
            ('tw', 'y'), ('y', '0'))         # 10: VR2a (master top), 11: VR2b
B4 = PortNetwork(B4_PORTS)
C10, C11, C12 = 470e-12, 0.022e-6, 0.022e-6


def atap(x):
    return max((81.0 ** x - 1.0) / 80.0, 1e-4)


def ltap(x):
    return max(min(x, 0.9999), 1e-4)


def b4_resistances(treble, middle, bass, master, fs=config.FS):
    """Port resistances of the B4 down-ports (index 1..11) as in the netlist; port 0 is left at 1."""
    cap = lambda c: 1.0 / (2.0 * c * fs)  # noqa: E731  (bilinear, WDF_DESIGN §2)
    return np.array([1.0, cap(C10), 33e3, cap(C11), cap(C12),
                     220e3 * (1 - ltap(treble)), 220e3 * ltap(treble),
                     1e6 * atap(bass) + 1,
                     22e3 * (1 - ltap(middle)), 22e3 * ltap(middle),
                     1e6 * (1 - atap(master)) + 1, 1e6 * atap(master) + 1])


# ---- fixtures for the C++ cross-check --------------------------------------------------------------------

FIXTURE_CASES = (
    ('b4_default', dict(treble=0.5, middle=0.5, bass=0.5, master=0.3)),
    ('b4_all_min', dict(treble=0.0, middle=0.0, bass=0.0, master=0.0)),
    ('b4_all_max', dict(treble=1.0, middle=1.0, bass=1.0, master=1.0)),
    ('b4_mixed', dict(treble=0.25, middle=1.0, bass=0.75, master=0.5)),
)


def random_network(seed=7):
    """A bridge network with 6 nodes and 9 ports, random resistances over 1 Ω–1 MΩ (seeded)."""
    rng = np.random.default_rng(seed)
    ports = (('a', '0'), ('a', 'b'), ('a', 'c'), ('b', 'c'), ('b', 'd'), ('c', 'e'), ('d', 'e'),
             ('d', '0'), ('e', '0'))
    return PortNetwork(ports), 10.0 ** rng.uniform(0, 6, len(ports))


def fixtures_text():
    lines = ['# R-type scattering fixtures: GENERATED by harness/mna.py (--fixtures), do not edit',
             '# per case: "case <name> <ports> <up>", "R" line (adapted port resistances), then S rows',
             f'# B4 port resistances at fs = {config.FS} Hz']
    cases = [(name, B4, b4_resistances(**k)) for name, k in FIXTURE_CASES]
    net, r = random_network()
    cases.append(('random_bridge', net, r))
    for name, net, r in cases:
        R, S = adapted(net, r, 0)
        lines.append(f'case {name} {len(net.ports)} 0')
        lines.append('R ' + ' '.join(f'{x:.17g}' for x in R))
        lines += [' '.join(f'{x:.17g}' for x in row) for row in S]
    return '\n'.join(lines) + '\n'


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--fixtures', action='store_true')
    ap.add_argument('--check', action='store_true')
    a = ap.parse_args()
    text = fixtures_text()
    if a.check:
        if not FIXTURES.exists() or FIXTURES.read_text() != text:
            print(f'{FIXTURES.relative_to(config.ROOT)} out of date (run python -m harness.mna --fixtures)')
            sys.exit(1)
    elif a.fixtures:
        FIXTURES.write_text(text)
        print(f'wrote {FIXTURES.relative_to(config.ROOT)}')
    else:
        ap.print_help()


if __name__ == '__main__':
    main()
