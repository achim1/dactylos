from setuptools import setup

from risingsun import __version__

setup(name='hvcontrol',
      version=__version__,
      description='Control the CAEN N1471ET HV power supply',
      long_description='Set and monitor voltage ramp ups for taking IV curves',
      author='Achim Stoessl',
      author_email="achim.stoessl@gmail.com",
      #url='https://github.com/achim1/goldschmidt',
      #download_url="pip install pyosci",
      install_requires=['numpy>=1.11.0',
                        'matplotlib>=1.5.0',
                        'pyzmq>=16.0.2',
                        'pyvisa>=1.9.1'],
      license="GPL",
      platforms=["Ubuntu 14.04","Ubuntu 16.04","Ubuntu 18.04"],
      classifiers=[
        "License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)",
        "Development Status :: 4 - Beta",
        "Intended Audience :: Science/Research",
        "Intended Audience :: Developers",
        "Programming Language :: Python :: 3.5",
        "Topic :: Scientific/Engineering :: Physics"
              ],
      keywords=["serial", "control",\
                "CAEN N1471ET", "voltage", "iv curve"],
      packages=['clhvcontrol'],
      #scripts=["bin/hvcontrol"],
      #package_data={'gui': ['risingsun.mplstyle']}
      )
