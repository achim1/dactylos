"""
Offline analysis on waveforms obtained with dactylos
The shaping is done with code from 
https://github.com/alowell-ssl/awlpico
"""
import pylab as p
import numpy as np
import concurrent.futures as fut
import uproot as up
import tqdm
import os
import os.path

from copy import copy

from .utils import get_stripname, plot_waveform

# shaping stuff 
from .shaping import GaussShaper

import hepbasestack as hep 

import dactylos

logger = hep.logger.get_logger(dactylos.LOGLEVEL)

########################################################################

def read_waveform(infile, ch):
    """
    Return waveform data from a rootfile with uproot

    Args:
        infile (str) : filename
        ch (int)     : digitizer channel

    Returns:
        ndarray : numpy array with waveform data. Waveform data is in
                  digitizer channels and of length of the record length
                  as set when configuring the digitizer
    """
    f = up.open(infile)
    data = f.get('ch' + str(ch)).get('waveform').array()
    data = np.array([baseline_correction(k, nsamples=1000)[0] for k in data])
    print (f'Read out {len(data)} events for channel {ch}')
    return ch, data

########################################################################

def baseline_correction(input_pulses, nsamples=2000):
    """
    Do a very simple baseline subtraction. Assume the first nsamples are
    the baseline, and then average over them and subtract it

    Args:
        input (ndarray)    : input waveform

    Keyword Args:
        nsamples (int)     : number of samples for baseline estimation 
                             at the beginning of the input data

    Returns:
        numpy.ndarray      : waveform with a subtracted baseline

    """

    absolute_zero = (2**14)/2
    baseline = input_pulses[:nsamples].mean()
    #print (baseline) 
    #baseline = input[:nsamples].mean() - zero
    #print (f"Baseline calculation gives us {baseline}")
    #return input - np.ones(len(input))*baseline - absolute_zero*np.ones(len(input)), baseline
    return input_pulses - np.ones(len(input_pulses))*baseline, baseline
    #return baseline,(input - baseline)

########################################################################

class WaveformAnalysis(object):
    """
    Allow for the read-in and chaining of multiple files

    """

    def __init__(self, njobs=4,\
                 active_channels=[0,1,2,3,4,5,6,7],\
                 shaper_order=4,\
                 adjust_shaper_order_dynamically=False):
        """
        Prepare the waveform analysis. This is a several step process.
        First, tell it what files to use, then load them and finaly analyze.

        Args:
            njobs (int) : number of jobs to use

        Returns:
            order (int)           :  -- unknown shaping parameter
            decay_time (float)    :  The decay time of the pulse. This has to
                                     pretty much mach the decaying tail of the pulse.
                                     Decay time has to be given in seconds.
            dt (float)            :  Digitizer sample size (that is 1/sampling rate) in 
                                     seconds. This value is fix for the CAEN 6725 with 
                                     250 MSamples/s
            shaper_order (int)    :  Order of the Gaussian shaper (4 is default)
            adjust_shaper_order_dynamically (bool) : Switch to a higher shaper order
                                                     for larger peakingtimes automatically
        """
        self.files = []
        self.tpexecutor = fut.ThreadPoolExecutor(max_workers=njobs)
        self.ppexecutor = fut.ProcessPoolExecutor(max_workers=njobs)
        self.active_channels = active_channels
        self.channel_data = dict([(k, np.array([])) for k in self.active_channels])  
        self.order = shaper_order
        self.adjust_shaper_order_dynamically = adjust_shaper_order_dynamically

    def get_recordlengths(self):
        """
        For debugging purposes. Check the recordlength of the 
        Waveforms and return a set for each channel with the
        recordlengths in the actual waveform data. This might 
        differ from the actual setting for whatever reason. 
        Might be even a bug.

        Returns:
            dict     : ch -> set(recordlength)

        """
        result = dict()
        for ch in self.active_channels:
            rl = np.array([len(k) for k in self.channel_data[ch]])
            result[ch] = set(rl)
        return result

    def get_eventcounts(self):
        """
        Return the number of events per channel. This only makes sense
        after some data has been via read_waveform

        Return:
            dict     : int->int Number of events per channel
        """
        nevents = dict()
        # readjust active channels
        # if one has 0 events, it is not active
        self.active_channels = []
        for ch in self.channel_data.keys():
            nevents[ch] = len(self.channel_data[ch])
            if nevents[ch] > 0:
                self.active_channels.append(ch)
            else:
                logger.warning("Ch {ch} has no observed events! Deactivating...")
        return nevents

    def __del__(self):
        self.tpexecutor.shutdown()
        self.ppexecutor.shutdown()

    def set_shaping_parameters(self, peaktime,
                               order = 4,\
                               decay_time = 80e-6,\
                               dt = 4e-9 ):
        """
        Set all the ncecessary parameters for Alex' gaussian
        shaper in a coherent way. 
        Typically order and decay_time do not need to be changed.

        Args: 
            peaktime (float)      :  The peaking time, that is the time
                                     where the waveform is sampled with 
                                     the gaussian. Peaktime has to be given in 
                                     seconds.

        Keyword Args:
            order (int)           :  -- unknown shaping parameter
            decay_time (float)    :  The decay time of the pulse. This has to
                                     pretty much mach the decaying tail of the pulse.
                                     Decay time has to be given in seconds.
            dt (float)            :  Digitizer sample size (that is 1/sampling rate) in 
                                     seconds. This value is fix for the CAEN 6725 with 
                                     250 MSamples/s
        """
        # peaktime can be e.g. 4e-6, that is 4 microseconds
        self.peaktime = peaktime
        self.order = order
        self.decay_time = decay_time
        self.dt = dt  

    def set_peakingtime_sequence(self, peakingtimes):
        """
        Set a number of peaking times we want to 
        try on each channel

        Args:
            peakingtimes (list)   : peakingtimes in microsec

        """
        self.peakingtime_sequence = peakingtimes

    def set_infiles(self, infiles):
        """
        Define a list of files to be readout later.

        Args:
            infiles (list)   : root files with waveform data
        
        """
        self.files = infiles

    def read_waveforms(self):
        """
        Read the waveforms from the files

        Returns:
            None
        """
        future_to_fname = dict()

        for fname in self.files:
            # read out every file once per channel
            for ch in self.active_channels:
                #read_waveform(fname, ch)
                future_to_fname[self.tpexecutor.submit(read_waveform, fname, ch)] = fname

        logger.info("Readout jobs submitted..")
        init_ch = [False]*8
        for future in fut.as_completed(future_to_fname):
            ch, data = future.result()    
            data = np.array([np.array(k) for k in data])
            if not init_ch[ch]:
                self.channel_data[ch] = data
                init_ch[ch] = True
            else:
                self.channel_data[ch] = np.vstack((self.channel_data[ch], data))
        logger.info("Channel data created")
        return None
        #return channel_data

    def plot_waveforms(self, ch, nwaveforms,\
                       prefix='',\
                       savedir='.'):
        """
        Plot some of the waveforms.

        Args:
            ch (int)              : Plot waveforms of this channel
            nwaveforms (int)      : Plot the first nwaveforms
    
        Keyword Args: 
            prefix (str)          : A prefix to the filename
            savedir (str)         : Save the resulting plot in this directory

        """
        wfplot = p.figure()
        # plot waveforms in a grid, always 3 colums
        lines = int(nwaveforms/3)
        if lines == 0 : lines = 1
        nwaveforms = lines*3

        times = np.array([k for k in range(len(self.channel_data[ch][0]))])
        for i in tqdm.tqdm(range(nwaveforms), total=nwaveforms):
            ax = wfplot.add_subplot(lines, 3, i+1)
            volts = self.channel_data[ch][i]
            ax = plot_waveform(ax, times, volts)     
            # plot the axes only on the leftmost plots
            # and the downmost
            if (i+1 - 3) == 0:
                ax.set_ylabel("d. chan.")
            if (i+1 - nwaveforms) < 4:
                ax.set_xlabel("time [$\mu$s]")
        stripname = get_stripname(ch) 
        if not prefix:
            savename = f'waveforms-{stripname}.png' 
        else:
            savename = f'waveforms-{prefix}-{stripname}.png'
        savename = os.path.join(savedir, savename)
        wfplot.savefig(savename)
        return savename 

    def analyze(self, channel):
        """
        Applyt the gaussian shaping algorithm on the waveform data.
        
        Args:
            channel (int)    : Select the channel

        """
        ptime_energies = dict()
        data = copy(self.channel_data[channel])
        for ptime in self.peakingtime_sequence:
            if self.adjust_shaper_order_dynamically:
                if ptime <= 5000:
                    order = 4
                else:
                    order = 7
            else:
                order = self.order
            shaper = GaussShaper(ptime, order=order)
            energies = np.array([energy for energy in self.ppexecutor.map(shaper.shape_it, data)])
            ptime_energies[ptime] = energies 
        return ptime_energies 

