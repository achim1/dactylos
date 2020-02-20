import hjson
import numpy as np
import pylab as p
import time 
import _pyCaenN6725 as _cn

import hepbasestack as hep
hep.visual.set_style_default()

from HErmes.visual.canvases import YStackedCanvas
import HErmes.visual.layout as lo
import HErmes.visual.plotting as plt


landscape_half_figure_factory= lambda : p.figure(dpi=150, figsize=lo.FIGSIZE_A4_LANDSCAPE_HALF_HEIGHT)



class CaenN6725(object):
    """
    A digitizer from CAEN. Model name N6725.

    Args:
        configfile (str) : Path to a .json configfile. See example config file
                           in this package
    """
    def __init__(self, configfile):
        self.digitizer = None
        self.recordlength = None
        self.samplingrate = 250e6
        self.configfile = configfile
        self.trigger_thresholds = dict()
        self.dc_offsets = dict()
        self.dynamic_range = list()
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

    def set_vprobe1(self, vprobe1):
        self.digitizer.set_vprobe1(vprobe1)

    def set_vprobe2(self, vprobe2):
        self.digitizer.set_vprobe2(vprobe2)

    def set_dprobe1(self, vprobe1):
        self.digitizer.set_vprobe1(vprobe1)

    def set_dprobe2(self, vprobe2):
        self.digitizer.set_vprobe2(vprobe2)

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
            self.dynamic_range = [-1,1]
        if digitizer_dynamic_range == "05VPP":
            digitizer_dynamic_range = _cn.DynamicRange.VPP05
            self.dynamic_range = [-0.5, 0.5]
        

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
            self.dc_offsets[ch] = offset
            # configure each channel individually
            dpp_params = self.extract_dpp_pha_parameters(ch, config['CaenN6725'], offset) 
            self.digitizer.configure_channel(ch, dpp_params)

        for ch, val in enumerate(self.digitizer.get_temperatures()):
            print (f'Chan: {ch} -  {val}\N{DEGREE SIGN}C')
        print ("Will calibrate the digitizer")
        self.digitizer.calibrate()
        self.digitizer.allocate_memory()
        return 

    def extract_digitizer_parameters(self,config):
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
        # also stroe in own field
        self.recordlength = config['RecordLength']
        pars.EventAggr = config['EventAggr']
        return pars

    def extract_dpp_pha_parameters(self, channel, config,\
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
        self.trigger_thresholds[channel] = trigger_threshold

        # check if the trapezoid is short enough to fit into the record length
        trrt  = config[channel_key]['trapezoid-rise-time']
        trft  = config[channel_key]['trapezoid-flat-top']
        trpho = config[channel_key]['peak-holdoff']
        pkrun = trrt + trft + trpho # in ns
        eventlength  = self.recordlength*(1/self.samplingrate)*1e9 # ns
        if pkrun >= eventlength: 
            raise ValueError(f"Trapezoid run time {pkrun} is longer than recordlength {self.recordlength}.Tof fix it, change the values of trapezoid-rise-time, trapezoid-flat-top, peak-holdoff.")
        print (f"We have an eventlength of {eventlength} and trapezoid run time of {pkrun}")
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

    def to_volts(self,ch, waveform):
        """
        Convert waveform from digitizer bins to volts

        """
        resolution = 2**14
        frs = self.dynamic_range[1] - self.dynamic_range[0]
        #offset = self.dc_offset[ch]
        #offset = (offset/4)*(frs/resolution)  + self.dynamic_range[0]
        volts = np.array([w*(frs/resolution) + self.dynamic_range[0] for w in waveform]) 
        return volts


    def get_times(self):
        """
        Return the time binning in microseconds

        """
        binsize = 1/self.samplingrate
        bins = np.array([k*binsize for k in range(self.recordlength)])
        return bins



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
        print ("Starting run")
        self.digitizer.start_acquisition()
        self.digitizer.continuous_readout(seconds)
        self.digitizer.end_acquisition()
        print (f"We saw {self.digitizer.get_n_events_tot()} events!")
        return

    def oscilloscope(self,\
                     maxtime=np.inf,\
                     filename="test.png",\
                     trace1 = _cn.DPPVirtualProbe1.Input,\
                     trace2 = _cn.DPPVirtualProbe2.TrapezoidReduced,\
                     dtrace1 = _cn.DPPDigitalProbe1.Peaking,\
                     dtrace2 = _cn.DPPDigitalProbe2.Trigger):
        """
        Show waveforms in oscilloscope mode 

        Keyword Args:
            maxtime (float)    : Maximum time the plot is active (in sec)
        """
        time_since_running = 0
        self.digitizer.enable_waveform_decoding()
        canvas = YStackedCanvas(subplot_yheights=(0.2, 0.7),
                                padding=(0.15, 0.05, 0.0, 0.1 ),\
                                space_between_plots=0,\
                                figsize="auto",\
                                figure_factory=None)
        # initialize figure
        xs  = self.get_times()*1e6 # times is in ns 
        #ax.set_xlabel("Time [$\mu$s]")
        #ax.set_ylabel("Voltage mV")
        p.ion()
        #ax.set_xlim(left=min(xs), right=max(xs))

        # trace 1 of the scope is the fast timing filter
        trace1_axes = canvas.select_axes(0)
        trace2_axes = canvas.select_axes(1)
        for ax in [trace1_axes, trace2_axes]:
            ax.set_xlabel("Time [$\mu$s]")
            ax.set_xlim(left=min(xs), right=max(xs))

        trace1_axes.set_xlabel("Time [$\mu$s]")
        trace1_axes.set_ylabel("Voltage mV")

        # waveform (input) plot
        trace1_plot = trace1_axes.plot(range(0), color="b", lw=1.2)[0]
        trace2_plot = trace2_axes.plot(range(0), color="r", lw=1.2)[0]

        start_time = time.monotonic()
     
        canvas.figure.show()
        canvas.figure.canvas.draw()
        canvas.figure.tight_layout()
   
        # set the traces -m
        self.digitizer.set_vprobe1(trace1)
        self.digitizer.set_vprobe2(trace2)
        self.digitizer.set_dprobe1(dtrace1)
        self.digitizer.set_dprobe2(dtrace2)
        #self.digitizer.allocate_memory()
        self.digitizer.start_acquisition()
        data = []
        while len(data) == 0:
            data = self.digitizer.read_data()
        self.digitizer.end_acquisition()

        ys_trace1  = self.digitizer.get_analog_trace1()
        ys_trace2  = self.digitizer.get_analog_trace2()
        ys_dtrace1 = self.digitizer.get_digital_trace1()
        ys_dtrace2 = self.digitizer.get_digital_trace2()
        print (set(ys_trace1)) 

        print ("-----sneak peak waveforms-----")
        print (ys_trace1[:10])
        print (ys_trace2[:10])
        print (ys_dtrace1[:10])
        print (ys_dtrace2[:10])
        print ("------------------------------")

        ys_trace1  = self.to_volts(0,ys_trace1)

        print (set(ys_trace1)) 
        ys_trace1 = 1e3*ys_trace1
        print (xs[:10])
        print (ys_trace1[:10])
        print (ys_trace2[:10])
        print (len(xs), len(ys_trace1), len(ys_trace2))
        trace1_plot.set_xdata(xs)
        trace1_plot.set_ydata(ys_trace1)
        if len(ys_trace2):
            trace2_plot.set_xdata(xs)
            trace2_plot.set_ydata(ys_trace2)


        for ys, ax in [(ys_trace1, trace1_axes),\
                       (ys_trace2, trace2_axes)]:
            if len(ys):
                ymax = max(ys) + abs(max(ys)*0.2)
                ymin = min(ys) - abs(min(ys)*0.2)
                ax.set_ylim(bottom=ymin, top=ymax)
                ax.grid(True)

        canvas.figure.tight_layout()

        plt.adjust_minor_ticks(trace1_axes, which = 'both')
        plt.adjust_minor_ticks(trace2_axes, which = 'y')

        canvas.figure.canvas.draw()
        canvas.figure.savefig(filename)
        #for i in range(6):
        #    time.sleep(5)
        #for k in data:
        #   for j in k:
        #       print (j.waveforms)
        #while True:
        #    sec = time.monotonic() - start_time
        #    datamins, datamaxes = [],[]
        #    for ch, line_plot in enumerate(line_plots):
     
        #        temp = self.get_temperature(ch)
        #        secs, temps = line_plot.get_data()
        #        secs = np.append(secs,sec)
        #        temps = np.append(temps,temp)
        #        line_plot.set_ydata(temps)
        #        line_plot.set_xdata(secs)
        #        datamins.append(min(temps))
        #        datamaxes.append(max(temps))
        #        xmin = min(secs)
        #        xmax = max(secs)
        #        if ch == feedback_ch:
        #            feedback_temp = temp
     
     
        #    datamax = max(datamaxes)
        #    datamin = min(datamins)
        #    if len(secs) == 1:
        #        continue
     
        #    # avoid matplotlib warning
        #    if abs(datamin - datamax) < 1:
        #        datamin -= 1
        #        datamax += 1
     
        #    if abs(xmax - xmin) < 1:
        #        xmin -= 1
        #        xmax += 1
     
        #    # update the plot
        #    ax.set_xlim(left=xmin, right=xmax)
        #    ax.set_ylim(bottom=datamin, top=datamax)
     
        #    fig.tight_layout()
        #    fig.canvas.draw()
        #    time.sleep(1)
        #    time_since_running += 1
     
        #    if time_since_running > maxtime:
        #        return
     
                 
     

