from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize
from Cython.Distutils import build_ext

sourcefiles = ['smbus.pyx']
ext_modules = [Extension("smbus", sourcefiles)]

setup(
 name = 'New'
 cmdclass = {'build_ext': build_ext},
 ext_modules = ext_modules
 }
 
 


