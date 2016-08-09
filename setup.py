from distutils.core import setup, Extension


cp_otp_mod = Extension('cp_otp', sources=['cp_otp.c'])

setup(name="cp_otp",
      version="0.1",
      description="Cyber Practicals One-Time Pad Extension Library",
      ext_modules=[cp_otp_mod])
