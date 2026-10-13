"""Recorded inputs for the blocks that are driven by another run (VERIFICATION §1: B5-open and B6).

- b5open: R22 is driven by the speaker voltage and bpX by the supply of a b56 run (B5 + B6 + B7 joined,
  instantaneous NFB) on the same J4 input.
- b6: the PI open-circuit outputs p3a / p3b come from that b5open run, the supply nodes ct / nScr from the
  b56 run.
The b56 / b5open renders are ordinary cached SPICE transients. If ngspice stops with "timestep too small"
at the configured RELTOL, the source run is repeated once at RETRY_RELTOL and the note says so.
"""
from . import config, spice
from .blocks import BLOCKS

RETRY_RELTOL = 1e-7
# nodes saved from every b56 source run; harness/partition_j5.py renders the same set, so the SPICE cache
# (keyed by the node list too) serves both
B56_NODES = ('out', 'ngn', 'nor', 'pa', 'pb', 'p3a', 'p3b', 'ct', 'y', 'bpx', 'nscr')


def render_nodes(block):
    """Nodes rendered for a block's transients (render_spice and the source runs use the same list; b56 uses
    B56_NODES, so its compare renders and the source runs share the cache)."""
    if block.id == 'b56':
        return list(B56_NODES)
    return [n for n in dict.fromkeys([block.output_node, block.input_node, *block.probes]) if n]


def _tran(text, knobs, x, nodes, recorded=None):
    try:
        return spice.tran(text, knobs, x, nodes, recorded=recorded), None
    except spice.SpiceError as e:
        if 'Timestep too small' not in str(e):
            raise
        res = spice.tran(text, knobs, x, nodes, recorded=recorded, reltol=RETRY_RELTOL)
        return res, f'source run rendered at RELTOL {RETRY_RELTOL:g} (timestep too small at {config.SPICE_RELTOL:g})'


def j4_input(sig):
    """The standalone signal on the J4 source's DC operating point (b56 and b5open share it, D-022)."""
    return sig.x + spice.vin_dc(BLOCKS['b56'].path.read_text())


def recordings(block, knobs, sig):
    """Returns ({name: samples} for the block's Vrec_<name> sources, list of notes)."""
    if not block.recorded:
        return {}, []
    r56, n56 = _tran(BLOCKS['b56'].path.read_text(), knobs, j4_input(sig), list(B56_NODES))
    notes = [f'b56: {n56}'] if n56 else []
    if block.id == 'b5open':
        return {'out': r56['out'], 'bpx': r56['bpx']}, notes
    if block.id == 'b6':
        r5, n5 = _tran(BLOCKS['b5open'].path.read_text(), knobs, j4_input(sig), render_nodes(BLOCKS['b5open']),
                       recorded={'out': r56['out'], 'bpx': r56['bpx']})
        notes += [f'b5open: {n5}'] if n5 else []
        return {'p3a': r5['p3a'], 'p3b': r5['p3b'], 'ct': r56['ct'], 'nscr': r56['nscr']}, notes
    raise NotImplementedError(f'no recording chain for {block.id}')


def engine_inputs(block, sig, rec):
    """(input samples for ampwav --in, {name: samples} for --supply) for a block with recorded inputs."""
    if block.id == 'b6':
        return rec['p3a'], {k: v for k, v in rec.items() if k != 'p3a'}
    return j4_input(sig), rec  # b5open, b56
