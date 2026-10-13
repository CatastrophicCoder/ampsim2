"""Runs the engine (ampwav) on the same signals as render_spice. Outputs go to
harness/renders/engine/<block>/<preset>/<signal>/ (out.wav, one WAV per probed node, stats.json).

    python -m harness.run_engine --block full --preset CRUNCH
"""
import argparse
import json
import subprocess

from . import config, recordings
from .blocks import BLOCKS
from .presets import PRESETS
from .render_spice import block_input, select_signals
from .wavio import read_wav, write_wav


class EngineError(RuntimeError):
    pass


def _ampwav(args):
    if not config.AMPWAV.exists():
        raise EngineError(f'{config.AMPWAV} not found; build first (cmake --build build)')
    r = subprocess.run([str(config.AMPWAV), *args], capture_output=True, text=True)
    if r.returncode != 0:
        raise EngineError(f'ampwav {" ".join(args)} failed:\n{r.stderr}')
    return r


def engine_nodes(block):
    return _ampwav(['--block', block.engine_block, '--list-nodes']).stdout.split()


def run(block, preset, signals, log=print, recorded=None):
    """Returns {signal id: {'out': array, 'nodes': {node: array}, 'stats': dict}}. `recorded`: {signal id:
    {name: samples}} for blocks driven by recordings (harness/recordings.py), passed as `--supply`."""
    available = set(engine_nodes(block))
    wanted = [n for n in dict.fromkeys([block.input_node, *block.probes, *block.dc_node_list()]) if n in available]
    params = [x for k, v in preset.all_knobs().items() for x in ('--param', f'{k}={v!r}')]
    results = {}
    for sig in signals:
        d = config.RENDER_DIR / 'engine' / block.id / preset.id / sig.id
        d.mkdir(parents=True, exist_ok=True)
        sig_wav = config.RENDER_DIR / 'signals' / block.id / f'{sig.id}.wav'
        sig_wav.parent.mkdir(parents=True, exist_ok=True)
        supplies = []
        if block.recorded:
            x, aux = recordings.engine_inputs(block, sig, recorded[sig.id])
            for name, v in aux.items():
                write_wav(d / f'rec_{name}.wav', v)
                supplies += ['--supply', f'{name}={d / f"rec_{name}.wav"}']
        else:
            x = block_input(block, sig)
        write_wav(sig_wav, x)
        base = ['--block', block.engine_block, '--in', str(sig_wav), '--rate', str(config.FS), *params, *supplies]

        # timing run without node dumps (block-wise processing), then a run with dumps
        _ampwav(base + ['--out', str(d / 'out.wav'), '--stats', str(d / 'stats.json')])
        dumps = [x for n in wanted for x in ('--dump-node', f'{n}={d / (n + ".wav")}')]
        _ampwav(base + ['--out', str(d / 'out_dump.wav'), *dumps])

        out, _ = read_wav(d / 'out.wav')
        results[sig.id] = {
            'out': out,
            'nodes': {n: read_wav(d / f'{n}.wav')[0] for n in wanted},
            'stats': json.loads((d / 'stats.json').read_text()),
        }
        log(f'  engine {block.id}/{preset.id}/{sig.id}: {results[sig.id]["stats"]["ns_per_sample"]:.1f} ns/sample')
    return results


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--block', required=True, choices=sorted(BLOCKS))
    ap.add_argument('--preset', default='DEFAULT', choices=sorted(PRESETS))
    ap.add_argument('--signal', help='only signals whose id contains this text')
    a = ap.parse_args()
    block, preset = BLOCKS[a.block], PRESETS[a.preset]
    sigs = select_signals(block, preset, a.signal)
    rec = {s.id: recordings.recordings(block, preset.all_knobs(), s)[0] for s in sigs} if block.recorded else None
    run(block, preset, sigs, recorded=rec)


if __name__ == '__main__':
    main()
