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

