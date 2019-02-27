from setuptools import setup

from risingsun import __version__

setup(name='risingsun',
      version=__version__,
      description='Readout a GU-3001D Milli-Gauss meter via USB/serial port on a linux system',
      long_description='Monitor and control a SUN EC13 climate chamber',
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
      keywords=["GPIB", "control",\
                "SUN EC13", "control"],
      packages=['risingsun'],
      scripts=["bin/risingsun"],
      package_data={'gui': ['risingsun.mplstyle']}
      )
