#!/usr/bin/env python
import numpy

# from distutils.core import setup, Extension
import setuptools
from setuptools import setup, Extension
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext


__version__ = "0.1"



# packages=setuptools.find_packages(include=['tspg', 'tspg.*']),

cargs = ['-O3',  '-std=c++11', '-fopenmp', '-fpermissive', '-D_PYTHON_LIB','-Wunused-variable','-Wno-unused-but-set-variable', '-Wno-unused-result', '-Wno-unused-variable']
#For debug:
# cargs = ['-O1', '-g', '-std=c++11', '-fopenmp', '-fpermissive',  '-Wall', '-D_PYTHON_LIB']

with open('README.md', 'r', encoding='utf-8') as f:
    long_description = f.read()

module1 = Extension('tspg', sources=['python/py_interf.cpp'], include_dirs=['python','.',numpy.get_include()], extra_compile_args=cargs)
                        # pyximport.install(setup_args={"script_args":["--compiler=mingw32"],
                              # },
                        
	    # include_dirs=[".",numpy.get_include()],
     # install_requires=[
        # 'numpy>=1.9',
        # # Add more required dependencies
    # ],

# ext_modules = [
    # Pybind11Extension("tspgc",
        # ["src/main.cpp"],
	    # include_dirs=[".",numpy.get_include()],
        # # Example: passing in the version to the compiled code
        # define_macros = [('VERSION_INFO', __version__)],
        # ),
# ]

ext_modules = [module1]
# ext_modules = [
    # Extension("tspgc",
        # sources=["src/main.cpp"],
	    # include_dirs=[".",numpy.get_include()],
        # # Example: passing in the version to the compiled code
        # define_macros = [('VERSION_INFO', __version__)],
        # ),

                       #                       
                        
# The TSPg software approximates Ward's agglomerative clustering
                      
setup(
    name='tspg',
    version='1.0',
    # packages=setuptools.find_packages(include=['tspg', 'tspg.*']),
    setup_requires=['wheel'],
    python_requires='>=3',
    provides=['tspg'],
    description='Fast agglomerative clustering using TSP-graph',
    long_description=long_description,
    long_description_content_type='text/markdown',
    classifiers=[
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        # Add more relevant classifiers
    ],
    extras_require={"test": "pytest"},
    ext_modules=ext_modules

    # ext_modules=[module1]
)
                        
# setup(name = 'tspg',
        # version='1.0',
        # setup_requires=['wheel'],
        # python_requires='>=3',
        # requires=['numpy'],
        # install_requires=["numpy>=1.9"],
        # provides=['tspg'],
        # include_dirs=[numpy.get_include()],
        # description='Fast agglomerative clustering using TSP-graph',
        # ext_modules = [module1])

