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

#### A word about DC offsets and trigger thresholds

Setting the DC threshold shifts the offset of the digitizer bins. E.g for a 50% DC offset and a dynamic range 
of 0.5Vpp, 0V will be at 8192. For an offset of 100%, 0V will be at 0 and for an offset of 0% 0V will be 
at 16384. The confusing part here is that the "natural, middle setting" is 50%. 
This also means e.g. a 10mV signal will saturate the digitizer for a 0% DC offset. 
There are 2 different trigger thresholds. One is configured for the digitizer, one per channel, this
is done via `set_channel_trigger_threshold`, the other one is part of the energy filter and 
set through the `DPP_PHA_Params` struct.

* Channel threshold: The waveform has to "reach" a certain bin, baseline dependent

* Trigger threshold in `DPP_PHA_Params`: This trigger threshold is **NOT** a waveform time-over-threshold,
  but is for the RC-CR2 signal. It is basically the minimum energy of the energy filter.
  Thus means it is **independent** of the DC offset, as the DC offset affects the baseline
  and the energy filter is independent of the baseline level.

### Different firmwares

There are different types of firmwares. One is the DPP-XXX firmware, which allows digital pulse processing 
on the digitizer board. The dactylos software supports the DPP-PHA firmware.

The 'standard', free firmware is the waveform recording firmware, this is supported by the dactylos 
software as well [_currently in development_]

