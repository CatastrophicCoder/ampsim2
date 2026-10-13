"""Generates harness/testbenches/*.cir from the reference netlist (VERIFICATION.md §1–2, SPEC §4.2).
Generated files are committed so they can be read and diffed; never edit them by hand.

    python -m harness.testbench_gen          # regenerate all testbenches (runs ngspice for DC values)
    python -m harness.testbench_gen --check  # exit 1 if a committed testbench is out of date

Conventions:
- `Vin <node> 0 DC <v> AC 1`: the block input (a J-Thevenin source voltage, or the guitar input).
  Transient runs replace it with the signal; DC <v> is the F operating-point value, so `op` on the
  testbench reproduces F.
- `Vrec_<name> <node> 0 DC <v>`: a recorded waveform from another run (supply rails, NFB, PI outputs).
  Transient runs replace it with the recording; DC <v> is the F operating-point value.
- `Vsup_<node>`, `Isink_<node>`: constant supply rails and preamp load currents from the F operating
  point (default parameters).
- Junction resistances come from harness/junctions.py; unmeasured ones are written with a
  `PENDING` marker and transient runs refuse such a testbench.
"""
import argparse
import functools
import hashlib
import re
import sys
from dataclasses import dataclass, field

from . import config, netlist, spice
from .junctions import RTH
from .spice import PENDING_MARK

# Partition of the reference netlist (SPEC §4.2). Every element belongs to exactly one block.
PARTITION = {
    'B12': 'Vin R2 R3 X1a R1 C1 R4 C3 R5 C4 C5 RVR1a RVR1b X1b R6 R7',   # V1a + V1b, joint root (D-017)
    'B3': 'C7 R10 C8 R11 X2a R9 R12 X2b R13',
    'B4': 'C10 R15 C11 C12 RVR3a RVR3b RVR5 RVR4a RVR4b RVR2a RVR2b',
    'B5': 'C9 R17 R16 R19 C15 R20 R23 C17 RVR6 R22 X3a X3b R18 R21',
    'B6': ('C14 C16 C18 R24 R25 C19 Vc20 R27 R26 RRV1 R31 R33 R32 R34 R35 R37 R36 R38 X4 X5 X6 X7 '
           'Lp1 Lp2 Ls Rwp1 Rwp2 Rws K1 K2 K3 Rload'),
    'B7': 'Vht Rsrc C23 Lch Rch C22 R29 C21b R8 C21a',
}

# J4 (master wiper) DC resistance with the J3 source shorted and capacitors open (WDF_DESIGN §6):
# VR2 bottom || (VR2 top + VR3 bottom + VR5 + VR4), from the same expressions as the netlist.
RTH_J4_EXPR = ('{1/(1/(1Meg*atap(master)+1) + 1/((1Meg*(1-atap(master))+1) + 220k*ltap(treble) '
               '+ (1Meg*atap(bass)+1) + 22k))}')


def _elements(*blocks):
    return [n.lower() for b in blocks for n in PARTITION[b].split()]


@functools.lru_cache(maxsize=1)
def reference():
    return netlist.parse(config.NETLIST.read_text())


def check_partition(ref):
    names = [e.key for e in ref.elements]
    assigned = _elements(*PARTITION)
    missing = sorted(set(names) - set(assigned))
    unknown = sorted(set(assigned) - set(names))
    dup = sorted({n for n in assigned if assigned.count(n) > 1})
    if missing or unknown or dup:
        raise ValueError(f'partition does not match the netlist: missing {missing}, unknown {unknown}, '
                         f'duplicated {dup}')


def _with_ammeter(text, element, node, name):
    """Inserts a 0 V source between `node` and the given pin node of `element` (measurement copy only)."""
    pat = re.compile(rf'^({re.escape(element)}\s.*?)\b{re.escape(node)}\b', re.MULTILINE | re.IGNORECASE)
    new, n = pat.subn(rf'\g<1>{node}_{name}', text, count=1)
    if n != 1:
        raise ValueError(f'cannot attach ammeter to {element}.{node}')
    return new.rstrip() + f'\nV{name} {node} {node}_{name} 0\n'


@functools.lru_cache(maxsize=1)
def f_operating_point():
    """F operating point at default parameters: all node voltages plus the preamp supply currents."""
    ref = reference()
    text = config.NETLIST.read_text()
    text = _with_ammeter(text, 'X2b', 'bpX', 'am_v2b_plate')
    nodes = ref.nodes()
    vals = spice.op(text, {}, nodes + ['@r4[i]', '@r7[i]', '@r12[i]', 'i(vam_v2b_plate)'])
    v = {n: vals[n] for n in nodes}
    i_a = vals['@r4[i]'] + vals['@r7[i]']                 # bpA -> V1a, V1b plates
    i_x = vals['@r12[i]'] + vals['i(vam_v2b_plate)']      # bpX -> V2a plate load, V2b plate
    return v, {'bpa': i_a, 'bpx': i_x}


@dataclass
class Spec:
    name: str
    blocks: tuple
    notes: list
    inserts: list = field(default_factory=list)   # (comment, line) pairs
    junctions: tuple = ()                          # RTH keys used


def _g(v):
    return f'{v:.9g}'


def _specs(v, i_pre):
    rth = lambda k: f'{{rth_{k}}}'  # noqa: E731
    return [
        Spec('b12', ('B12',), ['B12 StageV1aV1b (V1a and V1b, joint root). Input: guitar source at node in.',
                               'Output: p1b (open). Supply bpA: constant from F op.'],
             [('supply', f'Vsup_bpA bpA 0 DC {_g(v["bpa"])}')]),
        Spec('b3', ('B3',), ['B3 StageV2aV2b. Input: J2 Thevenin source (v_oc(p1b), R_th2) into node p1b.',
                             'Output: k2b (open). Supply bpX: constant from F op.'],
             [('J2', f'Vin j2s 0 DC {_g(v["p1b"])} AC 1'), ('J2', f'Rth_j2 j2s p1b {rth("j2")}'),
              ('supply', f'Vsup_bpX bpX 0 DC {_g(v["bpx"])}')], ('j2',)),
        Spec('b4', ('B4',), ['B4 ToneStackMaster. Input: J3 Thevenin source (v_oc(k2b), R_th3) into node k2b.',
                             'Output: y (open, master wiper).'],
             [('J3', f'Vin j3s 0 DC {_g(v["k2b"])} AC 1'), ('J3', f'Rth_j3 j3s k2b {rth("j3")}')], ('j3',)),
        Spec('b34', ('B3', 'B4'),
             ['B34 StageV2aV2bToneStack: B3 and B4 joined in one tree (no J3; DECISIONS D-033).',
              'Input: J2 Thevenin source (v_oc(p1b), R_th2) into node p1b. Output: y (open, master wiper).',
              'Supply bpX: constant from F op.'],
             [('J2', f'Vin j2s 0 DC {_g(v["p1b"])} AC 1'), ('J2', f'Rth_j2 j2s p1b {rth("j2")}'),
              ('supply', f'Vsup_bpX bpX 0 DC {_g(v["bpx"])}')], ('j2',)),
        Spec('b123', ('B12', 'B3'),
             ['Junction reference for J2: B12 and B3 joined directly (no J-Thevenin source), for the SPICE',
              'partition-error study of J2 (harness/partition.py). Input: guitar source at node in.',
              'Output: k2b (open). Supplies bpA, bpX: constant from F op.'],
             [('supply', f'Vsup_bpA bpA 0 DC {_g(v["bpa"])}'),
              ('supply', f'Vsup_bpX bpX 0 DC {_g(v["bpx"])}')]),
        Spec('b56', ('B5', 'B6', 'B7'),
             ['B56+B7: phase inverter, power stage and supply with instantaneous NFB (VERIFICATION §1).',
              'Input: J4 Thevenin source (v_oc(y), R_th4) into node y; R_th4 = DC formula (WDF_DESIGN §6).',
              'Preamp loads on bpA / bpX: constant current sinks from F op (recorded waveforms in M4).'],
             [('J4', f'.param rth_j4={RTH_J4_EXPR}'),
              ('J4', f'Vin j4s 0 DC {_g(v["y"])} AC 1'), ('J4', 'Rth_j4 j4s y {rth_j4}'),
              ('preamp load', f'Isink_bpA bpA 0 DC {_g(i_pre["bpa"])}'),
              ('preamp load', f'Isink_bpX bpX 0 DC {_g(i_pre["bpx"])}')]),
        Spec('b5open', ('B5',),
             ['B5-open: phase inverter with R22 driven by the recorded speaker voltage (ideal source).',
              'Input: J4 Thevenin source into node y. Outputs: p3a, p3b (open).',
              'Recorded sources (from a B56 run): out, bpX. DC values from F op.'],
             [('J4', f'.param rth_j4={RTH_J4_EXPR}'),
              ('J4', f'Vin j4s 0 DC {_g(v["y"])} AC 1'), ('J4', 'Rth_j4 j4s y {rth_j4}'),
              ('recorded', f'Vrec_out out 0 DC {_g(v["out"])}'),
              ('recorded', f'Vrec_bpx bpX 0 DC {_g(v["bpx"])}')]),
        Spec('b6', ('B6',),
             ['B6 PowerStage driven by the recorded PI open-circuit outputs through R_thA / R_thB.',
              'No Vin: both inputs are recorded sources (from a B56 / B5-open run). Output: out.',
              'Recorded supply rails (from a B56 run): ct, nScr. DC values from F op.'],
             [('J5a', f'Vrec_p3a j5a 0 DC {_g(v["p3a"])}'), ('J5a', f'Rth_j5a j5a p3a {rth("j5a")}'),
              ('J5b', f'Vrec_p3b j5b 0 DC {_g(v["p3b"])}'), ('J5b', f'Rth_j5b j5b p3b {rth("j5b")}'),
              ('recorded', f'Vrec_ct ct 0 DC {_g(v["ct"])}'),
              ('recorded', f'Vrec_nscr nScr 0 DC {_g(v["nscr"])}')], ('j5a', 'j5b')),
    ]


def _header(name, source_text, notes):
    sha = hashlib.sha256(source_text.encode()).hexdigest()[:16]
    lines = [f'* harness testbench "{name}" -- GENERATED by harness/testbench_gen.py, do not edit',
             f'* source: reference/spice/jcm800_2203.cir (sha256 {sha})']
    lines += [f'* {n}' for n in notes]
    return '\n'.join(lines) + '\n'


def gen_full(ref_text):
    """Reference F: the netlist unchanged (only the title line is replaced by the header)."""
    body = ref_text.split('\n', 1)[1]
    return _header('full', ref_text, ['Reference F: identical to the reference netlist.',
                                      'Input: Vin in 0 (replaced by a filesource for transient runs).']) + body


def gen_block(spec, ref, ref_text, rth=RTH):
    by = ref.by_name()
    out = [_header(spec.name, ref_text, spec.notes).rstrip()]
    for k in spec.junctions:
        if rth[k] is None:
            out.append(f'{PENDING_MARK}R_th for {k} not measured yet (M0 step 5); placeholder 1 ohm')
    out += list(ref.directives)
    for k in spec.junctions:
        out.append(f'.param rth_{k}={rth[k]!r}' if rth[k] is not None else f'.param rth_{k}=1')
    for b in spec.blocks:
        out.append(f'\n*** {b} (from the reference netlist)')
        out += [by[n].line for n in _elements(b)]
    last = None
    for comment, line in spec.inserts:
        if comment != last:
            out.append(f'\n*** {comment} (inserted by the harness)')
            last = comment
        out.append(line)
    return '\n'.join(out) + '\n'


def generate(rth=RTH):
    """Returns {file name: text}. `rth` overrides the junction resistances (used by rth_measure)."""
    ref_text = config.NETLIST.read_text()
    ref = reference()
    check_partition(ref)
    v, i_pre = f_operating_point()
    files = {'full.cir': gen_full(ref_text)}
    for spec in _specs(v, i_pre):
        files[f'{spec.name}.cir'] = gen_block(spec, ref, ref_text, rth)
    return files


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--check', action='store_true')
    a = ap.parse_args()
    stale = []
    config.TESTBENCH_DIR.mkdir(parents=True, exist_ok=True)
    for fname, text in generate().items():
        path = config.TESTBENCH_DIR / fname
        if a.check:
            if not path.exists() or path.read_text() != text:
                stale.append(fname)
        else:
            path.write_text(text)
            print(f'wrote {path.relative_to(config.ROOT)}')
    if stale:
        print('out of date: ' + ', '.join(stale) + ' (run python -m harness.testbench_gen)')
        sys.exit(1)


if __name__ == '__main__':
    main()
