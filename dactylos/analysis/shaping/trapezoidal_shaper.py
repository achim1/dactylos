
import numpy as np
from copy import deepcopy as copy
import time
#peaktime = 1.0 # micro sec
#flat = 1.0 # micro sec
#nramp = peaktime/4e-3 # 4 ns per sample
#nflat = flat/4e-3 
#ntot = 2*nramp+nflat
#
#def trapezoidal_filter(waveform,
#                       ptime = 1.0,\
#                       flat  = 1.0,\
#                       nramp = 1./4e-3,\
#                       ntot  = 2*(1/4e-3) + 1/4e-3):
class TrapezoidalFilter(object):

    def __init__(self,\
                 ptime = 1000,\
                 flat  = 1000,
                 recordlength = 50000):
        """
        Keyword Args:
            ptime  (float)         :
            flat   (float)         :
            recordlength (int)     : waveform recordlength for preparing of the shaper windows
        """
        self.ptime = ptime
        self.flat  = flat
        self.nflat = int(flat/4)
        self.nramp = int(ptime/4) 
        self.ntot  = int(2*self.nramp + self.nflat)
        i_s = np.arange(self.ntot,recordlength,1)
        j_s = np.arange(0, self.nramp, 1)
        self.windows  = np.array([[i - j for j in j_s] for i in i_s])

    def shape_it(self, waveform):
        """
        Apply Mengjiao's simple trapezoidal filter

        Args:
            waveform (np.ndarray)  :  input waveform, baseline corrected 

        """
        amp_filter = np.array([(waveform[index] - waveform[index - self.nramp - self.nflat]).sum()/self.nramp for index in self.windows], dtype=np.int16)

        #amp_filter = np.array([waveform[riseindex].sum() + waveform[downindex].sum() for (riseindex, downindex) in zip(k,l)])
        # l   = i_s - (np.ones(len(i_s))*self.nramp) + (np.ones(len(i_s))*self.nflat) - j_s
        #sum_rise = waveform[k].sum()
        #sum_down = waveform[l].sum()
        #amp_filter = sum_rise + sum_down
        energy = max(amp_filter)
        return energy
        ##for i in range(self.ntot, len(waveform)):

        #    #j = 0
        #    #while j < self.nramp:
        #    for j in range(self.nramp):
        #        thisindex1 = i-j 
        #        thisindex2 = i - (self.nramp + self.nflat) - j 
        #        sum_rise += waveform[thisindex1]/self.nramp
        #        sum_down -= waveform[thisindex2]/self.nramp
        #        #j += 1     

        #    ##j = 0
        #    ##while j < self.nramp:
        #    #for j in range(self.nramp):
        #    #    thisindex = i - (self.nramp + self.nflat) - j 
        #    #    sum_down -= waveform[thisindex]/self.nramp
        #    #    #j += 1
        #    ampfilter = sum_rise + sum_down
        #    if ampfilter > energy:
        #        energy = ampfilter
        #    i += 1
        print (energy)
        return energy

#        for(int j=ntot;j<waveform->size();j++)
#        {   
#          cout<<j<<" "<<(*waveform)[j]<<endl;
#            hraw->Fill(j, (*waveform)[j]-baseline);
#
#            double amp_filter=0.;
#            double sum_rise=0.;
#            double sum_down=0.;
#
#            for(int ii=0;ii<nramp;ii++)
#            {
#                sum_rise += ((*waveform)[j-ii]-baseline)*1/(nramp*1.0);
#            }
#            for(int jj=0; jj<nramp;jj++)
#            {
#                sum_down += ((*waveform)[j-(nramp+nflat)-jj]-baseline)*(-1/(nramp*1.0));
#            }
#            amp_filter = sum_rise + sum_down;
#
#            if(amp_filter>energy)
#            {
#                energy = amp_filter;  //energy calculation, need to be improved
#            }
#
#            hfilter->Fill(j, amp_filter);
#        }

