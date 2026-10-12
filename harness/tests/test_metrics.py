import unittest

import numpy as np
from scipy import signal as ss

from harness import config, metrics, signals


class TestMetrics(unittest.TestCase):
    def test_esr_known_value(self):
        ref = np.sin(np.linspace(0, 20, 1000))
        self.assertAlmostEqual(metrics.esr_db(ref, ref * 1.1), 20 * np.log10(0.1), places=9)
        self.assertLess(metrics.esr_db(ref, ref), -250)

    def test_esr_on_ac_part_ignores_static_dc_but_keeps_bias_shift(self):
        ac = np.sin(np.linspace(0, 20, 1000))
        ref = 250.0 + ac
        # a 1 V static offset (within the 0.5 % DC tolerance at 250 V) does not count
        self.assertLess(metrics.esr_db(metrics.ac_part(ref), metrics.ac_part(ref + 1.0)), -250)
        # raw voltages hide a 10 % AC error behind the DC term
        self.assertLess(metrics.esr_db(ref, 250.0 + 1.1 * ac), -60)
        self.assertAlmostEqual(metrics.esr_db(metrics.ac_part(ref), metrics.ac_part(250.0 + 1.1 * ac)),
                               20 * np.log10(0.1), places=9)
        # a bias shift that develops during the signal is scored
        shift = np.linspace(0.0, -0.5, 1000)
        self.assertGreater(metrics.esr_db(metrics.ac_part(ref), metrics.ac_part(ref + shift)), -10)

    def test_best_lag_detects_one_sample_delay(self):
        x = signals.sine(1000.0, 1.0).x
        late = np.r_[0.0, x[:-1]]
        lag, e = metrics.best_lag(x, late)
        self.assertEqual(lag, 1)
        self.assertLess(e, -250)
        self.assertEqual(metrics.best_lag(x, x)[0], 0)

    def test_harmonics_and_thd(self):
        t = np.arange(int(0.15 * config.FS)) / config.FS
        x = 2.0 * np.sin(2 * np.pi * 1000 * t) + 0.02 * np.sin(2 * np.pi * 3000 * t)
        h = metrics.harmonics(x, 1000.0)
        self.assertAlmostEqual(h[0], 2.0, delta=2e-3)
        self.assertAlmostEqual(h[2], 0.02, delta=2e-5)
        self.assertLess(h[1], 1e-6)
        self.assertAlmostEqual(metrics.thd_percent(h), 1.0, delta=2e-3)
        diff, scored = metrics.harmonic_diff_db(h, h * 10 ** (0.5 / 20))
        np.testing.assert_allclose(diff, 0.5, atol=1e-9)
        self.assertTrue(scored[0] and scored[2] and not scored[1])

    def test_sweep_response_matches_exact_filter(self):
        f = np.geomspace(20, 20000, 300)
        b1, a1 = ss.iirpeak(5000, 2, fs=config.FS)
        b2, a2 = ss.butter(2, 100, 'high', fs=config.FS)
        x = signals.sweep(0.01, 'SWP-lo').x
        y = ss.lfilter(b2, a2, ss.lfilter(b1, a1, x) + x)
        _, h1 = ss.freqz(b1, a1, f, fs=config.FS)
        _, h2 = ss.freqz(b2, a2, f, fs=config.FS)
        exact = 20 * np.log10(np.abs(h2 * (h1 + 1)))
        err = np.abs(metrics.sweep_response_db(x, y, f) - exact)
        self.assertLess(err.max(), 0.01, f'max error {err.max():.3g} dB at {f[err.argmax()]:.0f} Hz')

    def test_mean_power(self):
        t = np.arange(int(0.1 * config.FS)) / config.FS
        v = 40.0 * np.sin(2 * np.pi * 1000 * t)
        self.assertAlmostEqual(metrics.mean_power(v, 16.0), 40.0 ** 2 / 2 / 16.0, delta=1e-6)


class TestSignals(unittest.TestCase):
    def test_all_signals_start_at_zero_and_are_finite(self):
        for s in signals.standard_set({'small': 0.1, 'nominal': 1.0, 'hot': 10.0, 'overdrive': 30.0},
                                      include=('SIN', 'SWP-lo', 'SWP-hi', 'TT', 'BURST', 'STEP')):
            self.assertEqual(s.x[0], 0.0, s.id)
            self.assertTrue(np.all(np.isfinite(s.x)), s.id)

    def test_shapes_and_levels(self):
        s = signals.sine(1000.0, 0.5)
        self.assertEqual(len(s.x), int(0.15 * config.FS))
        self.assertAlmostEqual(np.max(np.abs(s.x)), 0.5, places=6)

        b = signals.burst(10.0)
        hi = b.x[int(0.1 * config.FS): int(0.3 * config.FS)]
        lo = b.x[int(0.4 * config.FS):]
        self.assertAlmostEqual(20 * np.log10(np.max(np.abs(hi)) / np.max(np.abs(lo))), 30.0, places=3)

        st = signals.step(1.0)
        self.assertAlmostEqual(np.mean(st.x), 0.0, delta=1e-3)
        self.assertEqual(set(np.unique(st.x)), {-1.0, 0.0, 1.0})

        sw = signals.sweep(1.0, 'SWP-lo')
        tail = int(signals.SWEEP_TAIL * config.FS)
        self.assertEqual(len(sw.x), int(signals.SWEEP_SECONDS * config.FS) + tail)
        self.assertTrue(np.all(sw.x[-tail:] == 0.0))

    def test_ids_are_unique(self):
        ids = [s.id for s in signals.standard_set({'small': 0.01, 'nominal': 0.1, 'hot': 0.5, 'overdrive': 2.0},
                                                  include=('SIN', 'SWP-lo', 'SWP-hi', 'TT', 'BURST', 'STEP'))]
        self.assertEqual(len(ids), len(set(ids)))


if __name__ == '__main__':
    unittest.main()
