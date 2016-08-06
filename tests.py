import unittest
from cp_otp import strxor


class CPOTOPTests(unittest.TestCase):
    def test_strxor(self):
        tests = (
            ("makes", "sense", b'\x1e\x04\x05\x16\x16'),
        )
        for w1, w2, ok in tests:
            self.assertEqual(strxor(w1, w2), ok)


if __name__ == '__main__':
    unittest.main()
