# setup
import sys
CRANEPATH = '/opt/cranelab/build/'
CRANEPATH = '/opt/cranelab/build'
if sys.path[-1] != CRANEPATH:
    sys.path.append(CRANEPATH)

import numpy as np
from skippylab.instruments.powersupplies import CAENN1471HV
from skippylab.instruments.climate_chambers import SunChamber
from skippylab.controllers import PrologixUsbGPIBController, ZMQController
from skippylab.controllers import SimpleSocketController
from skippylab.controllers import TelnetController
from skippylab.instruments.patchpannels import Cytec
import CraneLab as cl
import os, os.path
import importlib
importlib.reload(cl)

import time
import numpy as np

#SUNPORT = '/dev/ttyUSB1'
#SUNPORT='/dev/serial/by-id/usb-Prologix_Prologix_GPIB-USB_Controller_PX30FLUZ-if00-port0'
#sunec = SunChamber(PrologixUsbGPIBController(port=SUNPORT),
#                                             publish=False,
#                                             port=SUNPORT)
#
#
#

#rundir = '/home/achim/gaps/lab-data/RUN10'#del digitizer
rundir = "/data/test"

digitizer = cl.setup_digitizer(open(os.path.join(rundir,"config_x.json")))

for i in range(3):
    cl.run_digitizer(digitizer, 10,
                    filename = os.path.join(rundir, "test" + str(i) + ".root"),
                    read_waveforms=False)
    #sunec.ON
    #sunec.monitor_temperatures(target_temp=-47,
    #                       activate=True)
    #sunec.OFF
    

