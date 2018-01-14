from distutils.core import setup
from Cython.Build import cythonize

setup(
    name='meteor_cython',
    ext_modules=cythonize("meteor_cython.pyx"),
)
