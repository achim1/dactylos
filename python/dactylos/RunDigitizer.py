#! /usr/bin/env python3

import dactylos as dact
import os
import time
import tqdm
import pylab as p
p.ion()
from os.path import join as _j
from skippylab.controllers import PrologixUsbGPIBController

use_chamber = True
test = False
scope = False
shaping_spree  = True
runtime = 240
config = 'config_run10xray.json'
outdir = 'run10-Fr21FEB2020'


if use_chamber:
    from skippylab.instruments.climate_chambers import SunChamber
    SUNPORT='/dev/serial/by-id/usb-Prologix_Prologix_GPIB-USB_Controller_PX30FLUZ-if00-port0'
    sunec = SunChamber(PrologixUsbGPIBController(port=SUNPORT),
                                             publish=False,                                             port=SUNPORT)
try:
    os.mkdir(outdir)
except Exception as e:
    print (e)
    pass


if test:
    digi = dact.CaenN6725(config)
    digi.run_digitizer(10, rootfilename=_j(outdir,'example.root'),\
                   read_waveforms=True)
    del digi
#del digi # closes the connection to the digitzer

if shaping_spree:
    shaping_times = [100, 500, 1000, 1500, 2000, 3000, 5000, 6000, 7000, 10000]
    for stime in tqdm.tqdm(shaping_times):
        print (f"Taking data for shaping time {stime} ..")
        digi = dact.CaenN6725(config, shaping_time=stime)
        digi.run_digitizer(runtime, rootfilename=_j(outdir,f'test{stime}.root'),\
                           read_waveforms=True)
        del digi 
        if use_chamber:
            #cooldown
            print (f".. done. Cooling down!")
            sunec.ON
            sunec.monitor_temperatures(target_temp=-47,
                           activate=True)
            sunec.OFF

if scope:
    try:
        os.mkdir(outdir)
    except Exception as e:
        print (e)
        pass

    digi = dact.CaenN6725(config)
    digi.init_scope()
    while True:
        print ("Getting shaping...")
        digi.oscilloscope(filename = _j(outdir,"shaping.png"))
        print ("Getting trigger...")
        digi.oscilloscope(filename = _j(outdir, "trigger.png"),
                          trace1 = dact.DPPVirtualProbe1.Delta2,\
                          trace2 = dact.DPPVirtualProbe2.Input,\
                          dtrace1 = dact.DPPDigitalProbe1.TRGWin)
        time.sleep(5)
    #digi.oscilloscope(filename = _j(outdir, "test3.png"),
    #                  trace1 = dact.DPPVirtualProbe1.Trapezoid,\
    #                  trace2 = dact.DPPVirtualProbe2.Input,\
    #                  dtrace1 = dact.DPPDigitalProbe1.Armed)
    #digi.oscilloscope(filename = _j(outdir,"test4.png"),
    #                  trace1 = dact.DPPVirtualProbe1.Input,\
    #                  trace2 = dact.DPPVirtualProbe2.TrapezoidReduced,\
    #                  dtrace1 = dact.DPPDigitalProbe1.PkRun)
    #digi.oscilloscope(filename = _j(outdir,"test5.png"),
    #                  trace1 = dact.DPPVirtualProbe1.Input,\
    #                  trace2 = dact.DPPVirtualProbe2.Baseline,\
    #                  dtrace1 = dact.DPPDigitalProbe1.Busy)
    #digi.oscilloscope(filename = _j(outdir,"test6.png"),
    #                  trace1 = dact.DPPVirtualProbe1.Input,\
    #                  trace2 = dact.DPPVirtualProbe2.Threshold,\
    #                  dtrace1 = dact.DPPDigitalProbe1.PileUp)
    #digi.oscilloscope(filename = _j(outdir,"test7.png"),
    #                  trace1 = dact.DPPVirtualProbe1.Input,\
    #                  trace2 = dact.DPPVirtualProbe2.Threshold,\
    #                  dtrace1 = dact.DPPDigitalProbe1.TRGHoldoff)
    


