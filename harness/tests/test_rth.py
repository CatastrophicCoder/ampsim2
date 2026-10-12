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
        # junctions.py holds |Z(1 kHz)| (owner decision Q6b, D-018); a netlist or testbench change that
        # moves a value by more than 0.1 % must be re-measured and re-recorded
        self.assertEqual(set(RTH), {j.key for j in rth_measure.JUNCTIONS})
        for k, v in RTH.items():
            self.assertIsNotNone(v, k)
            z = abs(self.results[k]['ac'])
            self.assertLess(abs(v / z - 1), 1e-3, f'{k}: junctions.py {v} vs measured |Z(1 kHz)| {z:.6g}')


if __name__ == '__main__':
    unittest.main()


@unittest.skipUnless(HAS_NGSPICE, 'ngspice not installed')
class TestPartitionStudy(unittest.TestCase):
    def test_small_signal_partition_error_matches_thevenin_prediction(self):
        # The chained AC error of the J2 study must equal |Z_in + Z_th| / |Z_in + R| from the separately
        # measured upstream output impedance Z_th and downstream input impedance Z_in
        import numpy as np
        from harness import partition, spice, testbench_gen
        st = partition.STUDIES['j2']
        r = rth_measure.measure()[st.junction]
        fz, zth = r['f'], r['z'][st.up_node]
        rth = dict(RTH)
        rth[st.junction] = 1e12
        fi, zin = spice.impedance(testbench_gen.generate(rth)[f'{st.downstream}.cir'], {}, st.up_node)
        cands = partition.candidates(st)
        f, err = partition._ac(st, partition._texts(st, cands), {}, cands)
        interp = lambda fx, z: np.interp(np.log(f), np.log(fx), z.real) + 1j * np.interp(np.log(f), np.log(fx), z.imag)  # noqa: E731
        zt, zi = interp(fz, zth), interp(fi, zin[st.up_node])
        for c, R in cands.items():
            pred = 20 * np.log10(np.abs((zi + zt) / (zi + R)))
            self.assertLess(np.max(np.abs(err[c] - pred)), 0.01, c)
