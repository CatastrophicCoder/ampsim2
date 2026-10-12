"""Renders SPICE references for a block and preset: DC operating point, transient for every signal of the
preset, and the small-signal AC response when the preset has a sweep. Results are cached
(harness/cache/); WAV copies of the rendered nodes go to harness/renders/spice/<block>/<preset>/.

    python -m harness.render_spice --block full --preset CRUNCH
    python -m harness.render_spice --block full --preset DEFAULT --signal SIN-1000Hz
"""
import argparse
import json
import os
import time
from concurrent.futures import ThreadPoolExecutor

from . import config, spice
from .blocks import BLOCKS
from .presets import PRESETS
from .wavio import write_wav


def _require_standalone(block):
    if block.recorded:
        raise NotImplementedError(
            f'{block.id} is driven by recordings ({", ".join(block.recorded)}) from a B56 run; '
            'chained P renders arrive with M4')


def block_input(block, sig):
    """Input samples of a block for a standalone signal: the signal rides on the DC value of the block's
    `Vin` source (the junction source's operating point; 0 V for the guitar input), so SPICE and the
    engine start from the same operating point as F. The engine gets the same samples (DECISIONS D-022)."""
    return sig.x + (spice.vin_dc(block.path.read_text()) or 0.0)


def select_signals(block, preset, pattern=None):
    _require_standalone(block)
    sigs = preset.signals(block)
    return [s for s in sigs if pattern is None or pattern in s.id]


def render(block, preset, signals, jobs=None, log=print):
    """Returns {'op': {...}, 'ac': (f, h) or None, 'tran': {signal id: {node: array}}}."""
    _require_standalone(block)
    text = block.path.read_text()
    knobs = preset.all_knobs()
    nodes = list(dict.fromkeys([block.output_node, block.input_node, *block.probes]))

    result = {'op': spice.op(text, knobs, block.dc_node_list()), 'ac': None, 'tran': {}}
    if any(s.kind == 'SWP-lo' for s in signals):
        result['ac'] = spice.ac(text, knobs, block.output_node, block.input_node)

    def one(sig):
        t0 = time.time()
        res = spice.tran(text, knobs, block_input(block, sig), nodes)
        log(f'  spice {block.id}/{preset.id}/{sig.id}: {len(sig.x) / config.FS:.3f} s signal, {time.time() - t0:.1f} s')
        return sig.id, res

    with ThreadPoolExecutor(max_workers=jobs or os.cpu_count()) as ex:
        for sid, res in ex.map(one, signals):
            result['tran'][sid] = res
    return result


def write_renders(block, preset, result):
    out = config.RENDER_DIR / 'spice' / block.id / preset.id
    for sid, nodes in result['tran'].items():
        d = out / sid
        d.mkdir(parents=True, exist_ok=True)
        for node, v in nodes.items():
            write_wav(d / f'{node}.wav', v)
    (out / 'op.json').write_text(json.dumps(result['op'], indent=2))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--block', required=True, choices=sorted(BLOCKS))
    ap.add_argument('--preset', default='DEFAULT', choices=sorted(PRESETS))
    ap.add_argument('--signal', help='only signals whose id contains this text')
    ap.add_argument('--jobs', type=int, help='parallel ngspice runs (default: CPU count)')
    a = ap.parse_args()

    block, preset = BLOCKS[a.block], PRESETS[a.preset]
    sigs = select_signals(block, preset, a.signal)
    print(f'render_spice: {block.id} / {preset.id}: {len(sigs)} signals, ngspice {spice.ngspice_version()}')
    result = render(block, preset, sigs, a.jobs)
    out = write_renders(block, preset, result)
    print(f'render_spice: wrote {out.relative_to(config.ROOT)}')


if __name__ == '__main__':
    main()
