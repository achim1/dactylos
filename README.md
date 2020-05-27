## dactylos - python interface to CAEN N6725 digitizer

dactylos? - *dactylus* is greek for finger, which is digit in latin ... 

## CAEN N6725 digitizer API

CAEN provides a well written, functional style C library to interact with its digitizers. However, 
especially for the rapid development of lab environments, this might be a bit of a hindrence since
it is comprehensive, thus requires some time to accquaint oneself with the specific features of the
digitizer. This library - especially since it has pybindings - allows for quick setups of this CAEN
digitizer in the lab and obtaining first results.

* Configure the digitizer via `.json` config file

* Save data to `.root` files

* Allows to save the waveforms in root files as well

* Set trigger threshold in milliVolts or digitizer bins

* Basica analysis capabilities for energy spectra

**CAVEAT - the CAEN N6725 is a multifunctional instrument. This software allows to access a fraction
of its functionality. It is not guaranteed, that data taken with this software looks either sane nor
as clean/low noise as it is in principle possible**

**CAVEAT - it is not guaranteed that this software is neither harmful nor useful (See the attached GPL licensce**

### Requirements

* CAEN Digitizer libraries/drivers

* pybind11

* The root analysis package from CERN (modern root > 6.00 recommended)

* Cxx 11

### Installation

The build can be either performed with `CMake` or the shipped `setup.py` file. The `setup.py` method will 
invoke cmake, but for more control, `cmake` can be called directly as well

### Usage

Two binaries are provided, one for data-taking and another one for analysis of a (possible X-ray) spectrum




