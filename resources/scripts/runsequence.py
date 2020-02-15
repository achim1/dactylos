# setup
import sys
CRANEPATH = '/opt/cranelab/CraneLab-build/'
CRANEPATH = '/home/achim/gaps/CraneLab/build'
if sys.path[-1] != CRANEPATH:
    sys.path.append(CRANEPATH)

import numpy as np
from skippylab.instruments.powersupplies import CAENN1471HV
from skippylab.instruments.climate_chambers import SunChamber
from skippylab.controllers import PrologixUsbGPIBController, ZMQController
from skippylab.controllers import SimpleSocketController
from skippylab.controllers import TelnetController

from skippylab.instruments.patchpannels import Cytec
import pyCaenN6725

import time
import numpy as np

SUNPORT = '/dev/ttyUSB1'
SUNPORT='/dev/serial/by-id/usb-Prologix_Prologix_GPIB-USB_Controller_PX30FLUZ-if00-port0'
sunec = SunChamber(PrologixUsbGPIBController(port=SUNPORT),
                                             publish=False,
                                             port=SUNPORT)



import CraneLab as cl
import os, os.path
import importlib
importlib.reload(cl)

rundir = '/home/achim/gaps/lab-data/RUN10'#del digitizer
digitizer = cl.setup_digitizer(open(os.path.join(rundir,"config_x.json")))

for i in range(10):
    cl.run_digitizer(digitizer, 60,
                    filename = os.path.join(rundir, "test" + str(i) + ".root"),
                    read_waveforms=True)
    print (digitizer.get_n_events_tot())
    sunec.ON
    sunec.monitor_temperatures(target_temp=-47,
                           activate=True)
    sunec.OFF
    

