import uproot as up
from dataclasses import dataclass

ENERGY = 'energy'
WAVEFORM = 'waveform'
TRIGGER = 'trigger' 

@dataclass
class ChannelInspector:
    """
    Summary information about a specific channel
    """
    channel_id  : int = 0
    n_energies  : int = 0
    n_waveforms : int = 0
    n_trigger   : int = 0

def inspect_file(filename):
    """
    Gather basic information from a file. Active channels, 
    events per channel etc.

    Args;
        filename (str) : The file to be instpected
    """

    inspector  = dict()
    ch_inspector = dict()
    f = up.open(filename)
    for k in range(8):
        chdata = f.get(f'ch{k}')
        ch_inspect = ChannelInspector()
        ch_inspect.channel_id  = k
        ch_inspect.n_energies  = len(chdata.get(ENERGY).array())
        ch_inspect.n_waveforms = len(chdata.get(WAVEFORM).array())
        ch_inspect.n_trigger   = len(chdata.get(TRIGGER).array())
        inspector[k] = ch_inspect
    return inspector
