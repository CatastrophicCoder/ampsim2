import shutil
import unittest

from harness import ot_fit


@unittest.skipUnless(shutil.which('ngspice') is not None, 'ngspice not installed')
class TestOutputTransformer(unittest.TestCase):
    def test_netlist_transformer_matches_the_datasheet(self):
        problems, l_sc, l_oc, mag, ph = ot_fit.check()
        self.assertEqual(problems, [])

    def test_fit_reproduces_the_netlist_values(self):
        lpt, kot, cpp = ot_fit.fit()
        self.assertAlmostEqual(lpt, 25.61, delta=0.01)
        self.assertAlmostEqual(kot, 0.9997925, delta=1e-7)
        self.assertLess(cpp, 1e-12)


if __name__ == '__main__':
    unittest.main()
