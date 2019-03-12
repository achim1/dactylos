#ifndef CLCAEN6725_H_INCLUDED
#define CLCAEN6725_H_INCLUDED

#include <vector>

//#define CAEN_DGTZ_BoardInfo_t _TRASH_

#include <CAENDigitizerType.h>
#include <CAENDigitizer.h>


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
    CAEN_DGTZ_PulsePolarity_t PulsePolarity;
    CAEN_DGTZ_DPP_AcqMode_t AcqMode;
    CAEN_DGTZ_IOLevel_t IOlev;
    CAEN_DGTZ_DPP_PHA_Params_t* DPPParams;
};

/************************************************************************/

class CaenN6725 {

    public:
        CaenN6725();
        CaenN6725(DigitizerParams_t pars);
        ~CaenN6725();

        // helper function to 
        long get_time() const;

        // return the current error state
        CAEN_DGTZ_ErrorCode get_last_error() const;
        CAEN_DGTZ_BoardInfo_t get_board_info();

        // this needs to be called before any 
        // acquisition is started
        // to allocate the internal buffers
        void allocate_memory();

        // prepare acquisition
        // don't acquire anything yet
        void start_acquisition();
        
        // end acquistion mode
        void end_acquisition();

        // number of digitizer channels
        int get_nchannels() const;
        
        // the current temperatures
        std::vector<int> get_temperatures() const;

        // set the parameters for the DPP-PHA algorithm for
        // ALL ACTIVE channels (see channel mask in DigitizerParams_t)
        void configure_channels(CAEN_DGTZ_DPP_PHA_Params_t* params);

        // temperature calibrabion
        void calibrate();

        // get a chunk of data
        // this needs to be run in some sort of loop
        std::vector<std::vector<CAEN_DGTZ_DPP_PHA_Event_t>> read_data();

        // get the number of events acquired per read_data call
        std::vector<int> get_n_events();

    private:
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
        /* Buffers to store the data. The memory must be allocated using the appropriate
        CAENDigitizer API functions (see below), so they must not be initialized here
        NB: you must use the right type for different DPP analysis (in this case PHA) */
        uint32_t                        allocated_size_;
        uint32_t                        buffer_size_;
        char*                           buffer_ = nullptr; // readout buffer
        CAEN_DGTZ_DPP_PHA_Event_t*      events_[max_n_channels_];  // events buffer
        CAEN_DGTZ_DPP_PHA_Waveforms_t*  waveform_=nullptr;     // waveforms buffer
        //BoardInfo_t                     board_info_;
        CAEN_DGTZ_BoardInfo_t           board_info_;
        // channel mask (8bit)
        uint32_t                        active_channels_;

        uint32_t                        num_events_[max_n_channels_];
};

#endif
