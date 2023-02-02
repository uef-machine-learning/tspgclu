#!/usr/bin/env python

from distutils.core import setup, Extension

cargs = ['-O3',  '-std=c++11', '-fopenmp', '-fpermissive', '-D_PYTHON_LIB','-Wunused-variable','-Wno-unused-but-set-variable', '-Wno-unused-result', '-Wno-unused-variable']
#For debug:
# cargs = ['-O1', '-g', '-std=c++11', '-fopenmp', '-fpermissive',  '-Wall', '-D_PYTHON_LIB']


module1 = Extension('tspg', sources=['python/py_interf.cpp'], include_dirs=['python','.'], extra_compile_args=cargs)
                        
setup(name = 'tspg',
        version='1.0',
        setup_requires=['wheel'],
        python_requires='>=3',
        requires=['numpy'],
        install_requires=["numpy>=1.9"],
        provides=['tspg'],
       
        description='Fast agglomerative clustering using TSP-graph',
        ext_modules = [module1])

