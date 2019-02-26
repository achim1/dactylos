
#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>


#include <CAENDigitizerType.h>
#include <CAENDigitizer.h>
// FIXME find out how to set dynamic range

#include "TFile.h"
#include "TH1I.h"

// from gaps software
#include "gaps/GOptionParser.hh"
#include "gaps/GLogging.hh"
#include "gaps/GProgressBar.hh"

// actual number of connected boards
#define MAXNB   1
// NB: the following define MUST specify the ACTUAL max allowed number of board's channels
// it is needed for consistency inside the CAENDigitizer's functions used to allocate the memory
#define MaxNChannels 8

// The following define MUST specify the number of bits used for the energy calculation
#define MAXNBITS 15

// digitizer channels - we have 14 bit, so that 16834 channels
static const int NBINS(16834);

/**************************************************************/

///class 

/***************************************************************/

/*
typedef struct
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
} ChannelParams_t;
*/

/***************************************************************/

typedef struct
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
} DigitizerParams_t;

/***************************************************************/

typedef std::vector<TH1I*> HistogramSeries;

/***************************************************************/

/* --------------------------------------------------------------------------------------------------------- */
/*! \fn      int ProgramDigitizer(int handle, DigitizerParams_t Params, CAEN_DGTZ_DPPParamsPHA_t DPPParams)
*   \brief   Program the registers of the digitizer with the relevant parameters
*   \return  0=success; -1=error */
/* --------------------------------------------------------------------------------------------------------- */
int ProgramDigitizer(int handle, DigitizerParams_t Params)
{
    /* This function uses the CAENDigitizer API functions to perform the digitizer's initial configuration */
    int i, ret = 0;

    /* Reset the digitizer */
    ret |= CAEN_DGTZ_Reset(handle);

    if (ret) {
        printf("ERROR: can't reset the digitizer.\n");
        return -1;
    }
    ret |= CAEN_DGTZ_WriteRegister(handle, 0x8000, 0x01000114);  // Channel Control Reg (indiv trg, seq readout) ??

    /* Set the DPP acquisition mode
    This setting affects the modes Mixed and List (see CAEN_DGTZ_DPP_AcqMode_t definition for details)
    CAEN_DGTZ_DPP_SAVE_PARAM_EnergyOnly        Only energy (DPP-PHA) or charge (DPP-PSD/DPP-CI v2) is returned
    CAEN_DGTZ_DPP_SAVE_PARAM_TimeOnly        Only time is returned
    CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime    Both energy/charge and time are returned
    CAEN_DGTZ_DPP_SAVE_PARAM_None            No histogram data is returned */
    ret |= CAEN_DGTZ_SetDPPAcquisitionMode(handle, Params.AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
    
    // Set the digitizer acquisition mode (CAEN_DGTZ_SW_CONTROLLED or CAEN_DGTZ_S_IN_CONTROLLED)
    ret |= CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
    
    // Set the number of samples for each waveform
    ret |= CAEN_DGTZ_SetRecordLength(handle, Params.RecordLength);

    // Set the I/O level (CAEN_DGTZ_IOLevel_NIM or CAEN_DGTZ_IOLevel_TTL)
    ret |= CAEN_DGTZ_SetIOLevel(handle, Params.IOlev);

    /* Set the digitizer's behaviour when an external trigger arrives:

    CAEN_DGTZ_TRGMODE_DISABLED: do nothing
    CAEN_DGTZ_TRGMODE_EXTOUT_ONLY: generate the Trigger Output signal
    CAEN_DGTZ_TRGMODE_ACQ_ONLY = generate acquisition trigger
    CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT = generate both Trigger Output and acquisition trigger

    see CAENDigitizer user manual, chapter "Trigger configuration" for details */
    ret |= CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);

    // Set the enabled channels
    ret |= CAEN_DGTZ_SetChannelEnableMask(handle, Params.ChannelMask);

    // Set how many events to accumulate in the board memory before being available for readout
    ret |= CAEN_DGTZ_SetDPPEventAggregation(handle, Params.EventAggr, 0);
    
    /* Set the mode used to syncronize the acquisition between different boards.
    In this example the sync is disabled */
    ret |= CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled);
    
    // Set the DPP specific parameters for the channels in the given channelMask
    ret |= CAEN_DGTZ_SetDPPParameters(handle, Params.ChannelMask, Params.DPPParams);
    
    for(i=0; i<MaxNChannels; i++) {
        if (Params.ChannelMask & (1<<i)) {
            // Set a DC offset to the input signal to adapt it to digitizer's dynamic range
            ret |= CAEN_DGTZ_SetChannelDCOffset(handle, i, 0x8000);
            
            // Set the Pre-Trigger size (in samples)
            ret |= CAEN_DGTZ_SetDPPPreTriggerSize(handle, i, 1000);
            
            // Set the polarity for the given channel (CAEN_DGTZ_PulsePolarityPositive or CAEN_DGTZ_PulsePolarityNegative)
            ret |= CAEN_DGTZ_SetChannelPulsePolarity(handle, i, Params.PulsePolarity);
        }
    }

    /* Set the virtual probes settings
    DPP-PHA can save:
    2 analog waveforms:
        the first and the second can be specified with the  ANALOG_TRACE 1 and 2 parameters
        
    2 digital waveforms:
        the first can be specified with the DIGITAL_TRACE_1 parameter
        the second  is always the trigger

    CAEN_DGTZ_DPP_VIRTUALPROBE_SINGLE    -> Save only the ANALOG_TRACE_1 waveform
    CAEN_DGTZ_DPP_VIRTUALPROBE_DUAL      -> Save also the waveform specified in  ANALOG_TRACE_2

    Virtual Probes 1 types:
    CAEN_DGTZ_DPP_VIRTUALPROBE_Input
    CAEN_DGTZ_DPP_VIRTUALPROBE_Delta
    CAEN_DGTZ_DPP_VIRTUALPROBE_Delta2
    CAEN_DGTZ_DPP_VIRTUALPROBE_Trapezoid
    
    Virtual Probes 2 types:
    CAEN_DGTZ_DPP_VIRTUALPROBE_Input
    CAEN_DGTZ_DPP_VIRTUALPROBE_Threshold
    CAEN_DGTZ_DPP_VIRTUALPROBE_TrapezoidReduced
    CAEN_DGTZ_DPP_VIRTUALPROBE_Baseline
    CAEN_DGTZ_DPP_VIRTUALPROBE_None

    Digital Probes types:
    CAEN_DGTZ_DPP_DIGITALPROBE_TRGWin
    CAEN_DGTZ_DPP_DIGITALPROBE_Armed
    CAEN_DGTZ_DPP_DIGITALPROBE_PkRun
    CAEN_DGTZ_DPP_DIGITALPROBE_PileUp
    CAEN_DGTZ_DPP_DIGITALPROBE_Peaking
    CAEN_DGTZ_DPP_DIGITALPROBE_CoincWin
    CAEN_DGTZ_DPP_DIGITALPROBE_BLFreeze
    CAEN_DGTZ_DPP_DIGITALPROBE_TRGHoldoff
    CAEN_DGTZ_DPP_DIGITALPROBE_TRGVal
    CAEN_DGTZ_DPP_DIGITALPROBE_ACQVeto
    CAEN_DGTZ_DPP_DIGITALPROBE_BFMVeto
    CAEN_DGTZ_DPP_DIGITALPROBE_ExtTRG
    CAEN_DGTZ_DPP_DIGITALPROBE_Busy
    CAEN_DGTZ_DPP_DIGITALPROBE_PrgVeto*/

    ret |= CAEN_DGTZ_SetDPP_VirtualProbe(handle, ANALOG_TRACE_1, CAEN_DGTZ_DPP_VIRTUALPROBE_Delta2);
    ret |= CAEN_DGTZ_SetDPP_VirtualProbe(handle, ANALOG_TRACE_2, CAEN_DGTZ_DPP_VIRTUALPROBE_Input);
    ret |= CAEN_DGTZ_SetDPP_VirtualProbe(handle, DIGITAL_TRACE_1, CAEN_DGTZ_DPP_DIGITALPROBE_Peaking);

    if (ret) {
        WARN("Warning: errors found during the programming of the digitizer.\nSome settings may not be executed\n");
        return ret;
    } else {
        return 0;
    }
}

/***************************************************************/

DigitizerParams_t InitializeDigitizerForPulseGenerator(GOptionParser parser)
{
    DigitizerParams_t digiParams;
    digiParams.LinkType = CAEN_DGTZ_USB;  // Link Type
    digiParams.VMEBaseAddress = 0;  // For direct USB connection, VMEBaseAddress must be 0

    digiParams.IOlev = CAEN_DGTZ_IOLevel_NIM;
    /****************************\
    *  Acquisition parameters    *
    \****************************/
    //digiParams.AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Mixed;          // CAEN_DGTZ_DPP_ACQ_MODE_List or CAEN_DGTZ_DPP_ACQ_MODE_Oscilloscope
    digiParams.AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Mixed;          // CAEN_DGTZ_DPP_ACQ_MODE_List or CAEN_DGTZ_DPP_ACQ_MODE_Oscilloscope
    digiParams.RecordLength = 2000;                              // Num of samples of the waveforms (only for Oscilloscope mode)
    //digiParams.ChannelMask = 0x01;                               // Channel enable mask
    digiParams.ChannelMask = 0xff;                               // Channel enable mask
    digiParams.EventAggr = 0;                                   // number of events in one aggregate (0=automatic)
    digiParams.PulsePolarity = CAEN_DGTZ_PulsePolarityPositive; // Pulse Polarity (this parameter can be individual)

    CAEN_DGTZ_DPP_PHA_Params_t* DPPParams = new CAEN_DGTZ_DPP_PHA_Params_t();
    for (unsigned int ch=0; ch<MaxNChannels; ch++)
        {
            DPPParams->thr[ch] = parser.GetOption<int>("trigger-threshold");
            DPPParams->k[ch] = parser.GetOption<int>("trapezoid-rise-time");
            DPPParams->m[ch] = parser.GetOption<int>("trapezoid-flat-top");
            DPPParams->M[ch] = parser.GetOption<int>("decay-time-constant"); 
            DPPParams->ftd[ch] = parser.GetOption<int>("flat-top-delay");
            DPPParams->a[ch]= parser.GetOption<int>("trigger-filter-smoothing-factor");
            DPPParams->b[ch] = parser.GetOption<int>("input-signal-rise-time");
            DPPParams->trgho[ch] = parser.GetOption<int>("trigger-hold-off");
            DPPParams->nsbl[ch] = parser.GetOption<int>("n-samples");
            DPPParams->nspk[ch] = parser.GetOption<int>("peak-mean");
            DPPParams->pkho[ch] = parser.GetOption<int>("peak-holdoff");
            DPPParams->blho[ch] = parser.GetOption<int>("baseline-holdoff");
            DPPParams->enf[ch] = parser.GetOption<float>("energy-normalization-factor"); 
            DPPParams->decimation[ch] = parser.GetOption<int>("decimation");
            DPPParams->dgain[ch] = parser.GetOption<int>("decimation-gain");
            DPPParams->otrej[ch] = parser.GetOption<int>("otrej");
            DPPParams->trgwin[ch] = parser.GetOption<int>("trigger-window");
            DPPParams->twwdt[ch] = parser.GetOption<int>("rise-time-validation-window"); 
        } //r
    digiParams.DPPParams = DPPParams;
    return digiParams;
}

/***************************************************************/

/*! \fn      static long get_time()
*   \brief   Get time in milliseconds
*   \return  time in msec */
// stolen from CAEN examples
long get_time()
{
    long time_ms;
#ifdef WIN32
    struct _timeb timebuffer;
    _ftime( &timebuffer );
    time_ms = (long)timebuffer.time * 1000 + (long)timebuffer.millitm;
#else
    struct timeval t1;
    struct timezone tz;
    gettimeofday(&t1, &tz);
    time_ms = (t1.tv_sec) * 1000 + t1.tv_usec / 1000;
#endif
    return time_ms;
}

/***************************************************************/


// FIXME: pro;er close function
void CloseDigitizer()
{
    /* stop the acquisition, close the device and free the buffers */
    //for (b =0 ; b < MAXNB; b++) {
    //    CAEN_DGTZ_SWStopAcquisition(handle[b]);
    //    CAEN_DGTZ_CloseDigitizer(handle[b]);
    //    for (ch = 0; ch < MaxNChannels; ch++)
    //        if (EHisto[b][ch] != NULL)
    //            free(EHisto[b][ch]);
    //}   
    //CAEN_DGTZ_FreeReadoutBuffer(&buffer);
    //CAEN_DGTZ_FreeDPPEvents(handle[0], Events);
    //CAEN_DGTZ_FreeDPPWaveforms(handle[0], Waveform);
};

/***************************************************************/

int main(int argc, char* argv[])
{

    std::string description("Take data with the CAEN digitizer");
    GOptionParser parser = GOptionParser(argc, argv, description);

    // general options
    parser.AddOption<std::string>("output-file","write output to file","digitizer-test.root","o");

    // DPP related options
    parser.AddOption<int>("trigger-threshold", "in LSB", 10);
    parser.AddOption<int>("trapezoid-rise-time", "in ns", 4000);
    parser.AddOption<int>("trapezoid-flat-top", "in ns", 990);
    parser.AddOption<int>("decay-time-constant", "in ns", 10000); 
    parser.AddOption<int>("flat-top-delay", "(peaking time) in ns", 800);
    parser.AddOption<int>("trigger-filter-smoothing-factor", "number of samples to average for RC-CR2 filter. Options: 1,2,4,8,16,32", 16);
    parser.AddOption<int>("input-signal-rise-time", "in ns", 48);
    parser.AddOption<int>("trigger-hold-off", "?", 4992);
    parser.AddOption<int>("n-samples", "number of samples for baseline averagae claculation. Options: 1->16 samples, 2->64 samples, 3->256 samples, 4->1024 samples, 5->4096 samples, 6->16384 samples", 5);
    parser.AddOption<int>("peak-mean", "number of amples to average for trapezoid height calculation. Options 0->1 sample, 1->4 sampel, 2->16 sampels, 3->64 samples", 2 );
    parser.AddOption<int>("peak-holdoff", "in ns", 4992);
    parser.AddOption<int>("baseline-holdoff", "in ns", 500);
    parser.AddOption<float>("energy-normalization-factor","?", 1.0); 
    parser.AddOption<int>("decimation", "the input singal samples are averaged within this number of samples. 0->disabled, 1->2 samples, 2->4 samples, 3->8 samples", 0);
    parser.AddOption<int>("decimation-gain", "options: 0->DigitalGain=1, 1->DigitalGain=2 (only with decimation >= 2 samples), 2->DigitialGain=4 (only with decimation >= 4 samples, 3->DigitialGain=8 (only with decimation = 8 samples)", 0);
    parser.AddOption<int>("otrej", "FIXME ?", 0);
    // FIXME: this should be a flag then, no?
    parser.AddOption<int>("trigger-window", "enable rise time discrimatinon. Options 0->disabled, 1->enabled", 0);
    parser.AddOption<int>("rise-time-validation-window", "in ns", 100); 
    parser.AddOption<int>("n-events-ch1", "Acquire N events for channel 1", 1000, "n1"); 
    parser.AddOption<int>("n-events-ch2", "Acquire N events for channel 2", 1000, "n2"); 
    parser.AddOption<int>("n-events-ch3", "Acquire N events for channel 3", 1000, "n3"); 
    parser.AddOption<int>("n-events-ch4", "Acquire N events for channel 4", 1000, "n4"); 
    parser.AddOption<int>("n-events-ch5", "Acquire N events for channel 5", 1000, "n5"); 
    parser.AddOption<int>("n-events-ch6", "Acquire N events for channel 6", 1000, "n6"); 
    parser.AddOption<int>("n-events-ch7", "Acquire N events for channel 7", 1000, "n7"); 
    parser.AddOption<int>("n-events-ch8", "Acquire N events for channel 8", 1000, "n8"); 
    parser.Parse();
    
    /* The following variable is the type returned from most of CAENDigitizer
    library functions and is used to check if there was an error in function
    execution. For example:
    ret = CAEN_DGTZ_some_function(some_args);
    if(ret) printf("Some error"); */
    CAEN_DGTZ_ErrorCode ret;

    /* Buffers to store the data. The memory must be allocated using the appropriate
    CAENDigitizer API functions (see below), so they must not be initialized here
    NB: you must use the right type for different DPP analysis (in this case PHA) */
    uint32_t AllocatedSize, BufferSize;
    char *buffer = NULL;                                 // readout buffer
    CAEN_DGTZ_DPP_PHA_Event_t       *Events[MaxNChannels];  // events buffer
    CAEN_DGTZ_DPP_PHA_Waveforms_t   *Waveform=NULL;     // waveforms buffer
    CAEN_DGTZ_BoardInfo_t           BoardInfo;

    /* The following variables will store the digitizer configuration parameters */
    //CAEN_DGTZ_DPP_PHA_Params_t DPPParams[MAXNB];
    //DigitizerParams_t Params[MAXNB];

    //int ret, handle;
    int handle;

    DigitizerParams_t thisParams = InitializeDigitizerForPulseGenerator(parser);
    ret = CAEN_DGTZ_OpenDigitizer(thisParams.LinkType, 0, 0, thisParams.VMEBaseAddress, &handle);
    INFO("Handler : " << handle);
    /* The following is for b boards connected via 1 opticalLink in dasy chain
    in this case you must set Params[b].LinkType = CAEN_DGTZ_PCI_OpticalLink and Params[b].VMEBaseAddress = 0 */
    //ret = CAEN_DGTZ_OpenDigitizer(Params[b].LinkType, 0, b, Params[b].VMEBaseAddress, &handle[b]);

    /* The following is for b boards connected to A2818 (or A3818) via opticalLink (or USB with A1718)
    in this case the boards are accessed throught VME bus, and you must specify the VME address of each board:
    Params[b].LinkType = CAEN_DGTZ_PCI_OpticalLink (CAEN_DGTZ_PCIE_OpticalLink for A3818 or CAEN_DGTZ_USB for A1718)
    Params[0].VMEBaseAddress = <0xXXXXXXXX> (address of first board) 
    Params[1].VMEBaseAddress = <0xYYYYYYYY> (address of second board) 
    etc */
    //ret = CAEN_DGTZ_OpenDigitizer(Params[b].LinkType, 0, 0, Params[b].VMEBaseAddress, &handle[b]);

    if (ret) {
        FATAL("Can't open digitizer\n");
        //goto QuitProgram;    
    }
    
    /* Once we have the handler to the digitizer, we use it to call the other functions */
    ret = CAEN_DGTZ_GetInfo(handle, &BoardInfo);
    if (ret) {
        FATAL("Can't read board info\n");
        //goto QuitProgram;
    }
    printf("\nConnected to CAEN Digitizer Model %s, recognized as board %d\n", BoardInfo.ModelName, 0);
    printf("ROC FPGA Release is %s\n", BoardInfo.ROC_FirmwareRel);
    printf("AMC FPGA Release is %s\n", BoardInfo.AMC_FirmwareRel);

    ProgramDigitizer(handle, thisParams);

    /* WARNING: The mallocs MUST be done after the digitizer programming,
    because the following functions needs to know the digitizer configuration
    to allocate the right memory amount */
    /* Allocate memory for the readout buffer */
    // ufjehuebscht wird spaeta
    CAEN_DGTZ_ErrorCode errCode;
    errCode = CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &AllocatedSize);
    if (errCode != 0) WARN("Error while allocating the readout buffer : " << errCode);
    /* Allocate memory for the events */
    errCode = CAEN_DGTZ_MallocDPPEvents(handle, (void**)(Events), &AllocatedSize); 
    if (errCode != 0) WARN("Error while allocation DPP event buffer : " << errCode);
    /* Allocate memory for the waveforms */
    errCode = CAEN_DGTZ_MallocDPPWaveforms(handle, (void**)(&Waveform), &AllocatedSize); 
    if (errCode != 0) WARN("Error while allocating DPP waveform buffer : " << errCode);

    uint32_t temp;
    
    for (uint ch=0; ch<MaxNChannels; ch++)
        {
            CAEN_DGTZ_ReadTemperature(handle, ch, &temp);
            INFO("Ch " << ch << " ADC temperature: " <<  temp);
        }

    uint32_t NumEvents[MaxNChannels];
    for (unsigned int ch=0; ch<MaxNChannels; ch++)
        {NumEvents[ch] = 0;}
   
    unsigned int runs = 1;
    unsigned int const DATATRANSFER_INTERVAL(2);
    
    // run the digitizer for a while,
    // copy over the data and save it
    // to root files and then clear 
    // the buffers on the digitizer and run it 
    // again
    // FIXME: optimize - how large is the buffer 
    // on the digitizer?
   
    int *bin = new int();
    int *wdata = new int();
    int size;
    int16_t *WaveLine;
    uint8_t *DigitalWaveLine;

    // initialize histograms
    HistogramSeries histos;
    histos.reserve(8);
    std::string histname;
    std::string histtitle;
    for (unsigned int ch=0; ch<MaxNChannels; ch++)
        {
            histname = "ehistch" + std::to_string(ch);
            histtitle = "DigitizerChannel Ch" + std::to_string(ch);
            histos.push_back(new TH1I(histname.c_str(), histtitle.c_str(), NBINS, 0, NBINS-1));
        }

    std::string outfile = parser.GetOption<std::string>("output-file");
    INFO("Writing to root file " << outfile);
    TFile* output = new TFile(outfile.c_str(),"RECREATE");

    // runtime
    //int nsec = 5;

    //for (unsigned int r=0; r<runs; r++)
    //    {
    uint b(0);
    CAEN_DGTZ_SWStartAcquisition(handle);
    //      long currentTime = get_time(); // time in millisec
    INFO("Acquisition Started for Board " <<  b);
    
    std::vector<int> nAcquired({0,0,0,0,0,0,0,0});
    //std::cout << currentTime << std::endl;
    //long timeDelta = 0;
    std::vector<int> to_acquire(8);
    to_acquire.push_back(parser.GetOption<int>("n-events-ch1"));
    to_acquire.push_back(parser.GetOption<int>("n-events-ch2"));
    to_acquire.push_back(parser.GetOption<int>("n-events-ch3"));
    to_acquire.push_back(parser.GetOption<int>("n-events-ch4"));
    to_acquire.push_back(parser.GetOption<int>("n-events-ch5"));
    to_acquire.push_back(parser.GetOption<int>("n-events-ch6"));
    to_acquire.push_back(parser.GetOption<int>("n-events-ch7"));
    to_acquire.push_back(parser.GetOption<int>("n-events-ch8"));
    int n_events(0);
    for (int n : to_acquire) {n_events += n;}
    int n_acquired(0);
    GProgressBar bar = GProgressBar(n_events);
    //FIXME: How to decide when we have enough statistics"
    //Single channel > n, sum(all channels) > n or something else?
    //int n_acquired = *std::min_element(nAcquired.begin(), nAcquired.end());

    //std::vector<int> n_acquired_previous({0,0,0,0,0,0,0,0});
    int n_acquired_previous(0); // just used for propper counter display 
    while (nAcquired[0] < to_acquire[0] ||
           nAcquired[1] < to_acquire[1] ||
           nAcquired[2] < to_acquire[2] ||
           nAcquired[3] < to_acquire[3] ||
           nAcquired[4] < to_acquire[4] ||
           nAcquired[5] < to_acquire[5] ||
           nAcquired[6] < to_acquire[6] ||
           nAcquired[7] < to_acquire[7]
    )
    //while (timeDelta < 1000*nsec){
    {
        errCode = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize);
        DEBUG("Error : " << errCode);
        DEBUG("BufferSize : " << BufferSize);
        if (BufferSize == 0) continue;
        errCode =  CAEN_DGTZ_GetDPPEvents(handle, buffer, BufferSize, (void**)(Events), NumEvents);
        //std::cout << "Error : " << errCode << std::endl;
   
        for (unsigned int ch=0; ch<MaxNChannels; ch++)
            {
                nAcquired[ch] += NumEvents[ch];
                n_acquired += NumEvents[ch]; // total number of acquired events
                DEBUG("Ch. " << ch << " saw " << NumEvents[ch] << " events");
                DEBUG("N acq " << nAcquired[ch]);
                for (unsigned int ev=0; ev<NumEvents[ch]; ev++)
                    {
                        //Events[ch]->Energy;
                        histos.at(ch)->Fill(Events[ch]->Energy);

                        //std::cout<< Events[ch]->Energy << std::endl;
                        //CAEN_DGTZ_DecodeDPPWaveforms(handle, &Events[ch][ev], Waveform);
                        //size = (int)(Waveform->Ns); // Number of samples
                        //WaveLine = Waveform->Trace1; // First trace (ANALOG_TRACE_1)
   
                        //for (unsigned int k=0; k<size; k++)
                        //    {
                        //        //*(bin) = k;
                        //        //*(wdata) = WaveLine[k];
                        //        //tree->Fill();
                        //    }
                    //break;
                    } // end fill histos
            } // end loop over channels


          //long newTime = get_time();
          //timeDelta += newTime - currentTime;
          //currentTime = newTime;

        // update counter and bar
        //n_acquired = *std::min_element(nAcquired.begin(), nAcquired.end());
        DEBUG("Acquired " << n_acquired);
        for (int i=0; i<(n_acquired - n_acquired_previous); i++)
        {
            ++bar;
        }
    n_acquired_previous = n_acquired;
    DEBUG(n_acquired);
    } // end acquisition

    //CAEN_DGTZ_SendSWtrigger(handle); 
    CAEN_DGTZ_SWStopAcquisition(handle); 
    INFO("Acquisition Stopped for Board " << b);  

    std::string acquired_events_display("Acquired: ");
    for (unsigned int ch=0; ch<MaxNChannels; ch++)
      {
        acquired_events_display += std::to_string(nAcquired[ch]);
        acquired_events_display += " events for channel ";
        acquired_events_display += std::to_string(ch);
        acquired_events_display += " \n"; 
        //INFO("Acquired " << nAcquired[ch] << " events " << " for channel " << ch);
      }
    INFO( acquired_events_display);
    //Run(handle,DATATRANSFER_INTERVAL, histos);
    output->cd();
    for (auto h : histos)
        {h->Write();}
    output->Write();
    errCode = CAEN_DGTZ_ClearData(handle);
    if (errCode != 0) WARN("Error when clearing data : " << errCode);
    return EXIT_SUCCESS;
}
