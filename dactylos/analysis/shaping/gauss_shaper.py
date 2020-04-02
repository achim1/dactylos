"""
Gaussian shaper class primarily for the use with 
dactyos data.
"""
from scipy.signal import sosfilt

from .shapers import shaper

class GaussShaper(object):
    """
    A wrapper for Alex shaper software.

    """
    def __init__(self, peaktime, order=4,\
                 dt=4e-9, decay_time=80e-6):
        """
        
        Args:
            peaktime (float)  : peaking time in nanosecondd

        """
        # remember to convert everything to seconds
        self.sos = shaper("gaussian",order,1e-9*peaktime,dt=dt,pz=1./decay_time)

    def shape_it(self, tailpulses):
        #sos = shaper("gaussian",self.order,self.peaktime,dt=self.dt,pz=1./self.decay_time)
        #pulses = copy(tailpulses)
        #tailpulses_cr, bl = baseline_correction(pulses, nsamples=1024)
        y = sosfilt(self.sos,tailpulses)
        return max(y)

