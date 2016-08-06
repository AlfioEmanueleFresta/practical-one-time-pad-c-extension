import unittest
from cp_otp import strxor, intercept_in, intercept_out


class CPOTOPTests(unittest.TestCase):
    def test_strxor(self):
        tests = (
            ("makes", "sense", b'\x1e\x04\x05\x16\x16'),    # Normal strings
            ("\x00\x00", "\x00\x01", b'\x00\x01'),          # Null bytes
        )
        for w1, w2, ok in tests:
            self.assertEqual(strxor(w1, w2), ok)

    def test_encrypted_message_does_not_contain_null_characters(self):
        # The problem here is that the encrypted message, once XOR'd with the
        # secret key, may contain a NULL character, therefore screwing up with
        # the way C handles strings. We want to make sure this does not happen.
        no_times = 500000  # *Really* sure.
        for _ in range(no_times):
            try:
                intercept_out(intercept_in(silent=True), silent=True)
            except ValueError:
                self.fail()

    def test_intercept(self):
        self.assertFalse(
            intercept_out(intercept_in(silent=True),
                          silent=True)
        )

    def test_intercept_false(self):
        self.assertFalse(
            intercept_out(intercept_in(silent=True),
                          silent=True)
        )

    def test_intercept_true(self):
        msg = intercept_in(silent=True)
        diff = b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'\
               b'\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'\
               b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00'
        msg = strxor(diff, msg)
        self.assertTrue(
            intercept_out(msg, silent=True)
        )


if __name__ == '__main__':
    unittest.main()
