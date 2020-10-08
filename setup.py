"""
setup.py file for dactylos - CAEN N6725 digitizer read out software.
This setup.py file will compile the C++ extensions needed to interact
with the digitizer with the help of an additional CMakeLists.txt file.
"""

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

import os.path, re, sys
#import pybind11

import os
import re
import sys
import sysconfig
import platform
import subprocess
import shlex
from pathlib import Path
from glob import glob

from distutils.version import LooseVersion
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
from setuptools.command.test import test as TestCommand

def get_version(package):
    """
    Return package version as listed in `__version__` in `init.py`.
    """
    with open(os.path.join(package, '__init__.py'), 'rb') as init_py:
        src = init_py.read().decode('utf-8')
        return re.search("__version__ = ['\"]([^'\"]+)['\"]", src).group(1)

VERSION = get_version('dactylos')

tests_require = [
    'pytest>=3.0.5',
    'pytest-cov',
    'pytest-runner',
]

needs_pytest = set(('pytest', 'test', 'ptr')).intersection(sys.argv)
setup_requires = ['pytest-runner'] if needs_pytest else []
setup_requires.append('pybind11>2.4')

#####################################################
# Get dependnecies 
# ROOT lib/include dirs

def get_root_include_dir():
    rootsys = os.getenv('ROOTSYS')
    if rootsys is None:
        raise SystemError("$ROOTSYS shell variable not defined! Make sure to have root installed end this variable defined.")
    print (f'Found root include dir at {rootsys}/include')
    return os.path.join(rootsys, 'include')

def get_root_lib_dir():
    rootsys = os.getenv('ROOTSYS')
    if rootsys is None:
        raise SystemError("$ROOTSYS shell variable not defined! Make sure to have root installed end this variable defined.")
    print (f'Found root include dir at {rootsys}/lib')
    return os.path.join(rootsys, 'lib')

class get_pybind_include(object):
    """
    Helper class to determine the pybind11 include path
    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. 
    """

    def __init__(self, user=False):
        self.user = user

    def __str__(self):
        import pybind11
        return pybind11.get_include(self.user)


def get_root_cxx_standard():
    """
    Invoce root-config to find out which cxx standard was used
    to compile root
    """
    command = shlex.split('root-config --cflags')
    result = subprocess.Popen(command, stdout=subprocess.PIPE).communicate()[0]
    cxxstandard = re.compile('-std=c\+\+(?P<std>[0-9]+)')
    try:
        cxxstandard = cxxstandard.search(result.decode()).groupdict()['std']
    except Exception as e:
        print(f'Can not retrieve uxed C++ standard used when compiling root, exceoption {e}')
    return cxxstandard


# since root might be not in the superusers python path, add it to 
# sys path
sys.path.append(get_root_lib_dir())
import ROOT 

#####################################################

# As of Python 3.6, CCompiler has a `has_flag` method.
# cf http://bugs.python.org/issue26689
def has_flag(compiler, flagname):
    """
    Return a boolean indicating whether a flag name is supported on
    the specified compiler.
    """
    import tempfile
    with tempfile.NamedTemporaryFile('w', suffix='.cpp') as f:
        f.write('int main (int argc, char **argv) { return 0; }')
        try:
            compiler.compile([f.name], extra_postargs=[flagname])
        except setuptools.distutils.errors.CompileError:
            return False
    return True

#####################################################

class CMakeExtension(Extension):
    """
    Allows to install extensions with the
    help of an external CMakeLists.txt file
    """

    def __init__(self, name, **kwargs):
        Extension.__init__(self, name, **kwargs)
        print (f'Found extension with neame {name}')
        if name.startswith('_py'):
            self._cfilename = name# + '.so'
        elif name.startswith('_tr'):
            self._cfilename = name# + '.so'
        else:
            self._cfilename = 'lib' + name# + '.so'
        print (f'library name {self._cfilename}')
        self.has_explicit_destination = False


class CMakeBuild(build_ext):
    """
    Invoke cmake and an external CMakeLists.txt file to 
    build the external components

    """

    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError(
                "CMake must be installed to build the following extensions: " +
                ", ".join(e.name for e in self.extensions))

        build_directory = os.path.abspath(self.build_temp)

        cmake_args = [
            '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + build_directory,
            #'-DPYTHON_EXECUTABLE=' + sys.executable
            #'-DPYTHON_EXECUTABLE=/usr/bin/python3', \
            #'-DPYTHON_INCLUDE_DIR=/usr/include/python3.6m', \
            #'-DPYTHON_LIBRARY=/usr/lib/x86_64-linux-gnu/libpython3.6m.so'
            f'-DCMAKE_CXX_STANDARD={get_root_cxx_standard()}'\
        ]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]

        # Assuming Makefiles
        build_args += ['--', '-j2']

        self.build_args = build_args

        env = os.environ.copy()
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(
            env.get('CXXFLAGS', ''),
            self.distribution.get_version())
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        # CMakeLists.txt is in the same directory as this setup.py file
        cmake_list_dir = os.path.abspath(os.path.dirname(__file__))
        print('-'*10, 'Running CMake prepare', '-'*40)
        subprocess.check_call(['cmake', cmake_list_dir] + cmake_args,
                              cwd=self.build_temp, env=env)

        print('-'*10, 'Building extensions', '-'*40)
        cmake_cmd = ['cmake', '--build', '.'] + self.build_args
        subprocess.check_call(cmake_cmd,
                              cwd=self.build_temp)

        # Move from build temp to final position
        #print (self.extensions)
        

        for ext in self.extensions:
            build_temp = Path(self.build_temp).resolve(ext.name)
            print (f'Found build dir {build_temp}')        
            source_file = os.path.join(build_temp, ext._cfilename)
            print (f'Looking for source file {source_file}' + '*.so')
            source_files = glob(source_file + "*")
            if not source_files:
                print (f'WARN: {source_file}*.so not found...!') 
                continue
                #raise SystemError("Can not find source file!")
            source_file = source_files[0]
            # destination is the directory in the install path (hopefully)
            dest_path = os.path.split(Path(self.get_ext_fullpath(ext._cfilename)).resolve())[0] 
            dest_path = Path(dest_path) / 'dactylos' 
            if ext._cfilename.startswith('_tr'):
                # put trapezoidal shaper library in analysis
                dest_path = dest_path / 'analysis' / 'shaping'
            print (f'.. will copy to {dest_path}')
            print (f"Trying to copy {ext.name} from {source_file} to {dest_path}")
            self.copy_file(source_file, dest_path)
        # copy the dactylos library
            

# external modules, build by CMake. At the moment this is all 
# double a little bit, this must be also defined in the CMakeList.txt file
# this just helps for the actual install process
ext_modules = [
    CMakeExtension(
        'Dactylos',
        sources = ['src/trapezoidal_shaper.cxx',
                   'src/CaenN6725.cxx'],
        include_dirs=[
            # Path to pybind11 headers
            #get_pybind_include(),
            #get_pybind_include(user=True),
            "include",
            get_root_include_dir()
        ],
        libraries=['Dactylos'],
        language='c++'
    ),
    CMakeExtension(
        '_pyCaenN6725',
        sources = ['src/module.cxx'],
        include_dirs=[
            # Path to pybind11 headers
            get_pybind_include(),
            get_pybind_include(user=True),
            "include",
            get_root_include_dir()
        ],
        libraries=['CAENDigitizer','CaenN6725'],
        language='c++'
    )
]



setup(name='dactylos',
      version=VERSION,
      description='Python package to interact and readout CAEN N6725 digitizers. Can access waveform information, energy values of the shapers and allows for easy configuration of the instrument.',
      long_description='This is a private project, and without association of CAEN in any kind. There is no guarantee that this code is useful or working or not harmful. Please see the licensce agreement. This code needs the CAEN C libraries for communication of the digitizer via USB as well as the CAEN C interface library, see https://www.caen.it/products/n6725/.',
      author='Achim Stoessl',
      author_email="achim.stoessl@gmail.com",
      url='https://github.com/achim1/dactylos',
      #download_url="pip install skippylab",
      install_requires=['numpy>=1.11.0',
                        'matplotlib>=1.5.0',
                        'six>=1.1.0',
                        'pybind11>2.4'],
    ext_modules=ext_modules,
    #cmdclass={'build_ext': BuildExt},
    zip_safe=False,
    setup_requires=setup_requires,
    #tests_require=tests_require,
    license="GPL",
    platforms=["Ubuntu 18.04"],
    cmdclass=dict(build_ext=CMakeBuild),
    classifiers=[
      "License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)",
      "Development Status :: 3 - Alpha",
      "Intended Audience :: Science/Research",
      "Intended Audience :: Developers",
      "Programming Language :: Python :: 3.6",
      "Topic :: Scientific/Engineering :: Physics"
            ],
    keywords=["digitzer", "CAEN",\
              "CAEN N6725", "6725",\
              "readout", "physics", "engineering", "lab", "USB"],
    packages=['dactylos', 'dactylos.analysis', 'dactylos.analysis.shaping'],
    # FIXME: put the header for the trapezoidal shpaer somewhere else
    #data_files = [('.', ['CMakeLists.txt']),
    #             ],
                 # ('dactylos/analysis/shaping/', ['trapezoidal_shaper.h'])],
    # use the package_data hook to get the pybindings (as compiled with cmake) to the right
    # final destination
    #package_data={'dactylos.analysis.shaping' : ['*.so']},
    #include_package_data=True,
    scripts=['bin/RunDigitizer', 'bin/FitXrayData']
    )
