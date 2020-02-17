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
        ppa_pars  = self.extract_ppa_pha_parameters(config['CaenN6725']['dpp-pha-params'])

        self.digitizer = _cn.CaenN6725(digi_pars)
        bf = self.digitizer.get_board_info()
        print (f'Connected to digitizer model {bf.get_model_name()}, roc firmware {bf.get_model_roc_firmware_rel()},  amc firmware {bf.get_amc_firmware_rel()}')
        for ch, val in enumerate(self.digitizer.get_temperatures()):
            print (f'Chan: {ch} -  {val}\N{DEGREE SIGN}C')
        print ("Will calibrate the digitizer")
        self.digitizer.calibrate()

        # set input dynmaic range for all channels
        self.digitizer.set_input_dynamic_range(digitizer_dynamic_range)

        # set baseline offset
        for i,ch in enumerate(active_digitizer_channels):
            self.digitizer.set_baseline_offset(ch,digitizer_baseline_offset[i])

        self.digitizer.configure_channels(ppa_pars)
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
        pars.ChannelMask   = _cn.CHANNEL.ALL
    
        pars.VMEBaseAddress = config['VMEBaseAddress']
        pars.RecordLength = config['RecordLength']
        pars.EventAggr = config['EventAggr']
        return pars

    @staticmethod
    def extract_ppa_pha_parameters(config):
        """
        Extract the config parameters for the DPP-PHA
        algorithm from a config file 
        read by json
        """
        AVAILABLE_CHANNELS = 8
        pars = _cn.DPPPHAParams()
        pars.thr        = [config['trigger-threshold']]              *AVAILABLE_CHANNELS
        pars.k          = [config['trapezoid-rise-time']]            *AVAILABLE_CHANNELS
        pars.m          = [config['trapezoid-flat-top']]             *AVAILABLE_CHANNELS
        pars.M          = [config['decay-time-constant']]            *AVAILABLE_CHANNELS
        pars.ftd        = [config['flat-top-delay']]                 *AVAILABLE_CHANNELS
        pars.a          = [config['trigger-filter-smoothing-factor']]*AVAILABLE_CHANNELS
        pars.b          = [config['input-signal-rise-time']]         *AVAILABLE_CHANNELS
        pars.trgho      = [config['trigger-hold-off']]               *AVAILABLE_CHANNELS
        pars.nsbl       = [config['n-samples']]                      *AVAILABLE_CHANNELS
        pars.nspk       = [config['peak-mean']]                      *AVAILABLE_CHANNELS
        pars.pkho       = [config['peak-holdoff']]                   *AVAILABLE_CHANNELS
        pars.blho       = [config['baseline-holdoff']]               *AVAILABLE_CHANNELS
        pars.enf        = [config['energy-normalization-factor']]    *AVAILABLE_CHANNELS
        pars.decimation = [config['decimation']]                     *AVAILABLE_CHANNELS
        pars.dgain      = [config['decimation-gain']]                *AVAILABLE_CHANNELS
        pars.otrej      = [config['otrej']]                          *AVAILABLE_CHANNELS
        pars.trgwin     = [config['trigger-window']]                 *AVAILABLE_CHANNELS
        pars.twwdt      = [config['rise-time-validation-window']]    *AVAILABLE_CHANNELS
        return pars
 
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

