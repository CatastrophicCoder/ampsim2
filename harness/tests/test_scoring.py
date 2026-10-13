"""Milestone scoring (tolerances.py) and the M2 presets."""
import unittest

import numpy as np

from harness import compare, signals, tolerances
from harness.blocks import BLOCKS
from harness.presets import PRESETS


class TestM2Scoring(unittest.TestCase):
    def test_b12_checks_are_scored_in_m2_only(self):
        self.assertEqual(tolerances.status('M2', 'b12', 'nonlinear_esr_db', -45.0)[1], 'PASS')
        self.assertEqual(tolerances.status('M2', 'b12', 'nonlinear_esr_db', -35.0)[1], 'FAIL')
        self.assertEqual(tolerances.status('M2', 'b12', 'nonlinear_esr_db_overdrive', -35.0)[1], 'PASS')
        self.assertEqual(tolerances.status('M2', 'b12', 'newton_median_iterations', 3)[1], 'PASS')
        self.assertEqual(tolerances.status('M2', 'b12', 'newton_median_iterations', 4)[1], 'FAIL')
        self.assertEqual(tolerances.status('M2', 'b12', 'newton_max_iterations', 12)[1], 'PASS')
        self.assertEqual(tolerances.status('M1', 'b12', 'nonlinear_esr_db', -10.0)[1], 'INFO')
        self.assertEqual(tolerances.status('M2', 'b3', 'nonlinear_esr_db', -10.0)[1], 'INFO')

    def test_overdrive_sines_use_the_overdrive_limit(self):
        block = BLOCKS['b12']
        sigs = [signals.sine(1000.0, 0.5, seconds=0.06), signals.sine(1000.0, 2.0, seconds=0.06),
                signals.step(0.1, seconds=0.02)]
        res = {'tran': {s.id: {'p1b': s.x} for s in sigs}, 'ac': None}
        stats = {'ns_per_sample': 1.0, 'newton_failures': 0, 'max_iterations_used': 2, 'newton_solves': 10,
                 'median_iterations': 2}
        eng = {s.id: {'out': s.x + 1e-4 * np.sin(np.arange(len(s.x))), 'stats': stats} for s in sigs}
        rep = compare.Report('M2', block)
        compare.analyse(block, sigs, res, eng, rep)
        checks = {(r[0], r[1]) for r in rep.rows}
        self.assertIn(('nonlinear_esr_db', 'SIN-1000Hz-0.5V'), checks)
        self.assertIn(('nonlinear_esr_db_overdrive', 'SIN-1000Hz-2V'), checks)
        self.assertIn(('nonlinear_esr_db', 'STEP-0.1V'), checks)
        self.assertIn(('newton_median_iterations', 'STEP-0.1V'), checks)


class TestM2Presets(unittest.TestCase):
    def test_volume_presets_run_the_standard_set(self):
        block = BLOCKS['b12']
        default_ids = [s.id for s in PRESETS['DEFAULT'].signals(block) if s.kind != 'DI']
        for pid, vol in (('VOL0.2', 0.2), ('VOL1.0', 1.0)):
            p = PRESETS[pid]
            self.assertEqual(p.all_knobs()['preampVolume'], vol)
            self.assertEqual([s.id for s in p.signals(block) if s.kind != 'DI'], default_ids)


if __name__ == '__main__':
    unittest.main()
