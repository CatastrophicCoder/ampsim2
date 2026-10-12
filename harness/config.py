"""Paths and constants shared by the harness modules."""
from pathlib import Path

FS = 384000                    # verification rate (DECISIONS D-007)
INPUT_VOLTS_PER_FS = 4.361     # 0 dBFS = +12 dBu at the jack (D-010); scales DI recordings

ROOT = Path(__file__).resolve().parent.parent
HARNESS = ROOT / 'harness'
SPICE_DIR = ROOT / 'reference' / 'spice'
NETLIST = SPICE_DIR / 'jcm800_2203.cir'
TUBES = SPICE_DIR / 'tubes.lib'
DI_DIR = ROOT / 'reference' / 'di'
TESTBENCH_DIR = HARNESS / 'testbenches'
CACHE_DIR = HARNESS / 'cache'
RENDER_DIR = HARNESS / 'renders'
REPORT_DIR = HARNESS / 'reports'
AMPWAV = ROOT / 'build' / 'tools' / 'ampwav' / 'ampwav'

# ngspice transient relative tolerance for reference renders (owner decision, DECISIONS D-014 / PROGRESS
# Q4). 1e-6 gives about -87 dB at `out` and -76 dB at `y` vs a 1e-7 run on the full amp, at about 5x
# the run time of the ngspice default (1e-3, used by reference/spice/spice_tools.py). op and ac are
# unaffected. Part of the render cache key; None = ngspice default.
SPICE_RELTOL = 1e-6

# engine parameter name -> netlist .param name (SPEC §6, jcm800_2203.cir)
KNOBS = {
    'preampVolume': 'vol',
    'treble': 'treble',
    'middle': 'middle',
    'bass': 'bass',
    'master': 'master',
    'presence': 'presence',
}
DEFAULT_KNOBS = {'preampVolume': 0.6, 'treble': 0.5, 'middle': 0.5, 'bass': 0.5, 'master': 0.3,
                 'presence': 0.5}
