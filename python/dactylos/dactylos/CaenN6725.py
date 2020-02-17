import hjson

import _pyCaenN6725 as _cn

class CaenN6725(object):
    """
    A digitizer from CAEN. Model name N6725.

    Args:
        configfile (str) : Path to a .json configfile. See example config file
                           in this package
    """
    def __init__(self, configfile):
        self.digitizer = None
        self.configfile = configfile
        self.setup()

    @staticmethod
    def baseline_offset_percent_to_val(percent):
        """
        Convert the value of the baseline offset from 
        percentage to something the digitizer can understand
        """
        val = int(((2**16)/100)*percent)
        print (f"Calculated dc offset of {val}")
        return val


    def setup(self):
        config = open(self.configfile)
        # get the configuration
        config = hjson.load(config)

        # get the active channels
        active_digitizer_channels = config['CaenN6725']['active-channels']
        
        # baseline offset for active digitizier channels
        digitizer_baseline_offset = config['CaenN6725']['baseline-offset']
        assert len(digitizer_baseline_offset) == len(active_digitizer_channels), "The number of active channels does not match the number of baseline offsets give!"

        # dynamic range 
        digitizer_dynamic_range = config['CaenN6725']['dynamic-range']
        assert (digitizer_dynamic_range == "2VPP") or (digitizer_dynamic_range == "05VPP"), "Dynamic range has to be either 2VPP (2 volt peak-peak) or 05VPP (0.5 volt peak-peak"
        if digitizer_dynamic_range == "2VPP":
            digitizer_dynamic_range = _cn.DynamicRange.VPP2
        if digitizer_dynamic_range == "05VPP":
            digitizer_dynamic_range = _cn.DynamicRange.VPP05
        
        

        # as an example, for now just take data with the digitzer
        digi_pars = self.extract_digitizer_parameters(config['CaenN6725'])
    
        self.digitizer = _cn.CaenN6725(digi_pars)
        bf = self.digitizer.get_board_info()
        print (f'Connected to digitizer model {bf.get_model_name()}, roc firmware {bf.get_model_roc_firmware_rel()},  amc firmware {bf.get_amc_firmware_rel()}')

        # set input dynmaic range for all channels
        self.digitizer.set_input_dynamic_range(digitizer_dynamic_range)

        # set baseline offset
        # baseline offset has to go extra, for some reason CAEN treats this separatly
        for i,ch in enumerate(active_digitizer_channels):
            offset = self.baseline_offset_percent_to_val(digitizer_baseline_offset[i]) 
            self.digitizer.set_channel_dc_offset(ch,offset)
            # configure each channel individually
            dpp_params = self.extract_dpp_pha_parameters(ch, config['CaenN6725'], offset) 
            self.digitizer.configure_channel(ch, dpp_params)

        for ch, val in enumerate(self.digitizer.get_temperatures()):
            print (f'Chan: {ch} -  {val}\N{DEGREE SIGN}C')
        print ("Will calibrate the digitizer")
        self.digitizer.calibrate()
        self.digitizer.allocate_memory()
        return 

    @staticmethod
    def extract_digitizer_parameters(config):
        """
        Extract the general configuration parameters
        for the digitizer from a config file 
        read by json
        """
        pars               = _cn.DigitizerParams()
        # the enums we fix forcn
        pars.LinkType      = _cn.ConnectionType.USB
        pars.IOlev         = _cn.IOLevel.NIM
        pars.PulsePolarity = _cn.PulsePolarity.Positive
        pars.DPPAcqMode    = _cn.DPPAcqMode.Mixed
        active_channels    = config['active-channels']
        channelmask        = 0
        for ch in active_channels:
            channelmask += (1 << ch)
        pars.ChannelMask = channelmask
        pars.VMEBaseAddress = config['VMEBaseAddress']
        pars.RecordLength = config['RecordLength']
        pars.EventAggr = config['EventAggr']
        return pars

    @staticmethod
    def extract_dpp_pha_parameters(channel, config,\
                                   dc_offset,
                                   dynamic_range=[-0.5, 0.5]):
        """
        Extract the config parameters for the DPP-PHA
        algorithm from a config file 
        read by json
        
        Args:
            channel (int)    : channel number (0-7)
            config (dict)    : parsed config file
            dc_offset (list) : the set channel dc offset. This is necessary to calculate the trigger leve
                               the channel_dc need to be given in digitizer bins
        Keyword Args:
            dynamic_range (tuple) : V_pp dynamic range [min, max] in Volt

        """
        pars = _cn.DPPPHAParams()
        nchan = 8
        channel_key     = "ch" + str(channel)
        print (config[channel_key]['trigger-threshold'])
        trigger_threshold = config[channel_key]['trigger-threshold'] #in minivolts, neeed to convert to bin
        # resolution is 14 bit
        fsr = dynamic_range[1] - dynamic_range[0]
        lsb = fsr/2**14
        trigger_threshold = 1e-3*trigger_threshold # convert to volt
        trigger_threshold = int(trigger_threshold/lsb)
        print(f"Calculated value for {trigger_threshold}")
        print(f"Found a dc offset of {dc_offset}")
        #trigger_threshold += dc_offset/4 
        trigger_threshold = int(trigger_threshold)
        print(f"This means we have a trigger threshold after applying the dc offset of {trigger_threshold}")

        pars.thr        = [trigger_threshold                                    ]*nchan 
        pars.k          = [config[channel_key]['trapezoid-rise-time']           ]*nchan 
        pars.m          = [config[channel_key]['trapezoid-flat-top']            ]*nchan 
        pars.M          = [config[channel_key]['decay-time-constant']           ]*nchan 
        pars.ftd        = [config[channel_key]['flat-top-delay']                ]*nchan 
        pars.a          = [config[channel_key]['trigger-filter-smoothing-factor']]*nchan
        pars.b          = [config[channel_key]['input-signal-rise-time']        ]*nchan 
        pars.trgho      = [config[channel_key]['trigger-hold-off']              ]*nchan 
        pars.nsbl       = [config[channel_key]['n-samples']                     ]*nchan 
        pars.nspk       = [config[channel_key]['peak-mean']                     ]*nchan 
        pars.pkho       = [config[channel_key]['peak-holdoff']                  ]*nchan 
        pars.blho       = [config[channel_key]['baseline-holdoff']              ]*nchan
        pars.enf        = [config[channel_key]['energy-normalization-factor']   ]*nchan 
        pars.decimation = [config[channel_key]['decimation']                    ]*nchan 
        pars.dgain      = [config[channel_key]['decimation-gain']               ]*nchan 
        pars.otrej      = [config[channel_key]['otrej']                         ]*nchan 
        pars.trgwin     = [config[channel_key]['trigger-window']                ]*nchan 
        pars.twwdt      = [config[channel_key]['rise-time-validation-window']   ]*nchan
        return pars

    @staticmethod
    def convert_waveform_to_volt(input):
        """
        """
        pass
 
    def run_digitizer(self,\
                      seconds,\
                      rootfilename=None,\
                      read_waveforms=False):
        """
        For the interactive use in an ipython notebook
    
        Args:
            digitizer (CaenN1471) : a pre configured digitizer instance
            seconds   (int)       : runtime in seconds
            rootfilename  (str)   : filename of the output root file
            read_waveforms (bool) : sawe waveform data to the output root file
        """
        if read_waveforms:
            self.digitizer.enable_waveform_decoding()
        if rootfilename is not None:
            self.digitizer.set_rootfilename(rootfilename)
        # run calibration before readout
        self.digitizer.calibrate()
        self.digitizer.start_acquisition()
        print ("Starting run")
        self.digitizer.continuous_readout(seconds)
        self.digitizer.end_acquisition()
        print (f"We saw {self.digitizer.get_n_events_tot()} events!")
        return

