
#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>


#include <CAENDigitizerType.h>
#include <CAENDigitizer.h>
// FIXME find out how to set dynamic range

#include "TFile.h"
#include "TChain.h"

// actual number of connected boards
#define MAXNB   1
// NB: the following define MUST specify the ACTUAL max allowed number of board's channels
// it is needed for consistency inside the CAENDigitizer's functions used to allocate the memory
#define MaxNChannels 8

// The following define MUST specify the number of bits used for the energy calculation
#define MAXNBITS 15


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

    CAEN_DGTZ_DPP_VIRTUALPROBE_SINGLE	 -> Save only the ANALOG_TRACE_1 waveform
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
        printf("Warning: errors found during the programming of the digitizer.\nSome settings may not be executed\n");
        return ret;
    } else {
        return 0;
    }
}

/***************************************************************/

DigitizerParams_t InitializeDigitizerForPulseGenerator()
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
			DPPParams->thr[ch] = 10 ;     // Trigger Threshold (in LSB)
			DPPParams->k[ch] = 4000;       // Trapezoid Rise Time (ns) 
			DPPParams->m[ch] = 992;       // Trapezoid Flat Top  (ns) 
			DPPParams->M[ch] = 10000;      // Decay Time Constant (ns) 
			DPPParams->ftd[ch] = 800;      // Flat top delay (peaking time) (ns) 
			DPPParams->a[ch] = 16;         // Trigger Filter smoothing factor (number of samples to average for RC-CR2 filter) Options: 1; 2; 4; 8; 16; 32

			DPPParams->b[ch] = 48   ;     // Input Signal Rise time (ns) 
			DPPParams->trgho[ch] = 4992;   // Trigger Hold Off
			DPPParams->nsbl[ch] = 5;       //number of samples for baseline average calculation. Options: 1->16 samples; 2->64 samples; 3->256 samples; 4->1024 samples; 5->4096 samples; 6->16384 samples
			DPPParams->nspk[ch] = 2;       //Peak mean (number of samples to average for trapezoid height calculation). Options: 0-> 1 sample; 1->4 samples; 2->16 samples; 3->64 samples
			DPPParams->pkho[ch] = 4992;    //peak holdoff (ns)
			DPPParams->blho[ch] = 500;     //Baseline holdoff (ns)
			DPPParams->enf[ch] = 1.0;      // Energy Normalization Factor
			DPPParams->decimation[ch] = 0; //decimation (the input signal samples are averaged within this number of samples): 0 ->disabled; 1->2 samples; 2->4 samples; 3->8 samples
			DPPParams->dgain[ch] = 0;      //decimation gain. Options: 0->DigitalGain=1; 1->DigitalGain=2 (only with decimation >= 2samples); 2->DigitalGain=4 (only with decimation >= 4samples); 3->DigitalGain=8( only with decimation = 8samples).
			DPPParams->otrej[ch] = 0;
			DPPParams->trgwin[ch] = 0;     //Enable Rise time Discrimination. Options: 0->disabled; 1->enabled
			DPPParams->twwdt[ch] = 100;    //Rise Time Validation Window (ns)
        } //r
    digiParams.DPPParams = DPPParams;
    return digiParams;
}

/***************************************************************/

void Run(int handle, int nsec)
{
    uint b(0);
    CAEN_DGTZ_SWStartAcquisition(handle);
    printf("Acquisition Started for Board %d\n", b);
    sleep(nsec);
    CAEN_DGTZ_SendSWtrigger(handle); 
    CAEN_DGTZ_SWStopAcquisition(handle); 
    printf("Acquisition Stopped for Board %d\n", b);
    

}


/***************************************************************/

int main()
{
    
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

    DigitizerParams_t thisParams = InitializeDigitizerForPulseGenerator();
    ret = CAEN_DGTZ_OpenDigitizer(thisParams.LinkType, 0, 0, thisParams.VMEBaseAddress, &handle);
    std::cout << "Handler : " << handle << std::endl;
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
        printf("Can't open digitizer\n");
        //goto QuitProgram;    
    }
    
    /* Once we have the handler to the digitizer, we use it to call the other functions */
    ret = CAEN_DGTZ_GetInfo(handle, &BoardInfo);
    if (ret) {
        printf("Can't read board info\n");
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
    std::cout << "Error : " << errCode << std::endl;
    /* Allocate memory for the events */
    errCode = CAEN_DGTZ_MallocDPPEvents(handle, (void**)(Events), &AllocatedSize); 
    std::cout << "Error : " << errCode << std::endl;
    /* Allocate memory for the waveforms */
    errCode = CAEN_DGTZ_MallocDPPWaveforms(handle, (void**)(&Waveform), &AllocatedSize); 
    std::cout << "Error : " << errCode << std::endl;
    uint32_t temp;
    for (uint ch=0; ch<MaxNChannels; ch++)
        {
            CAEN_DGTZ_ReadTemperature(handle, ch, &temp);
            printf("Ch %d  ADC temperature: %d %cC\n", ch, temp, 248);
        }
    uint32_t NumEvents[MaxNChannels];
    for (unsigned int ch=0; ch<MaxNChannels; ch++)
        {NumEvents[ch] = 42;}
   
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

    TFile* output = new TFile("testfile.root","RECREATE");
    TChain * tree = new TChain("Waveform");
    tree->Branch("bin", &bin);
    tree->Branch("wdata", &wdata);
    tree->Add("testfile.root");


 
    for (unsigned int r=0; r<runs; r++)
        {
            Run(handle,DATATRANSFER_INTERVAL);
            errCode = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize);
            std::cout << "Error : " << errCode << std::endl;
            std::cout << "BufferSize : " << BufferSize << std::endl;
            errCode =  CAEN_DGTZ_GetDPPEvents(handle, buffer, BufferSize, (void**)(Events), NumEvents);
            std::cout << "Error : " << errCode << std::endl;

            for (unsigned int ch=0; ch<MaxNChannels; ch++)
                {
                    std::cout << "Saw " << NumEvents[ch] << " events " << " for channel " << ch << std::endl;
                    for (unsigned int ev=0; NumEvents[ch]; ev++)
                        {
                            CAEN_DGTZ_DecodeDPPWaveforms(handle, &Events[ch][ev], Waveform);
                            size = (int)(Waveform->Ns); // Number of samples
                            WaveLine = Waveform->Trace1; // First trace (ANALOG_TRACE_1)

                            for (unsigned int k=0; k<size; k++)
                                {
                                    //*(bin) = k;
                                    //*(wdata) = WaveLine[k];
                                    //tree->Fill();
                                }
                        break;
                        }
                }
            tree->Write();
            errCode = CAEN_DGTZ_ClearData(handle);
            std::cout << "Error : " << errCode << std::endl;
        }
    return EXIT_SUCCESS;
}
