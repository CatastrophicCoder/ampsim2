import unittest

import numpy as np

from harness import mna


class TestMna(unittest.TestCase):
    def test_parallel_three_port_matches_closed_form(self):
        net = mna.PortNetwork((('n', '0'), ('n', '0'), ('n', '0')))
        R = np.array([10.0, 220.0, 4.7e3])
        G = 1 / R
        # common voltage v = Σ G a / Σ G, b = 2 v − a
        want = 2 * np.outer(np.ones(3), G) / G.sum() - np.eye(3)
        np.testing.assert_allclose(mna.scattering(net, R), want, atol=1e-14)

    def test_series_three_port_matches_closed_form(self):
        # loop: v1 + v2 + v3 = 0, common current i = Σ a / Σ R, b = a − 2 R i
        net = mna.PortNetwork((('n1', '0'), ('n2', 'n1'), ('0', 'n2')))
        R = np.array([10.0, 220.0, 4.7e3])
        want = np.eye(3) - 2 * np.outer(R, np.ones(3)) / R.sum()
        np.testing.assert_allclose(mna.scattering(net, R), want, atol=1e-14)

    def test_b4_adaptor_is_lossless_and_reflection_free(self):
        for name, knobs in mna.FIXTURE_CASES:
            R, S = mna.adapted(mna.B4, mna.b4_resistances(**knobs), 0)
            G = np.diag(1 / R)
            np.testing.assert_allclose(S.T @ G @ S, G, rtol=1e-9, atol=1e-9 * G.max(), err_msg=name)
            self.assertLess(abs(S[0, 0]), 1e-12, name)

    def test_up_port_resistance_is_the_input_resistance(self):
        # two ports in series to ground behind the up-port: R_up = R1 + R2
        net = mna.PortNetwork((('a', '0'), ('a', 'b'), ('b', '0')))
        self.assertAlmostEqual(mna.up_port_resistance(net, [1.0, 100.0, 220.0], 0), 320.0, places=9)

    def test_committed_fixtures_are_up_to_date(self):
        self.assertTrue(mna.FIXTURES.exists(), 'run python -m harness.mna --fixtures')
        self.assertEqual(mna.FIXTURES.read_text(), mna.fixtures_text())


if __name__ == '__main__':
    unittest.main()
