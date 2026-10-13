import shutil
import unittest

import numpy as np

from harness import config, signals, spice

DIVIDER = """* divider testbench
.param gain=0.5
Vin in 0 DC 0 AC 1
R1 in out 1k
R2 out 0 1k
"""

RC = """* RC low-pass testbench
Vin in 0 DC 0 AC 1
R1 in out 1k
C1 out 0 1u
"""


@unittest.skipIf(shutil.which('ngspice') is None, 'ngspice not installed')
class TestSpice(unittest.TestCase):
    def test_tran_is_sample_aligned_including_last_sample(self):
        # 5 kHz: without per-sample breakpoints the input would be low-passed by ~1.3e-3 (D-014)
        for freq in (1000.0, 5000.0):
            x = signals.sine(freq, 1.0, seconds=0.01).x
            res = spice.tran(DIVIDER, {}, x, ['out', 'in'])
            self.assertEqual(len(res['out']), len(x))
            np.testing.assert_allclose(res['in'], x, atol=1e-9)
            np.testing.assert_allclose(res['out'], x / 2, atol=1e-9)

    def test_rc_tran_matches_exact_response_with_tight_reltol(self):
        # Reference accuracy check: with per-sample breakpoints and reltol 1e-6, SPICE matches the exact
        # continuous-time response to the piecewise-linear input (scipy lsim) to better than -80 dB.
        # At the ngspice default reltol it gives about -67 dB at 15 kHz (-52 dB without breakpoints; D-014).
        from scipy import signal as ss
        for freq in (5000.0, 15000.0):
            x = signals.sine(freq, 1.0, seconds=0.01).x
            t = np.arange(len(x)) / config.FS
            _, exact, _ = ss.lsim(([1.0], [1e-3, 1.0]), x, t, interp=True)
            res = spice.tran(RC, {}, x, ['out'], reltol=1e-6)['out']
            err = 10 * np.log10(np.sum((res - exact) ** 2) / np.sum(exact ** 2))
            self.assertLess(err, -80.0, f'{freq:g} Hz')

    def test_op_and_ac(self):
        self.assertAlmostEqual(spice.op(DIVIDER, {}, ['out'])['out'], 0.0)
        f, h = spice.ac(RC, {}, 'out', 'in')
        exact = 1 / (1 + 2j * np.pi * f * 1e3 * 1e-6)
        np.testing.assert_allclose(h, exact, rtol=1e-6)

    def test_impedance_of_rc_with_input_ac_grounded(self):
        # Z at out = R1 || C1 (Vin AC-grounded); the transimpedance to `in` is 0 (ideal source)
        f, z = spice.impedance(RC, {}, 'out', ('in',))
        exact = 1e3 / (1 + 2j * np.pi * f * 1e3 * 1e-6)
        np.testing.assert_allclose(z['out'], exact, rtol=1e-6)
        np.testing.assert_allclose(np.abs(z['in']), 0.0, atol=1e-9)
        self.assertTrue(np.any(np.isclose(f, 1000.0, rtol=1e-12)))

    def test_node_names_that_are_control_language_operators(self):
        # `ne`, `gt` and `and` are operators in ngspice's control language (B4 has a node nE)
        tb = '* operator names\nVin ne 0 DC 0 AC 1\nR1 ne gt 1k\nR2 gt and 1k\nR3 and 0 2k\n'
        x = signals.sine(1000.0, 1.0, seconds=0.002).x
        res = spice.tran(tb, {}, x, ['gt', 'and', 'ne'])
        np.testing.assert_allclose(res['gt'], 0.75 * x, atol=1e-9)
        np.testing.assert_allclose(res['and'], 0.5 * x, atol=1e-9)
        _, h = spice.ac(tb, {}, 'and', 'ne')
        np.testing.assert_allclose(h, 0.5, rtol=1e-9)
        _, z = spice.impedance(tb, {}, 'gt', ('and',))
        np.testing.assert_allclose(z['gt'], 1e3 * 3e3 / 4e3, rtol=1e-9)

    def test_vin_dc(self):
        self.assertEqual(spice.vin_dc('* t\nVin j3s 0 DC 232.5 AC 1\n'), 232.5)
        self.assertEqual(spice.vin_dc(DIVIDER), 0.0)
        self.assertIsNone(spice.vin_dc('* t\nVrec_a a 0 DC 1\n'))

    def test_apply_params_substitutes_known_params_only(self):
        text = spice.apply_params('.param vol=0.6 master=0.3\n.include tubes.lib\n',
                                  {'preampVolume': 0.8, 'presence': 1.0})
        self.assertIn('vol=0.8 ', text)
        self.assertIn('master=0.3', text)
        self.assertIn(f'.include {config.TUBES}', text)



class TestCacheKeyCoversLibraries(unittest.TestCase):
    def test_key_changes_with_included_library_content(self):
        import tempfile
        from pathlib import Path
        with tempfile.TemporaryDirectory() as d:
            lib = Path(d) / 'x.lib'
            text = f'* t\n.include {lib}\nR1 a 0 1k\n'
            lib.write_text('* one\n')
            k1 = spice._key('op', text)
            lib.write_text('* two\n')
            self.assertNotEqual(k1, spice._key('op', text))
            self.assertEqual(spice._key('op', text), spice._key('op', text))


if __name__ == '__main__':
    unittest.main()
