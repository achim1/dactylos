#! /usr/bin/env python3

import dactylos as dact
import os
from os.path import join as _j

try:
    os.mkdir(outdir)
except Exception as e:
    print (e)
    pass

scope = True
shaping_spree  = False
outdir = 'digi-test'

digi = dact.CaenN6725('config_x.json')
digi.run_digitizer(10, rootfilename=_j(outdir,'test.root'),\
                   read_waveforms=True)

del digi # closes the connection to the digitzer

if shaping_spree:
    shaping_times = [100, 500, 1000, 1500, 2000, 3000, 5000]
    for stime in shaping_times:
        digi = dact.CaenN6725('config_x.json', shaping_time=stime)
        digi.run_digitizer(30, rootfilename=_j(outdir,f'test{stime}.root'),\
                           read_waveforms=True)
        del digi 

if scope:
    try:
        os.mkdir(outdir)
    except Exception as e:
        print (e)
        pass

    digi = dact.CaenN6725('config_x.json')
    digi.oscilloscope(filename = _j(outdir,"shaping.png"))
    digi.oscilloscope(filename = _j(outdir, "trigger.png"),
                      trace1 = dact.DPPVirtualProbe1.Delta2,\
                      trace2 = dact.DPPVirtualProbe2.Input,\
                      dtrace1 = dact.DPPDigitalProbe1.TRGWin)
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
    


