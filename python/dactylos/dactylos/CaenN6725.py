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
    vprobe1_to_str = {\
    _cn.DPPVirtualProbe1.Input  : 'Input',\
    _cn.DPPVirtualProbe1.Delta  : 'Delta',\
    _cn.DPPVirtualProbe1.Delta2 : 'Delta2',\
    _cn.DPPVirtualProbe1.Trapezoid : 'Trapezoid'}

    vprobe2_to_str = {\
    _cn.DPPVirtualProbe2.Input             : 'Input',\
    _cn.DPPVirtualProbe2.TrapezoidReduced  : 'TrapezoidReduced',\
    _cn.DPPVirtualProbe2.Baseline          : 'Baseline',\
    _cn.DPPVirtualProbe2.Threshold         : 'Threshold',\
    _cn.DPPVirtualProbe2.NONE              : 'None'}

    dprobe1_to_str = {\
    _cn.DPPDigitalProbe1.TRGWin            :  'TRGWin',\
    _cn.DPPDigitalProbe1.Armed             :  'Armed',\
    _cn.DPPDigitalProbe1.PkRun             :  'PkRun',\
    _cn.DPPDigitalProbe1.Peaking           :  'Peaking',\
    _cn.DPPDigitalProbe1.CoincWin          :  'CoincWin',\
    _cn.DPPDigitalProbe1.TRGHoldoff        :  'TRGHoldoff',\
    _cn.DPPDigitalProbe1.ACQVeto           :  'ACQVeto',\
    _cn.DPPDigitalProbe1.BFMVeto           :  'BFMVeto',\
    _cn.DPPDigitalProbe1.ExtTRG            :  'ExtTRG',\
    _cn.DPPDigitalProbe1.Busy              :  'Busy',\
    _cn.DPPDigitalProbe1.PrgVeto           :  'PrgVeto',\
    _cn.DPPDigitalProbe1.PileUp            :  'PileUp',\
    _cn.DPPDigitalProbe1.BLFreeze          :  'BLFreeze'}
    
    dprobe2_to_str = {\
    _cn.DPPDigitalProbe2.Trigger           :  'Trigger'}

    @property
    def samplingrate(self):
        return self.digitizer.get_current_sampling_rate()

    def __init__(self, configfile, shaping_time=None):
        self.digitizer = None
        self.recordlength = None
        self.configfile = configfile
        self.trigger_thresholds = dict()
        self.dc_offsets = dict()
        self.dynamic_range = list()
        self.shaping_time = shaping_time
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
        # reminder, the trapezoid rise time is our SHAPINGTIME
        if self.shaping_time is None:
            trrt  = config[channel_key]['trapezoid-rise-time']
        else:
            print ('Warning, overwriting shaping time from configfile with {self.shaping_time}')
            trrt = self.shaping_time
        trft  = config[channel_key]['trapezoid-flat-top']
        trpho = config[channel_key]['peak-holdoff']
        pkrun = trrt + trft + trpho # in ns
        eventlength  = self.recordlength*(1/self.samplingrate)*1e9 # ns
        if pkrun >= eventlength: 
            raise ValueError(f"Trapezoid run time {pkrun} is longer than recordlength {self.recordlength}.Tof fix it, change the values of trapezoid-rise-time, trapezoid-flat-top, peak-holdoff.")
        print (f"We have an eventlength of {eventlength} and trapezoid run time of {pkrun}")
        pars.thr        = [trigger_threshold                                    ]*nchan 
        pars.k          = [trrt                                                 ]*nchan                     
        #pars.k          = [config[channel_key]['trapezoid-rise-time']           ]*nchan 
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
        if hasattr(waveform, '__iter__'):
            volts = np.array([w*(frs/resolution) + self.dynamic_range[0] for w in waveform]) 
        else:
            volts = waveform*(frs/resolution) + self.dynamic_range[0]
        return volts


    def get_times(self, digital_trace=False):
        """
        Return the time binning in microseconds

        """
        binsize = 1/self.samplingrate
        if digital_trace:
            # FIXME: binsize always full sampling rate
            binsize = 1/250e6
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
        canvas = YStackedCanvas(subplot_yheights=(0.1, 0.1, 0.2, 0.5),
                                padding=(0.15, 0.05, 0.0, 0.1 ),\
                                space_between_plots=0,\
                                figsize="auto",\
                                figure_factory=None)
        # initialize figure
        xs_analog  = self.get_times()*1e6 # times is in ns 
        xs_digital = self.get_times(digital_trace=True)*1e6
        # FIXME
        xs = xs_digital # find out in what conditions the smapling rate is reduced
        #ax.set_xlabel("Time [$\mu$s]")
        #ax.set_ylabel("Voltage mV")
        p.ion()
        #ax.set_xlim(left=min(xs), right=max(xs))

        # trace 1 of the scope is the fast timing filter
        trace1_axes  = canvas.select_axes(0)
        trace2_axes  = canvas.select_axes(1)
        dtrace1_axes = canvas.select_axes(2)
        dtrace2_axes = canvas.select_axes(3)
        all_axes = trace1_axes, trace2_axes, dtrace1_axes, dtrace2_axes

        for ax in all_axes:
            ax.set_xlim(left=min(xs), right=max(xs))
        
        trace1_axes.set_xlabel("Time [$\mu$s]")

        if trace1 == _cn.DPPVirtualProbe1.Input:
            trace1_axes.set_ylabel("Voltage mV")

        # waveform (input) plot
        trace1_plot  = trace1_axes.plot(range(0), color="b", lw=1.2)[0]
        trace2_plot  = trace2_axes.plot(range(0), color="r", lw=1.2)[0]
        dtrace1_plot = dtrace1_axes.plot(range(0), color="k", lw=1.2)[0]
        dtrace2_plot = dtrace2_axes.plot(range(0), color="g", lw=1.2)[0]

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
        energy     = self.digitizer.get_energy()

        print ("-----sneak peak waveforms-----")
        print ("tr1",  ys_trace1[:10],  len(ys_trace1),  self.vprobe1_to_str[trace1])
        print ("tr2",  ys_trace2[:10],  len(ys_trace2),  self.vprobe2_to_str[trace2])
        print ("dtr1", ys_dtrace1[:10], len(ys_dtrace1), self.dprobe1_to_str[dtrace1])
        print ("dtr2", ys_dtrace2[:10], len(ys_dtrace2), self.dprobe2_to_str[dtrace2])
        print ("energy", energy)
        print ("------------------------------")
        
        energy    = 1e3*self.to_volts(0, energy)
        plot_energy = None
        if trace1 == _cn.DPPVirtualProbe1.Input:
            ys_trace1  = self.to_volts(0,ys_trace1)
            ys_trace1 = 1e3*ys_trace1
            delta = abs(max(ys_trace1)) - abs(min(ys_trace1))
            bl_corrected = abs(ys_trace1) - abs(min(ys_trace1))*np.ones(len(ys_trace1))
            # calculate decay time, that is ln(2)*half signal height    
            over_threshold = bl_corrected > delta/2
            half_time = xs[over_threshold][-1]
            print (f"Calculated decay time of {np.log(2)*half_time}")

            energy    = 1e3*self.to_volts(0, energy)
            plot_energy = 1
        #if trace2 == _cn.DPPVirtualProbe2.Input:
            #ys_trace2  = self.to_volts(0,ys_trace2)
            #ys_trace2 = 1e3*ys_trace2
            #plot_energy = 2
            #print (ys_trace1[:10])
        #print (ys_trace2[:10])
        #print (len(xs), len(ys_trace1), len(ys_trace2))
        #all_plots = trace1_plot, trace2_plot, dtrace1_plot, dtrace2_plot
        trace1_plot.set_xdata(xs)
        trace1_plot.set_ydata(ys_trace1)
        #if len(ys_trace2):
        trace2_plot.set_xdata(xs)
        trace2_plot.set_ydata(ys_trace2)
        dtrace1_plot.set_xdata(xs)
        dtrace1_plot.set_ydata(ys_dtrace1)
        dtrace2_plot.set_xdata(xs)
        dtrace2_plot.set_ydata(ys_dtrace2)

        if plot_energy == 1:
            trace1_axes.hlines(min(xs), max(xs), energy)
        if plot_energy == 2:
            trace2_axes.hlines(min(xs), max(xs), energy)

        for ys, ax in [(ys_trace1, trace1_axes),\
                       (ys_trace2, trace2_axes),\
                       (ys_dtrace2, dtrace1_axes),\
                       (ys_dtrace2, dtrace2_axes)]:
            if len(ys):
                ymax = max(ys) + abs(max(ys)*0.2)
                ymin = min(ys) - abs(min(ys)*0.2)
                print (f"adjugsting range {ymin} - {ymax}")
                ax.set_ylim(bottom=ymin, top=ymax)
                ax.grid(True)

        #canvas.figure.tight_layout()

        plt.adjust_minor_ticks(trace1_axes, which = 'both')
        plt.adjust_minor_ticks(trace2_axes, which = 'y')
        #plt.adjust_minor_ticks(dtrace1_axes, which = 'y')
        #plt.adjust_minor_ticks(dtrace2_axes, which = 'y')
                

        trace1_axes.text(xs[int(0.7*len(xs))],0.8*max(ys_trace1),   self.vprobe1_to_str[trace1]) 
        trace2_axes.text(xs[int(0.7*len(xs))],0.8*max(ys_trace2),   self.vprobe2_to_str[trace2]) 
        dtrace1_axes.text(xs[int(0.7*len(xs))],0.8*max(ys_dtrace1), self.dprobe1_to_str[dtrace1]) 
        dtrace2_axes.text(xs[int(0.7*len(xs))],0.8*max(ys_dtrace2), self.dprobe2_to_str[dtrace2]) 

        # despine
        trace2_axes.spines['bottom'].set_visible(False)
        dtrace1_axes.spines['bottom'].set_visible(False)
        dtrace2_axes.spines['bottom'].set_visible(False)
        for tic in trace2_axes.xaxis.get_major_ticks():
            tic.tick1line.set_visible(False)
            #tic.tick1On = tic.tick2On = False
        for tic in dtrace1_axes.xaxis.get_major_ticks():
            tic.tick1line.set_visible(False)
            #tic.tick1On = tic.tick2On = False
        for tic in dtrace2_axes.xaxis.get_major_ticks():
            tic.tick1line.set_visible(False)
            #tic.tick1On = tic.tick2On = False


        canvas.figure.canvas.draw()
        canvas.figure.savefig(filename)
        #for i in range(6):
        #    time.sleep(5)
        #for k in data:
        #   for j in k:
        #       print (j.wavefor
        #ms)
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
     
                 
     
