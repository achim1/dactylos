## dactylos - python interface to CAEN N6725 digitizer

dactylos? - *dactylus* is greek for finger, which is digit in latin ... 

### General outline of the software

The package provides an interface to configere the CAEN N6725 digitizer via a .json config file and functions to read out data and save energy and waveforms. It is written in C++ utiilizing the CAEN C library which is published by CAEN. On top of the C++ code lives a thin layer of *pybind11* code to provide the python module. 
