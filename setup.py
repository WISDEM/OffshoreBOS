#!/usr/bin/env python
# encoding: utf-8

from setuptools import setup
from numpy.distutils.core import setup, Extension
import os, platform


if platform.system() == 'Windows':
    arglist = ['-std=gnu++11','-fPIC']
else:
    arglist = ['-std=c++11','-fPIC']

setup(
    name='OffshoreBOS',
    version='1.0',
    description='Offshore balance of station model',
    author='NREL WISDEM Team',
    author_email='systems.engineering@nrel.gov',
    package_dir={'': 'src'},
    py_modules=['offshorebos'],
    package_data={'offshorebos': []},
    packages=['offshorebos'],
    license='Apache License, Version 2.0',
    ext_modules=[Extension('lib_wind_obos', ['src/offshorebos/lib_wind_obos.cpp',
                                             'src/offshorebos/lib_wind_obos_cable_vessel.cpp',
                                             'src/offshorebos/lib_wind_obos_defaults.cpp'],
                           extra_compile_args=arglist)],
    zip_safe=False
)
