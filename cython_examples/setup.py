from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

sourcefiles = ['myAverage.pyx']

ext_modules = [
    Extension("myAverage", sourcefiles)]

setup(
    name = "myAverage",
    cmdclass = {'build_ext': build_ext},
    ext_modules = ext_modules
)
