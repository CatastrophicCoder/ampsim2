import shutil
import unittest

from harness import rth_measure
from harness.junctions import RTH

HAS_NGSPICE = shutil.which('ngspice') is not None


@unittest.skipUnless(HAS_NGSPICE, 'ngspice not installed')
class TestJunctionMeasurement(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.results = rth_measure.measure()

    def test_ac_dc_point_matches_operating_point_finite_difference(self):
        rth = rth_measure.upstream_rth(self.results, 'dc')
        for j in rth_measure.JUNCTIONS:
            fd = rth_measure.dc_finite_difference(j, rth)
            self.assertAlmostEqual(self.results[j.key]['dc'].real / fd, 1.0, delta=1e-4, msg=j.key)

    def test_junctions_py_matches_measurement(self):
        # junctions.py holds the owner-chosen basis of each measured value (DECISIONS); a netlist or
        # testbench change that moves a value by more than 0.1 % must be re-measured and re-recorded
        for k, v in RTH.items():
            if v is None:
                continue
            r = self.results[k]
            self.assertTrue(min(abs(v / r['dc'].real - 1), abs(v / abs(r['ac']) - 1)) < 1e-3,
                            f'{k}: junctions.py {v} vs measured DC {r["dc"].real:.6g}, 1 kHz {abs(r["ac"]):.6g}')


if __name__ == '__main__':
    unittest.main()


@unittest.skipUnless(HAS_NGSPICE, 'ngspice not installed')
class TestPartitionStudy(unittest.TestCase):
    def test_small_signal_partition_error_matches_thevenin_prediction(self):
        # The chained AC error of the J1 study must equal |Z_in + Z_th| / |Z_in + R| from the separately
        # measured upstream output impedance Z_th and downstream input impedance Z_in
        import numpy as np
        from harness import partition, spice, testbench_gen
        st = partition.STUDIES['j1']
        r = rth_measure.measure()['j1']
        fz, zth = r['f'], r['z']['p1a']
        rth = dict(RTH)
        rth['j1'] = 1e12
        fi, zin = spice.impedance(testbench_gen.generate(rth)['b2.cir'], {}, 'p1a')
        cands = partition.candidates(st)
        f, err = partition._ac(st, partition._texts(st, cands), {}, cands)
        interp = lambda fx, z: np.interp(np.log(f), np.log(fx), z.real) + 1j * np.interp(np.log(f), np.log(fx), z.imag)  # noqa: E731
        zt, zi = interp(fz, zth), interp(fi, zin['p1a'])
        for c, R in cands.items():
            pred = 20 * np.log10(np.abs((zi + zt) / (zi + R)))
            self.assertLess(np.max(np.abs(err[c] - pred)), 0.01, c)
