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
from pathlib import Path

from distutils.version import LooseVersion
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
from setuptools.command.test import test as TestCommand


class CMakeExtension(Extension):
    def __init__(self, name, **kwargs):
        Extension.__init__(self, name, **kwargs)
        if name.startswith('_py'):
            self._cfilename = name + '.so'
        elif name.startswith('_tr'):
            self._cfilename = name + '.so'
        else:
            self._cfilename = 'lib' + name + '.so'

        self.has_explicit_destination = False

    def set_destination_explicit(self, destination):
        """

        """
        pass

class CMakeBuild(build_ext):
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
            '-DPYTHON_EXECUTABLE=/usr/bin/python3', \
            '-DPYTHON_INCLUDE_DIR=/usr/include/python3.6m', \
            '-DPYTHON_LIBRARY=/usr/lib/x86_64-linux-gnu/libpython3.6m.so'
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
            self.move_output(ext)

    def move_output(self, ext):
        build_temp = Path(self.build_temp).resolve()
        print (Path(self.get_ext_fullpath(ext._cfilename)).resolve())
        dest_path = Path(os.path.split(os.path.split(Path(self.get_ext_fullpath(ext._cfilename)).resolve())[0])[0])
        #source_path = build_temp / self.get_ext_filename(ext._cfilename)
        try:
            os.mkdir(dest_path)
        except Exception as e:
            print (e)
        source_path = build_temp / ext._cfilename

        dest_directory = dest_path.parents[0]

        # FIXME : not here
        # we have to branch this in case for the 
        # the analysis library, this should go directly in the python-package
        print (ext.name)
        if ext.name.startswith('_trp') or ext.name.startswith('DactylosAnalysis'):
            dest_directory = dest_directory / 'analysis' / 'dactylos' / 'shaping'
        dest_directory.mkdir(parents=True, exist_ok=True)
        print (f"Trying to copy {ext.name} from {source_path} to {dest_path}")
        try:
            self.copy_file(source_path, dest_path/ext._cfilename)
        except Exception as e:
            print (f"Failed, raised {e}. Trying again..")
            self.copy_file(build_temp/self.get_ext_filename(ext.name), dest_path)        

def get_version(package):
    """
    Return package version as listed in `__version__` in `init.py`.
    """
    with open(os.path.join(package, '__init__.py'), 'rb') as init_py:
        src = init_py.read().decode('utf-8')
        return re.search("__version__ = ['\"]([^'\"]+)['\"]", src).group(1)

version = get_version('dactylos')

tests_require = [
    'pytest>=3.0.5',
    'pytest-cov',
    'pytest-runner',
]

needs_pytest = set(('pytest', 'test', 'ptr')).intersection(sys.argv)
setup_requires = ['pytest-runner'] if needs_pytest else []
setup_requires.append('pybind11>2.4')

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
    """Helper class to determine the pybind11 include path
    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __init__(self, user=False):
        self.user = user

    def __str__(self):
        import pybind11
        return pybind11.get_include(self.user)

# external modules, build by CMake. At the moment this is all 
# double a little bit, this must be also defined in the CMakeList.txt file
# this just helps for the actual install process
ext_modules = [
    CMakeExtension(
        'CaenN6725',
        sources = ['src/CaenN6725.cxx'],
        include_dirs=[
            "include",
            get_root_include_dir()
        ],
        libraries=['CAENDigitizer'],
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
    ),
    CMakeExtension(
        'DactylosAnalysis',
        sources = ['dactylos/analysis/shaping/trapezoidal_shaper.cxx'],
        include_dirs=[
            # Path to pybind11 headers
            get_pybind_include(),
            get_pybind_include(user=True),
            "include",
            "dactylos/analysis/shaping"
        ],
        libraries=['DactylosAnalysis'],
        language='c++'
    ),
    CMakeExtension(
        '_trapezoidal_shaper',
        sources = ['dactylos/analysis/shaping/module.cxx'],
        include_dirs=[
            # Path to pybind11 headers
            get_pybind_include(),
            get_pybind_include(user=True),
            "include",
            "dactylos/analysis/shaping"
        ],
        libraries=['DactylosAnalysis'],
        language='c++'
    )
]


# As of Python 3.6, CCompiler has a `has_flag` method.
# cf http://bugs.python.org/issue26689
def has_flag(compiler, flagname):
    """Return a boolean indicating whether a flag name is supported on
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


def cpp_flag(compiler):
    """Return the -std=c++[11/14/17] compiler flag.
    The newer version is prefered over c++11 (when it is available).
    """
    flags = ['-std=c++17', '-std=c++14', '-std=c++11']

    for flag in flags:
        if has_flag(compiler, flag): return flag

    raise RuntimeError('Unsupported compiler -- at least C++11 support '
                       'is needed!')

#ROOT.gSystem.GetIncludePath() 
# since root might be not in the superusers python path, add it to 
# sys path
sys.path.append(get_root_lib_dir())
import ROOT 

class BuildExt(build_ext):
    """A custom build extension for adding compiler-specific options."""
    c_opts = {
        'msvc': ['/EHsc'],
        #'unix': [ROOT.gSystem.GetIncludePath(), "-I" + pybind11.get_include(), '-std=c++14'],
        'unix': [ROOT.gSystem.GetIncludePath(), '-std=c++14'],
    }
    l_opts = {
        'msvc': [],
        'unix': ['-lCAENDigitizer'],
    }

    if sys.platform == 'darwin':
        darwin_opts = ['-stdlib=libc++', '-mmacosx-version-min=10.7']
        c_opts['unix'] += darwin_opts
        l_opts['unix'] += darwin_opts

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        link_opts = self.l_opts.get(ct, [])
        if ct == 'unix':
            opts.append('-DVERSION_INFO="%s"' % self.distribution.get_version())
            #opts.append(cpp_flag(self.compiler))
            if has_flag(self.compiler, '-fvisibility=hidden'):
                opts.append('-fvisibility=hidden')
        elif ct == 'msvc':
            opts.append('/DVERSION_INFO=\\"%s\\"' % self.distribution.get_version())
        for ext in self.extensions:
            ext.extra_compile_args = opts
            ext.extra_link_args = link_opts
        build_ext.build_extensions(self)



setup(name='dactylos',
      version=version,
      description='Python package to interact and readout CAEN N6725 digitizers. Can access waveform information, energy values of the shapers and allows for easy configuration of the instrument.',
      long_description='This is a private project, and with association of CAEN in any kind. There is no guarantee that this code is useful or working or not harmful. Please see the licensce agreement. This code needs the CAEN C libraries for communication of the digitizer via USB as well as the CAEN C interface library, see https://www.caen.it/products/n6725/.',
      author='Achim Stoessl',
      author_email="achim.stoessl@gmail.com",
      url='https://github.com/achim1/skippylab',
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
    # use the package_data hook to get the pybindings (as compiled with cmake) to the right
    # final destination
    #package_data={'dactylos.analysis.shaping' : ['*.so']},
    #include_package_data=True,
    scripts=['bin/RunDigitizer', 'bin/FitXrayData']
    )
