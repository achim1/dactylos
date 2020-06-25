"""
Helper functions for getting data, plotting etc.

"""

import pylab as p

from dataclasses import dataclass

@dataclass
class Measurment:
    """
    A simple container holding spectral fit results
    """
    ptime         : int
    strip         : str
    mtype         : str
    fwhm          : float
    fwhme         : float
    ptimefromfile : int = -1
    filename      : str = ''
    statistics    : int = -1

#################################################################

def get_data_from_summaryfile(sfile, mtype):
    """
    Extract the data from a summaryfile written by FitXrayData

    Args:
        sfile (file descriptor)  : open summaryfile
        mtype (str)              : descriptive string about the 
                                   measurement type. Can be either
                                   'WF'   - waveform analysis
                                   'CAEN' - online CAEN energy analysis
                                   'MC2'  - Data taken with MC2
    """

    data = []
    for line in sfile:
        if line.startswith('#'):
            continue
        line = line.split()
        m = Measurment(ptime = float(line[2]),\
                       mtype = mtype,\
                       strip = line[1],\
                       fwhm  = float(line[3]),\
                       fwhme = float(line[4]))
        data.append(m)
    return data

########################################################################

def get_data_from_mit_file(sfile):
    """
    Extract MC2 data from ascii file format used by MIT
    This will always be MC2 data.

    Args:
        sfile (file descriptor)  : input ascii file
    """
    data = []
    for line in sfile:
        if line.startswith('#'):
            continue
        line = line.split()
        m = Measurment(ptime = float(1000*float(line[2])),\
                       mtype = 'MC2',\
                       strip = f'strip{line[1]}',\
                       fwhm  = float(line[5]),\
                       fwhme = float(line[6]))
        data.append(m)
    return data

########################################################################

def get_stripname(ch):
    """ 
    Mapping channel number <-> strip name
    
    Args:
        ch (int) : channel number [0-7]

    Returns
        str      : strip name

    """
    channel_names = {\
    0 : 'stripA',\
    1 : 'stripB',\
    2 : 'stripC',\
    3 : 'stripD',\
    4 : 'stripE',\
    5 : 'stripF',\
    6 : 'stripG',\
    7 : 'stripH'}
    return channel_names[ch]


########################################################################


def plot_waveform(ax, times, digi_channels):
    """
    Create a plot for the individual waveform, set labels assume
    times is in microseconds and voltages are mV.

    Args:
        times (ndarray)                : Times in microseconds
        voltages (ndarrya)             : Waveform in milliVolts

    Returns:
        None
    """
    ax.plot(times, digi_channels, lw=0.9, color="r", alpha=0.7, label='waveform')
    return ax


