from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

sourcefiles = ['mySMBus.pyx']
ext_modules = [Extension("mySMBus", sourcefiles)]

setup(
 name = 'mySMBus',
 cmdclass = {'build_ext': build_ext},
 ext_modules = ext_modules
 )
 
 


