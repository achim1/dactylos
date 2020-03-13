# CraneLab - interactively control the GAPS Si(Li) testing and calibration facility

The GAPS project utilizes a Si(Li) tracker, its individual detector are large area, hight temperature, segmented silicon wafers with drifted lithium.

One of the efforts of the GAPS calibration is the development, testing and calibration of such detectors. This software might help especially for the effort of mass calibration of around 1000 detectors for the final flight configuration of GAPS.

The software is meant as a tool to help calibrating GAPS detectors with the least amount of personal labor, and also with the least amount of knowledge about the calibration process itself.

## Requirements

The software reiles on:

* the frienldy operatirng system with the cute little penguin.

* all instruments to be controlled have to be connected to the server which runs cranelab either via USB or Ethernet. For GPIB connections, a Prologix controller is prefreed, but the software might support a model from NI as well, however this requires further drivers.

* for USB connections: It is recommended to set up udev ruels, so that the ports of the USB connection under '/dev' do not change all the time.

* python3, django, monodb, djongo ( a django module to interact with mongo db )

* a function wsgi setup and a http, e.g. nginx. For development, this is not necessary, sicne django comes with its own development server.

* additonal software to control the individual components. Most importantly, this is [skippylab](https://github.com/achim1/skippylab) and [dactylos](https://github.com/achim1/dactylos). 

* for the control of CAEN instruments (e.g. N6725 digitizer) the **necesary drivers and libraries** need to be installed.

* for analysis, ROOT is required. It is also a requirment for `dactylos`

## Running it

The application runs entirely from a browser and should be more or less self explanatory. There are a few concepts noteworthy:

* A "run" - defined by the user with a specific configfile, which instructs `cranelab` to perform a sequence of events, e.g. cooldown, data-taking, warmup...

* Database support - all important information go in a mongodb database (e.g. runs)

* Analysis - `cranelab` comes with the necessary analysis scripts

* Monitoring - A dashboard style overview will provide necessary informatinos during the run 
