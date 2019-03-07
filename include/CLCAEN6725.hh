#ifndef CLCAEN6725_H_INCLUDED
#define CLCAEN6725_H_INCLUDED

#include <vector>

#include <CAENDigitizerType.h>
#include <CAENDigitizer.h>


// actual number of connected boards
static const int MAXNB(1);
// NB: the following define MUST specify the ACTUAL max allowed number of board's channels
// it is needed for consistency inside the CAENDigitizer's functions used to allocate the memory
static const int MaxNChannels(8);

// The following define MUST specify the number of bits used for the energy calculation
static const int MAXNBITS(15);

// digitizer channels - we have 14 bit, so that 16834 channels
static const int NBINS(16834);

/************************************************************************/

class GOptionParser;

/************************************************************************/

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
//} ChannelParams_t;

/************************************************************************/

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
        ~CaenN6725();
        DigitizerParams_t InitializeDigitizerForPulseGenerator(GOptionParser parser);
        int ProgramDigitizer(int handle, DigitizerParams_t Params);
        long get_time() const;
        CAEN_DGTZ_ErrorCode get_last_error() const;
        CAEN_DGTZ_BoardInfo_t get_board_info();
        void allocate_memory();
        int get_nchannels() const;
        std::vector<int> get_temperatures() const;
        void configure_channel(int channel, ChannelParams_t params);
        void calibrate();

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
        char                            *buffer_ = nullptr; // readout buffer
        CAEN_DGTZ_DPP_PHA_Event_t       *events_[max_n_channels_];  // events buffer
        CAEN_DGTZ_DPP_PHA_Waveforms_t   *waveform_=nullptr;     // waveforms buffer
        CAEN_DGTZ_BoardInfo_t           board_info_;




};

/************************************************************************/



/************************************************************************/

//long get_time();

/************************************************************************/

//void CloseDigitizer();

/************************************************************************/

#endif
