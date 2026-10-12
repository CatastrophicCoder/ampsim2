"""Block registry: which testbench, input source, output node and probes belong to each block id
(SPEC §4.2, VERIFICATION §1). Block testbenches B1..B6 are added in M0 step 4."""
from dataclasses import dataclass, field

from . import config

# DC reference nodes of the full netlist (CIRCUIT.md §5)
FULL_DC_NODES = ('ct', 'nscr', 'bpx', 'bpa', 'nbias', 'p1a', 'k1a', 'p1b', 'k1b', 'p2a', 'k2a', 'k2b',
                 'p3a', 'p3b', 'k3', 'nf', 'ng')


@dataclass(frozen=True)
class Block:
    id: str
    title: str
    testbench: str                # file in harness/testbenches/
    input_node: str               # node driven by the input source ('Vin <node> 0 ...' in the testbench)
    output_node: str              # node compared against the engine's 'out'
    probes: tuple = ()            # extra nodes rendered for overlays / debugging
    dc_nodes: tuple = ()          # nodes in the DC operating-point tables
    levels: dict = field(default_factory=dict)   # VERIFICATION §4, volts peak
    engine_block: str = ''        # ampwav --block id
    output_load: float = 0.0      # ohms, for power metrics (0 = none)

    @property
    def path(self):
        return config.TESTBENCH_DIR / self.testbench


BLOCKS = {
    'full': Block(
        id='full', title='Full amplifier (reference F)', testbench='full.cir',
        input_node='in', output_node='out',
        probes=('in', 'p1a', 'w1', 'p1b', 'p2a', 'k2b', 'y', 'p3a', 'p3b', 'ct', 'nscr', 'bpx', 'bpa'),
        dc_nodes=FULL_DC_NODES,
        levels={'small': 0.01, 'nominal': 0.1, 'hot': 0.5, 'overdrive': 2.0},
        engine_block='full', output_load=16.0),
}
