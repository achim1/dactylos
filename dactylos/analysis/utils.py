import pylab as p


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


