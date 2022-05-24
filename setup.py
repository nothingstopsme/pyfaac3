from distutils.core import setup, Extension

pyfaac3 = Extension('pyfaac3', sources = ['src/pyfaac3.c'], libraries=["faac"])

setup (
    name = 'pyfaac3',
    version = '1.0',
    ext_modules = [pyfaac3]
)
