#! /usr/bin/env python3

import matplotlib.pyplot as py
import numpy as np
from scipy.signal import sosfilt

from shapers import shaper
from sos2c import sos2c

def create_dummy_tailpulses(t, peaktime, dt, decaytime):
    tail = np.zeros_like(t)
    tail[t>0] = np.exp(-t[t>0]/decay_time)
    tail += np.random.normal(scale=.05,size=len(t))
    return tail

def shape_it(t, tail, peaktime, order, decay_time, dt):
    sos = shaper("gaussian",order,peaktime,dt=dt,pz=1./decay_time)
    y = sosfilt(sos,tail) 
    return y    
    py.plot(t*1E6,tail,'k',lw=0.5,label='input tail pulse')
    py.plot(t*1E6,y,'g',label='shaper output')
    py.xlabel('time (microseconds)')
    py.ylabel('amplitude')
    py.legend()
    py.show()
    
    print(sos2c(sos))

if __name__ == '__main__':

    peaktime = 4E-6 #four microseconds
    order = 4 #fourth order filter
    decay_time = 100E-6 # 100 microsecond preamp tail pulse decay time
    dt = 32E-9 # 32 nanosecond sample period
    t = np.arange(-4*peaktime,8*peaktime,dt)

    #print (t)
    #raise
    tail = create_dummy_tailpulses(t,peaktime, dt, decay_time)
    y = shape_it(t, tail,peaktime, order, decay_time, dt)

    py.plot(t*1E6,tail,'k',lw=0.5,label='input tail pulse')
    py.plot(t*1E6,y,'g',label='shaper output')
    py.xlabel('time (microseconds)')
    py.ylabel('amplitude')
    py.legend()
    py.show()



