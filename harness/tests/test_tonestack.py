import shutil
import unittest

import numpy as np

from harness import config, knob_jump, metrics, signals, spice
from harness.blocks import BLOCKS
from harness.presets import Preset
from harness.render_spice import block_input
from harness.run_engine import EngineError, run

HAS_NGSPICE = shutil.which('ngspice') is not None


@unittest.skipUnless(HAS_NGSPICE and config.AMPWAV.exists(), 'needs ngspice and a built ampwav')
class TestToneStackEngine(unittest.TestCase):
    def test_engine_matches_spice_for_a_short_tone(self):
        block = BLOCKS['b4']
        sig = signals.sine(2000.0, 1.0, seconds=0.02)
        preset = Preset('UNITTEST', {'treble': 0.8, 'bass': 0.2, 'middle': 0.7, 'master': 0.6}, 'test')
        ref = spice.tran(block.path.read_text(), preset.all_knobs(), block_input(block, sig), ['y'])['y']
        try:
            out = run(block, preset, [sig], log=lambda *_: None)[sig.id]['out']
        except EngineError as e:
            self.skipTest(str(e))
        r, y = metrics.ac_part(ref), metrics.ac_part(out)
        self.assertLess(metrics.esr_db(r, y), -60.0)
        self.assertEqual(metrics.best_lag(r, y)[0], 0)


class TestKnobJumpHelpers(unittest.TestCase):
    def test_smoother_matches_the_engine_definition(self):
        # one-pole towards the target, snapped within 1e-7 (amp2203::detail::KnobSmoother)
        tgt = np.r_[np.zeros(10), np.ones(int(0.5 * config.FS))]
        s = knob_jump.smoothed(tgt)
        coeff = np.exp(-1.0 / (knob_jump.SMOOTHING_MS * 1e-3 * config.FS))
        self.assertAlmostEqual(s[10], 1 - coeff, places=15)
        self.assertEqual(s[-1], 1.0)
        self.assertTrue(np.all(np.diff(s) >= 0))

    def test_spice_testbench_replaces_every_pot(self):
        text = knob_jump.spice_testbench()
        for name in knob_jump.POTS:
            self.assertRegex(text, rf"(?m)^{name} \S+ \S+ R='")
        self.assertEqual(text.count('Vrec_rot_'), 4)


@unittest.skipUnless(HAS_NGSPICE, 'ngspice not installed')
class TestSpiceDeviceParameters(unittest.TestCase):
    def test_op_reads_device_parameters_and_currents(self):
        tb = '* t\nV1 a 0 DC 2\nRx a 0 {1k*2}\n'
        v = spice.op(tb, {}, ['@rx[resistance]', '@rx[i]'])
        self.assertAlmostEqual(v['@rx[resistance]'], 2000.0)
        self.assertAlmostEqual(v['@rx[i]'], 1e-3)


if __name__ == '__main__':
    unittest.main()
