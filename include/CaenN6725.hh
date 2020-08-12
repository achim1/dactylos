#ifndef CLCAEN6725_H_INCLUDED
#define CLCAEN6725_H_INCLUDED

#include <vector>
#include <iostream>

//#define CAEN_DGTZ_BoardInfo_t _TRASH_

#include <CAENDigitizerType.h>
#include <CAENDigitizer.h>
#include <fstream>

#include "TFile.h"
#include "TTree.h"


/************************************************************************/

// channels are encoded with an 8 bit mask
// combinations can be achieved with bitwise &
enum CHANNEL : uint32_t
{
    CH0 = 1,
    CH1 = 2,
    CH2 = 4,
    CH3 = 8,
    CH4 = 16,
    CH5 = 32,
    CH6 = 64,
    CH7 = 128,
    ALL = 0xff 
};

/************************************************************************/

enum DynamicRange : uint32_t
{
    VPP2  = 0,
    VPP05 = 1
}; 

/************************************************************************/
// FIXME: I do not really understand how to handle the different options for
// the virtual/digital probes, since there is an option for the probe for the
// trace1 (virtualprobe1) in the library, but the otheres to not work, so we create 
// them here
// to be consistent, define one for DPPVirtualProbe1 as well
enum class DPPVirtualProbe1 : long
{
    Input            = CAEN_DGTZ_DPP_VIRTUALPROBE_Input,
    Delta            = CAEN_DGTZ_DPP_VIRTUALPROBE_Delta,
    Delta2           = CAEN_DGTZ_DPP_VIRTUALPROBE_Delta2,
    Trapezoid        = CAEN_DGTZ_DPP_VIRTUALPROBE_Trapezoid
};

enum class DPPVirtualProbe2 : long
{
    Input            = CAEN_DGTZ_DPP_VIRTUALPROBE_Input,
    TrapezoidReduced = CAEN_DGTZ_DPP_VIRTUALPROBE_TrapezoidReduced,
    Baseline         = CAEN_DGTZ_DPP_VIRTUALPROBE_Baseline,
    Threshold        = CAEN_DGTZ_DPP_VIRTUALPROBE_Threshold,
    None             = CAEN_DGTZ_DPP_VIRTUALPROBE_None
};

enum class DPPDigitalProbe1 : long
{
    TRGWin            = CAEN_DGTZ_DPP_DIGITALPROBE_TRGWin,
    Armed             = CAEN_DGTZ_DPP_DIGITALPROBE_Armed,
    PkRun             = CAEN_DGTZ_DPP_DIGITALPROBE_PkRun,
    Peaking           = CAEN_DGTZ_DPP_DIGITALPROBE_Peaking,
    CoincWin          = CAEN_DGTZ_DPP_DIGITALPROBE_CoincWin,
    TRGHoldoff        = CAEN_DGTZ_DPP_DIGITALPROBE_TRGHoldoff,
    ACQVeto           = CAEN_DGTZ_DPP_DIGITALPROBE_ACQVeto,
    BFMVeto           = CAEN_DGTZ_DPP_DIGITALPROBE_BFMVeto,
    ExtTRG            = CAEN_DGTZ_DPP_DIGITALPROBE_ExtTRG,
    Busy              = CAEN_DGTZ_DPP_DIGITALPROBE_Busy,
    PrgVeto           = CAEN_DGTZ_DPP_DIGITALPROBE_PrgVeto,
    PileUp            = CAEN_DGTZ_DPP_DIGITALPROBE_PileUp,
    BLFreeze          = CAEN_DGTZ_DPP_DIGITALPROBE_BLFreeze
};

enum class DPPDigitalProbe2 : long
{
    Trigger          = CAEN_DGTZ_DPP_DIGITALPROBE_Trigger
};

/************************************************************************/

// configure the DPP_PHA algorithm
struct ChannelParams_t
{
    uint32_t trigger_threshold; //100 (number of bins y axis)
    uint32_t trapezoidal_rise_time; //4 microsec // shaping time
    uint32_t trapezoidal_flat_top; //1 microsec
    uint32_t input_decay_time; //10microsec
    uint32_t flat_top_delay; //80 per cent of trapezoid flat top
    uint32_t input_signal_rise_time; // 50ns
    // different for detector set to 80microsec, for muon 100
    uint32_t trigger_filter_smoothing_factor; //16
    uint32_t trigger_hold_off; // to avoid pile up the longer the better 5microsec 
    uint32_t nsamples_baseline;// 5        
    uint32_t peak_mean; //2 
    uint32_t peak_holdoff; //5 microsec
    uint32_t baseline_holdoff;// - unknown
    float    energy_normalization;// 1.0
    uint32_t decimation;// 0
    uint32_t decimation_gain;// 0 
    uint32_t otrej; //unknown
    uint32_t enable_rise_time_discrimination;//
    uint32_t rise_time_validation_window;
};


/************************************************************************/

// configure digitizer connection, active channels etc.
struct DigitizerParams_t
{
    CAEN_DGTZ_ConnectionType LinkType;
    uint32_t VMEBaseAddress;
    uint32_t RecordLength;
    uint32_t ChannelMask;
    int EventAggr;
    int PostTriggerPercent; 
    CAEN_DGTZ_PulsePolarity_t PulsePolarity;
    CAEN_DGTZ_DPP_AcqMode_t AcqMode;
    CAEN_DGTZ_IOLevel_t IOlev;
    CAEN_DGTZ_DPP_PHA_Params_t* DPPParams;
};

/************************************************************************/

// string representation for numerical error codes
std::string error_code_to_string(CAEN_DGTZ_ErrorCode err);
// string representation coined into an operator
std::ostream& operator<<(std::ostream& os, const CAEN_DGTZ_ErrorCode& err);

/************************************************************************/

/**
 * The digitizer with the waveform recording firmware
 *
 */
class CaenN6725WF {
    
  public:
    CaenN6725WF();
    CaenN6725WF(DigitizerParams_t pars);
    ~CaenN6725WF();

    void connect();
    void configure(DigitizerParams_t params);
    void set_handle(int handle);
    int get_handle() const;
    long get_time() const;

    // get the expected baseline in ADU based on the set dc offset
    float get_expected_baseline(int channel);


    // set the 16bit dac value for the DC offset
    void set_channel_dc_offset(int channel, int offset);
    uint32_t  get_channel_dc_offset(int channel);

    // this trigger threshold is applied on the waveform
    // that means it is the minimum bin the waveform has
    // to reach (depending on the dc offset)
    void set_channel_trigger_threshold(int channel, int threshold);

    // return the current error state
    CAEN_DGTZ_ErrorCode get_last_error() const;

    // information about the board. Installed firmware version
    CAEN_DGTZ_BoardInfo_t get_board_info();

    // this needs to be called before any 
    // acquisition is started
    // to allocate the internal buffers
    void allocate_memory();

    // return the size of the allocated buffer in 
    //  
    uint32_t get_allocated_buffer_size();
    
    // the name of the file containing waveforms + energy
    void set_rootfilename(std::string fname);

    // prepare acquisition
    // don't acquire anything yet
    void start_acquisition();
    
    // end acquistion mode
    void end_acquisition();

    // number of digitizer channels
    int get_nchannels() const;
    
    // the current temperatures
    std::vector<int> get_temperatures() const;
    // temperature calibrabion
    void calibrate();
    // the input dynamic range is the peak-to-peak voltage
    // the digitizer is able to measure. the 14 bits are 
    // distributed over -vpp to +vpp
    // it is either -0.5 to 0.5 or -1 to 1 Volt
    void set_input_dynamic_range(DynamicRange range);

    // returns a 32 bit per channel but only LSB of these is relevant
    // 0 -> 2 Vpp
    // 1 ->0.5 Vpp 
    std::vector<uint32_t> get_input_dynamic_range();

    // get the number of events from a single
    // run - that is from acquistiion start - stop
    std::vector<long> get_n_events_tot();

    void readout_routine(bool write_root=true);
    void readout_and_save(unsigned int seconds);
    
    // for 'oscilloscope' use -> return the seen waveforms
    std::vector<std::vector<uint16_t>> readout_and_return();
    // free all event buffers and reallocate them
    void reset_memory(); 
  private:

    // before recording data, start/overwrite a 
    // new rootfile and set up the trees in it
    void prepare_rootfile();
    // check if certain channel is active
    bool is_active(int channel) const;

    // the handle is an unique identifier to this specific board
    // two boards can not be connected via the same handle!
    int handle_;
    // connection status of the digitizer - will be true after
    // the link to the digitizer has been established correctly
    bool is_connected_;
    // last seen error, 0 - success, negative are error codes
    CAEN_DGTZ_ErrorCode current_error_;

    // hold  the board info internally
    // FIXME: why?
    CAEN_DGTZ_BoardInfo_t board_info_;

    // buffer for the storage of events
    uint32_t  allocated_size_ = 0;
    uint32_t  buffer_size_ = 0;
    //char*     buffer_ = nullptr; // readout buffer
    char* buffer_;

    // structures for individual events
    CAEN_DGTZ_EventInfo_t event_info_;
    CAEN_DGTZ_UINT16_EVENT_t* this_event_ = nullptr;
    char * evt_bytestream_ = nullptr;


    // output to a root file
    std::string rootfile_name_  = "";
    TFile*      root_file_      = nullptr;
    std::vector<TTree*> channel_trees_ = {};

    // NB: the following define MUST specify the ACTUAL max allowed number of board's channels
    // it is needed for consistency inside the CAENDigitizer's functions used to allocate the memory
    static const uint32_t max_n_channels_ = 8;

    // number of acquired events per acquistion interval
    // [start acqusitizion , stop acquisitioin
    std::vector<long> n_events_acq_ = {};

    uint32_t num_events_[max_n_channels_];

    // data structures to store the waveforms
    std::vector<std::vector<uint16_t>> waveform_ch_ = {};


    // keep some configuration settings
    int recordlength_;

    // active channel bitmask - compare with it to check
    // if a particular channel is active
    uint8_t active_channel_bitmask_;

};

/************************************************************************/

/**
 * The digitizer with the DPP-PHA recording firwmare
 * [This firmware has to be purchased seperatly]
 * 
 */
class CaenN6725DPPPHA {

    public:
        CaenN6725DPPPHA();
        CaenN6725DPPPHA(DigitizerParams_t pars);
        ~CaenN6725DPPPHA();

        // open the link to the digitizer
        void connect();

        // configure the digitizer in case
        void configure(DigitizerParams_t params);

        // get the digitzer handle
        int get_handle();

        // set a new handle
        void set_handle(int handle);

        // show the supported probes, that is settings for the 
        // waveform fields (what will trace1/trace2/dtrace1/dtrace2 be?
        void show_supported_probes();

        // helper function to 
        long get_time() const;

        // set the 16bit dac value for the DC offset
        void set_channel_dc_offset(int channel, int offset);
        uint32_t  get_channel_dc_offset(int channel);

        // this trigger threshold is applied on the waveform
        // that means it is the minimum bin the waveform has
        // to reach (depending on the dc offset)
        void set_channel_trigger_threshold(int channel, int threshold);

        void enable_waveform_decoding();

        // return the current error state
        CAEN_DGTZ_ErrorCode get_last_error() const;

        // information about the board. Installed firmware version
        CAEN_DGTZ_BoardInfo_t get_board_info();

        // this needs to be called before any 
        // acquisition is started
        // to allocate the internal buffers
        void allocate_memory();

        // return the size of the allocated buffer in 
        //  
        uint32_t get_allocated_buffer_size();
        

        // prepare acquisition
        // don't acquire anything yet
        void start_acquisition();
        
        // end acquistion mode
        void end_acquisition();

        // number of digitizer channels
        int get_nchannels() const;
        
        // the current temperatures
        std::vector<int> get_temperatures() const;

        // set the parameters for the DPP-PHA algorithm for a specific channel
        void configure_channel(unsigned int channel, CAEN_DGTZ_DPP_PHA_Params_t* params);

        // temperature calibrabion
        void calibrate();

        // get a chunk of data
        // this needs to be run in some sort of loop.
        // display channel is used for the scope mode
        // and determines which channels shall be available
        // for the get_traces functions
        std::vector<std::vector<CAEN_DGTZ_DPP_PHA_Event_t>> read_data(bool fill_histogram = false);

        // get the number of events acquired per read_data call
        std::vector<int> get_n_events();
        
        // get the number of events acquired per acquisition call
        std::vector<long> get_n_events_tot();

        // get the number of event triggered per acquisition call
        std::vector<long> get_n_triggers_tot();

        // get the number of triggers lost due to deadtime etc. per acquisition call
        std::vector<long> get_n_lost_triggers_tot();

        // the input dynamic range is the peak-to-peak voltage
        // the digitizer is able to measure. the 14 bits are 
        // distributed over -vpp to +vpp
        // it is either -0.5 to 0.5 or -1 to 1 Volt
        void set_input_dynamic_range(DynamicRange range);

        // returns a 32 bit per channel but only LSB of these is relevant
        // 0 -> 2 Vpp
        // 1 ->0.5 Vpp 
        std::vector<uint32_t> get_input_dynamic_range();

        // read out the digitizer continuously
        // @param seconds : read out time
        void continuous_readout(unsigned int seconds);        
    
        // the name of the file containing waveforms + energy
        void set_rootfilename(std::string fname);
       
        // replaces the upper functions. If the virtual/digital probes 
        // are set, the traces will contain the respective values, 
        // depending on the setting of the probes
        std::vector<int16_t> get_analog_trace1();
        std::vector<int16_t> get_analog_trace2();
        std::vector<uint8_t> get_digital_trace1();
        std::vector<uint8_t> get_digital_trace2();
        
        // acces the last seen energy
        uint16_t get_energy();


        std::vector<int16_t> get_last_waveform(int channel);
        // set the virtualprobes for traces 1 and 2
        // this defines what will be stored in the waveform field 
        // of the dpp event
        // note that if both probes are used, the individual samplingrate is cut in half
        void set_virtualprobe1(DPPVirtualProbe1 vprobe1);
        void set_virtualprobe2(DPPVirtualProbe2 vprobe2);
        void set_digitalprobe1(DPPDigitalProbe1 vprobe1);
        void set_digitalprobe2(DPPDigitalProbe2 vprobe2);

        // check if certain channel is active
        bool is_active(int channel) const;
        
        // for dual trace mode, the sampling rate is only half
        int get_current_sampling_rate();

        // at which time sample do we have
        // the trigger point?
        // this requires the digital trace2 to be filled
        int get_trigger_point();

        // clear the energy histogram
        void clear_energy_histogram();

        // get the energy histogram
        std::vector<uint32_t> get_energy_histogram(int channel);

    private:

        // active channel bitmask - compare with it to check
        // if a particular channel is active
        uint8_t active_channel_bitmask_;

        // internal readout method, use read_data
        // if you want to interface with the individual 
        // data. This method is used by continuous_readout
        // and in the end only will save data to disk
        void fast_readout_();
        
        // number of acquired events per acquistion interval
        // [start acqusitizion , stop acquisitioin
        std::vector<long> n_events_acq_ = {};

        // length of the waveforms
        int recordlength_;

        // is this instance connected to the digitizer 
        // and has a handle assigned?
        bool is_connected_;

        // fill the internal field for the traces
        void fill_analog_trace1_();
        void fill_analog_trace2_();
        void fill_digital_trace1_();
        void fill_digital_trace2_();


        // is it configured"
        bool configured_ = false;
        // actual number of connected boards
        const int MAXNB_ = 1;
        // NB: the following define MUST specify the ACTUAL max allowed number of board's channels
        // it is needed for consistency inside the CAENDigitizer's functions used to allocate the memory
        static const uint32_t max_n_channels_ = 8;
        
        // The following define MUST specify the number of bits used for the energy calculation
        const int MAXNBITS_ = 15;
        
        // digitizer channels - we have 14 bit, so that 16834 channels
        const int NBINS_ = 16834;
    
        // something like the "address"
        // this gets assigned when the digitizer
        // is opened
        int handle_;
        CAEN_DGTZ_ErrorCode current_error_;

        // since event aggregate has to be set after the
        // acquisition mode, save this value in case the acquisition mode
        // gets changed.
        int event_aggregate_ = 0;

        /* Buffers to store the data. The memory must be allocated using the appropriate
        CAENDigitizer API functions (see below), so they must not be initialized here
        NB: you must use the right type for different DPP analysis (in this case PHA) */
        uint32_t                        allocated_size_ = 0;
        uint32_t                        buffer_size_ = 0;
        char*                           buffer_ = nullptr; // readout buffer
        CAEN_DGTZ_DPP_PHA_Event_t*      events_[max_n_channels_];  // events buffer
        CAEN_DGTZ_DPP_PHA_Waveforms_t*  waveform_ = nullptr;     // waveforms buffer
        CAEN_DGTZ_BoardInfo_t           board_info_;
        uint32_t                        num_events_[max_n_channels_];
};
        bool                            decode_waveforms_;

        // save data to a rootfle
        std::string                        rootfile_name_  = "digitizer_output.root";
        TFile*                             root_file_      = nullptr;
        std::vector<uint16_t>              energy_ch_      = {};
        std::vector<int>                   trigger_ch_     = {};
        std::vector<uint8_t>               saturated_ch_   = {};

        std::vector<std::vector<int16_t>>  waveform_ch_    = {};
        std::vector<TTree*>                channel_trees_  = {};

        // hold a single waveform. The values the actual fields are holding
        // depend on the setting for the analog and digital probes
        std::vector<int16_t> analog_trace1_;  // in case the analog_trace holds something else than the raw waveform, negative values are possible, e.g. for the fast timing filter
        std::vector<int16_t> analog_trace2_;
        std::vector<uint8_t> digital_trace1_;
        std::vector<uint8_t> digital_trace2_;

        int16_t* atrace1_;
        int16_t* atrace2_;
        uint8_t* dtrace1_;
        uint8_t* dtrace2_;
        uint32_t trace_ns_;
        uint16_t energy_; // the last seen energy 
        
        // aggregate quantities for all readout events
        std::vector<long> channel_triggers_;
        std::vector<long> channel_lost_triggers_;

        // a simle energy histogram
        std::vector<std::vector<uint32_t>> energy_histogram_;
        // this is basically the overflow bin for the energy histogram
        std::vector<uint32_t>fail_events_;
#endif
