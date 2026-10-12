import shutil
import unittest

import numpy as np

from harness import config, dc_check, netlist, signals, spice, testbench_gen

HAS_NGSPICE = shutil.which('ngspice') is not None


class TestNetlistAndPartition(unittest.TestCase):
    def test_parser_reads_every_reference_element(self):
        ref = netlist.parse(config.NETLIST.read_text())
        body = [ln for ln in config.NETLIST.read_text().splitlines()[1:]
                if ln.strip() and not ln.lstrip().startswith(('*', '.'))]
        self.assertEqual(len(ref.elements), len(body))
        by = ref.by_name()
        self.assertEqual(by['x2b'].nodes, ('bpx', 'p2a', 'k2b'))
        self.assertEqual(by['x4'].nodes, ('pa', 's4', 'g4', '0'))
        self.assertEqual(by['k1'].nodes, ())

    def test_partition_covers_every_element_exactly_once(self):
        testbench_gen.check_partition(testbench_gen.reference())

    def test_partition_check_detects_missing_element(self):
        ref = testbench_gen.reference()
        broken = netlist.Netlist(ref.title, ref.directives, ref.elements + (netlist.Element('R99', 'R99 a 0 1', ('a', '0')),))
        with self.assertRaises(ValueError):
            testbench_gen.check_partition(broken)


@unittest.skipUnless(HAS_NGSPICE, 'ngspice not installed')
class TestGeneratedTestbenches(unittest.TestCase):
    def test_committed_testbenches_are_up_to_date(self):
        for fname, text in testbench_gen.generate().items():
            path = config.TESTBENCH_DIR / fname
            self.assertTrue(path.exists(), f'{fname} missing; run python -m harness.testbench_gen')
            self.assertEqual(path.read_text(), text, f'{fname} stale; run python -m harness.testbench_gen')

    def test_full_testbench_equals_reference_netlist(self):
        ref = config.NETLIST.read_text().split('\n', 1)[1]
        self.assertTrue((config.TESTBENCH_DIR / 'full.cir').read_text().endswith(ref))

    def test_dc_matches_full_netlist(self):
        # b6 has a known 40 µV partition deviation at out/sl (see dc_check.KNOWN); everything else 0.5 %
        for name in dc_check.TESTBENCHES:
            for node, vf, vt, err in dc_check.check(name):
                if (name, node) in dc_check.KNOWN:
                    continue
                self.assertLessEqual(err, 0.005, f'{name} {node}: F {vf} vs {vt}')

    def test_pending_testbench_is_refused_for_transient(self):
        text = (config.TESTBENCH_DIR / 'b3.cir').read_text()
        if spice.PENDING_MARK not in text:
            self.skipTest('R_th already measured')
        with self.assertRaises(spice.SpiceError):
            spice.tran(text, {}, signals.sine(1000.0, 1.0, seconds=0.001).x, ['k2b'])


@unittest.skipUnless(HAS_NGSPICE, 'ngspice not installed')
class TestRecordedSources(unittest.TestCase):
    TB = '* rec\nVrec_a a 0 DC 1\nVrec_b b 0 DC 2\nR1 a out 1k\nR2 out b 1k\n'

    def test_recorded_sources_drive_the_testbench(self):
        a = signals.sine(1000.0, 1.0, seconds=0.002).x
        b = 0.5 * signals.sine(3000.0, 1.0, seconds=0.002).x
        out = spice.tran(self.TB, {}, None, ['out'], recorded={'a': a, 'b': b})['out']
        np.testing.assert_allclose(out, (a + b) / 2, atol=1e-9)

    def test_missing_recording_is_an_error(self):
        with self.assertRaises(spice.SpiceError):
            spice.tran(self.TB, {}, None, ['out'], recorded={'a': np.zeros(10)})


if __name__ == '__main__':
    unittest.main()
