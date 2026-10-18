import shutil
import unittest

import numpy as np
from scipy.special import wrightomega

from harness import config, device_sweeps

HAS_NGSPICE = shutil.which('ngspice') is not None
VT = 1.38064852e-23 * 300.15 / 1.6021766208e-19    # 27 °C with ngspice's constants (WDF_DESIGN §3.1)
IS_DIODE = 1e-9


def softplus(u):
    return np.where(u > 30, u + np.log1p(np.exp(-np.minimum(u, 700))), np.log1p(np.exp(np.minimum(u, 30))))


def grid_current(vgk, rs):
    """WDF_DESIGN §3.1 closed form; below −3·Vt across the junction ngspice's diode uses its reverse-bias
    approximation −Is·(1 + (3·Vt / (e·Vd))³) instead (≤ 5e-11 A from the ideal diode, D-019)."""
    z = np.log(IS_DIODE * rs / VT) + (vgk + IS_DIODE * rs) / VT
    ig = VT / rs * np.real(wrightomega(z)) - IS_DIODE
    rev = vgk < -3 * VT
    for _ in range(3):           # junction voltage vd = vgk − Rs·ig (fixed point; ig ≈ −Is)
        vd = vgk - rs * ig
        ig = np.where(rev, -IS_DIODE * (1 + (3 * VT / (np.e * vd)) ** 3), ig)
    return ig


def triode(vgk, vpk, mu=100, ex=1.4, kg1=1060, kp=600, kvb=300):
    e1 = vpk * softplus(kp * (1 / mu + vgk / np.sqrt(kvb + vpk ** 2))) / kp
    return 2 * np.maximum(e1, 0) ** ex / kg1, grid_current(vgk, 2001.0)


def pentode(vsk, vgk, vpk, mu=14.77, ex=1.046, kg1=194.3, kg2=2243, kp=53.82, kvb=40.43):  # tubes.lib EL34 (D-058)
    vs = np.maximum(vsk, 1e-3)
    f = 2 * np.maximum(vs * softplus(kp * (1 / mu + vgk / vs)) / kp, 0) ** ex
    p = np.maximum(vpk, 0) + 5 * np.log1p(np.exp(-np.abs(vpk) / 5))   # 5 V softplus of the plate voltage (D-061)
    ip = f / kg1 * np.arctan(p / kvb)
    is_ = f * (1 / kg2 + (1.5707963 - np.arctan(p / kvb)) / kg1)   # tubes.lib's π/2
    return ip, is_, grid_current(vgk, 1001.0)


def assert_close(test, got, want, what):
    # 1e-7 relative or 1e-13 A: >= 1e4 tighter than VERIFICATION §5.1 (0.1 % or 1 nA), so the tables are
    # checked to reproduce the WDF_DESIGN §3 equations, not merely to be within the engine tolerance
    err = np.abs(got - want)
    bad = err > np.maximum(1e-7 * np.abs(want), 1e-13)
    test.assertFalse(bad.any(), f'{what}: {bad.sum()} points off, worst {err.max():.3g} A')


class TestDeviceSweepTables(unittest.TestCase):
    def test_triode_tables_match_wdf_design_equations(self):
        for name in ('12ax7_plate.csv', '12ax7_grid.csv'):
            d = device_sweeps.read(name)
            ip, ig = triode(d['vgk'], d['vpk'])
            assert_close(self, d['ip'], ip, f'{name} ip')
            assert_close(self, d['ig'], ig, f'{name} ig')
            assert_close(self, d['ip_terminal'], ip + d['vpk'] / device_sweeps.RCP, f'{name} ip_terminal')

    def test_pentode_tables_match_wdf_design_equations(self):
        for name in ('el34_plate.csv', 'el34_grid.csv'):
            d = device_sweeps.read(name)
            ip, is_, ig = pentode(d['vsk'], d['vgk'], d['vpk'])
            assert_close(self, d['ip'], ip, f'{name} ip')
            assert_close(self, d['is'], is_, f'{name} is')
            assert_close(self, d['ig'], ig, f'{name} ig')

    def test_tables_cover_the_verification_grid(self):
        d = device_sweeps.read('12ax7_plate.csv')
        self.assertEqual(len(d['vpk']), 7 * 81)
        self.assertEqual(sorted(set(d['vgk'])), sorted(device_sweeps.TRIODE_VGK))
        d = device_sweeps.read('el34_plate.csv')
        self.assertEqual(len(d['vpk']), 3 * 8 * 61)
        self.assertEqual(sorted(set(d['vsk'])), sorted(device_sweeps.PENTODE_VSK))

    @unittest.skipUnless(HAS_NGSPICE, 'ngspice not installed')
    def test_committed_tables_are_up_to_date(self):
        for name in device_sweeps.TABLES:
            path = device_sweeps.DATA_DIR / name
            self.assertTrue(path.exists(), f'{name} missing; run python -m harness.device_sweeps')
            self.assertEqual(path.read_text(), device_sweeps.render(name),
                             f'{name} stale; run python -m harness.device_sweeps')


if __name__ == '__main__':
    unittest.main()
