#include <stdexcept>

#include <CAENDigitizerType.h>
#include <CAENDigitizer.h>

#include "CLCAEN6725.hh"

#include "gaps/GOptionParser.hh"

/***************************************************************/



CaenN6725::CaenN6725()
{
    // make this specific for our case
    // third 0 is VMEBaseAddress, which must be 0 for direct USB connections
    current_error_ = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB, 0, 0, 0, &handle_);
    if (current_error_ !=0 ) throw std::runtime_error("Can not open digitizer err code: " + std::to_string(current_error_));

    /* Reset the digitizer */
    current_error_ = CAEN_DGTZ_Reset(handle_);
    if (current_error_ !=0 ) throw std::runtime_error("Can not reset digitizer err code:" + std::to_string(current_error_));
    current_error_ = CAEN_DGTZ_WriteRegister(handle_, 0x8000, 0x01000114);  // Channel Control Reg (indiv trg, seq readout) ??
    if (current_error_ !=0 ) throw std::runtime_error("Can not write register err code:" + std::to_string(current_error_));
    // Set t`he digitizer acquisition mode (CAEN_DGTZ_SW_CONTROLLED or CAEN_DGTZ_S_IN_CONTROLLED)
    current_error_ = CAEN_DGTZ_SetAcquisitionMode(handle_, CAEN_DGTZ_SW_CONTROLLED);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set acquisition mode err code:" + std::to_string(current_error_));
    //see CAENDigitizer user manual, chapter "Trigger configuration" for details */
    current_error_ = CAEN_DGTZ_SetExtTriggerInputMode(handle_, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set trigger input  mode err code:" + std::to_string(current_error_));


    /* Set the mode used to syncronize the acquisition between different boards.
    In this example the sync is disabled */
    current_error_ = CAEN_DGTZ_SetRunSynchronizationMode(handle_, CAEN_DGTZ_RUN_SYNC_Disabled);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set run synchronisation mode err code: " + std::to_string(current_error_));
    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, ANALOG_TRACE_1, CAEN_DGTZ_DPP_VIRTUALPROBE_Delta2);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP virtual probe trace 1 err ode: " + std::to_string(current_error_));
    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, ANALOG_TRACE_2, CAEN_DGTZ_DPP_VIRTUALPROBE_Input);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP virtual probe trace 2 err ode: " + std::to_string(current_error_));
    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, DIGITAL_TRACE_1, CAEN_DGTZ_DPP_DIGITALPROBE_Peaking);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP virtual probe trace 2 err ode: " + std::to_string(current_error_));
};

/***************************************************************/

CaenN6725::CaenN6725(DigitizerParams_t params) : CaenN6725()
{
    current_error_ = CAEN_DGTZ_SetDPPAcquisitionMode(handle_, params.AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP acquisition mode err code:" + std::to_string(current_error_));

    // Set the number of samples for each waveform
    current_error_ = CAEN_DGTZ_SetRecordLength(handle_, params.RecordLength);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set record length err code: " + std::to_string(current_error_));

    // Set the I/O level (CAEN_DGTZ_IOLevel_NIM or CAEN_DGTZ_IOLevel_TTL)
    current_error_ = CAEN_DGTZ_SetIOLevel(handle_, params.IOlev);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set IO level err code:" + std::to_string(current_error_));

    // Set the enabled channels
    current_error_ = CAEN_DGTZ_SetChannelEnableMask(handle_, params.ChannelMask);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set channel enable mask err code:" + std::to_string(current_error_));

    // Set how many events to accumulate in the board memory before being available for readout
    current_error_ = CAEN_DGTZ_SetDPPEventAggregation(handle_, params.EventAggr, 0);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set dpp event agregation err code:" + std::to_string(current_error_));

    for(unsigned int i=0; i<max_n_channels_; i++) {
        if (params.ChannelMask & (1<<i)) {
            // Set a DC offset to the input signal to adapt it to digitizer's dynamic range
            current_error_ = CAEN_DGTZ_SetChannelDCOffset(handle_, i, 0x8000);
            if (current_error_ !=0 ) throw std::runtime_error("Can not set channel dc offset err code:" + std::to_string(current_error_));

            // Set the Pre-Trigger size (in samples)
            current_error_ = CAEN_DGTZ_SetDPPPreTriggerSize(handle_, i, 1000);
            if (current_error_ !=0 ) throw std::runtime_error("Can not set dpp trigger sixe err code:" + std::to_string(current_error_));

            // Set the polarity for the given channel (CAEN_DGTZ_PulsePolarityPositive or CAEN_DGTZ_PulsePolarityNegative)
            current_error_ = CAEN_DGTZ_SetChannelPulsePolarity(handle_, i, params.PulsePolarity);
            if (current_error_ !=0 ) throw std::runtime_error("Can not set pulse polarity err code:" + std::to_string(current_error_));
        }
    }
};

/***************************************************************/

CAEN_DGTZ_ErrorCode CaenN6725::get_last_error() const
{
    return current_error_;
}

/***************************************************************/

CAEN_DGTZ_BoardInfo_t CaenN6725::get_board_info()
{
    current_error_ = CAEN_DGTZ_GetInfo(handle_, &board_info_);
    if (current_error_ != 0) throw std::runtime_error("Error while getting board infoe, err code " + std::to_string(current_error_));
    return board_info_;
}


/***************************************************************/

void CaenN6725::allocate_memory()
{
    //if (!configured_) throw std::runtime_error("ERROR: The mallocs MUST be done after the digitizer programming because the following functions needs to know the digitizer configuration to allocate the right memory amount");
    current_error_ = CAEN_DGTZ_MallocReadoutBuffer(handle_, &buffer_, &allocated_size_);
    if (current_error_ != 0) throw std::runtime_error("Error while allocating readout buffer, err code " + std::to_string(current_error_));
    /* Allocate memory for the events */
    current_error_ = CAEN_DGTZ_MallocDPPEvents(handle_, (void**)(events_), &allocated_size_);
    if (current_error_ != 0) throw std::runtime_error("Error while allocating DPP event buffer, err code " + std::to_string(current_error_));
    /* Allocate memory for the waveforms */
    current_error_ = CAEN_DGTZ_MallocDPPWaveforms(handle_, (void**)(&waveform_), &allocated_size_);
    if (current_error_ != 0) throw std::runtime_error("Error while allocating DPP waveform buffer, err code " + std::to_string(current_error_));
}

/***************************************************************/

void CaenN6725::read_data()
{
    current_error_ = CAEN_DGTZ_ReadData(handle_, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer_, &buffer_size_);
    //if (current_error_ != 0) throw std::runtime_error("Error while reading data from the digitizer, err code " + std::to_string(current_error_));

}

/***************************************************************/

DigitizerParams_t CaenN6725::InitializeDigitizerForPulseGenerator(GOptionParser parser)
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

    //CAEN_DGTZ_DPP_PHA_Params_t* DPPParams = new CAEN_DGTZ_DPP_PHA_Params_t();
    //for (unsigned int ch=0; ch<MaxNChannels; ch++)
    //    {
    //        DPPParams->thr[ch] = parser.GetOption<int>("trigger-threshold");
    //        DPPParams->k[ch] = parser.GetOption<int>("trapezoid-rise-time");
    //        DPPParams->m[ch] = parser.GetOption<int>("trapezoid-flat-top");
    //        DPPParams->M[ch] = parser.GetOption<int>("decay-time-constant");
    //        DPPParams->ftd[ch] = parser.GetOption<int>("flat-top-delay");
    //        DPPParams->a[ch]= parser.GetOption<int>("trigger-filter-smoothing-factor");
    //        DPPParams->b[ch] = parser.GetOption<int>("input-signal-rise-time");
    //        DPPParams->trgho[ch] = parser.GetOption<int>("trigger-hold-off");
    //        DPPParams->nsbl[ch] = parser.GetOption<int>("n-samples");
    //        DPPParams->nspk[ch] = parser.GetOption<int>("peak-mean");
    //        DPPParams->pkho[ch] = parser.GetOption<int>("peak-holdoff");
    //        DPPParams->blho[ch] = parser.GetOption<int>("baseline-holdoff");
    //        DPPParams->enf[ch] = parser.GetOption<float>("energy-normalization-factor");
    //        DPPParams->decimation[ch] = parser.GetOption<int>("decimation");
    //        DPPParams->dgain[ch] = parser.GetOption<int>("decimation-gain");
    //        DPPParams->otrej[ch] = parser.GetOption<int>("otrej");
    //        DPPParams->trgwin[ch] = parser.GetOption<int>("trigger-window");
    //        DPPParams->twwdt[ch] = parser.GetOption<int>("rise-time-validation-window");
    //    } //r
    //digiParams.DPPParams = DPPParams;
    return digiParams;
}

/***************************************************************/

int CaenN6725::get_nchannels() const
{
    return max_n_channels_;
}

/***************************************************************/

int CaenN6725::ProgramDigitizer(int handle, DigitizerParams_t Params)
{
    /* This function uses the CAENDigitizer API functions to perform the digitizer's initial configuration */
    int i, ret = 0;


    /* Set the DPP acquisition mode
    This setting affects the modes Mixed and List (see CAEN_DGTZ_DPP_AcqMode_t definition for details)
    CAEN_DGTZ_DPP_SAVE_PARAM_EnergyOnly        Only energy (DPP-PHA) or charge (DPP-PSD/DPP-CI v2) is returned
    CAEN_DGTZ_DPP_SAVE_PARAM_TimeOnly        Only time is returned
    CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime    Both energy/charge and time are returned
    CAEN_DGTZ_DPP_SAVE_PARAM_None            No histogram data is returned */

    /* Set the digitizer's behaviour when an external trigger arrives:

    CAEN_DGTZ_TRGMODE_DISABLED: do nothing
    CAEN_DGTZ_TRGMODE_EXTOUT_ONLY: generate the Trigger Output signal
    CAEN_DGTZ_TRGMODE_ACQ_ONLY = generate acquisition trigger
    CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT = generate both Trigger Output and acquisition trigger


    // Set the enabled channels
    ret |= CAEN_DGTZ_SetChannelEnableMask(handle, Params.ChannelMask);

    // Set how many events to accumulate in the board memory before being available for readout
    ret |= CAEN_DGTZ_SetDPPEventAggregation(handle, Params.EventAggr, 0);


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


    // set as configured
    configured_ = true;

    if (ret) {
        //WARN("Warning: errors found during the programming of the digitizer.\nSome settings may not be executed\n");
        throw std::runtime_error("Problems during digitizer configuration, err code : " + std::to_string(ret));
        return ret;
    } else {
        return 0;
    }
}

/*******************************************************************/

std::vector<int> CaenN6725::get_temperatures() const
{
    std::vector<int> temperatures({});
    uint32_t temp;
    for (uint ch=0; ch<MaxNChannels; ch++)
        {
          CAEN_DGTZ_ReadTemperature(handle_, ch, &temp);
          temperatures.push_back(temp);   
       }
    return temperatures;

}

/*******************************************************************/


/*! \fn      static long get_time()
*   \brief   Get time in milliseconds
*   \return  time in msec */
// stolen from CAEN examples
long CaenN6725::get_time() const
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
};


/*******************************************************************/

void CaenN6725::configure_channel(int channel, CAEN_DGTZ_DPP_PHA_Params_t* params)
{
    uint32_t mask;
    switch (channel){
        case (0) : {
                    mask = 0x01;
                    break;   
                 }
        case (1) : {
                    mask = 0x02;
                    break;
                 }
        case (2) : {
                    mask = 0x04;
                    break;
                 }
        case (3) : {
                    mask = 0x08;
                    break;
                 }
        case (4) : {
                    mask = 0x10;
                    break;
                 }
        case (5) : {
                    mask = 0x20;
                    break;
                 }
        case (6) : {
                    mask = 0x40;
                    break;
                 }
        case (7) : {
                    mask = 0x80;
                    break;
                 }
    }
    mask = 0xff;

    // channel mask 0xff means all channels ( 8bit set)
    current_error_ = CAEN_DGTZ_SetDPPParameters(handle_, mask, params);
    if (current_error_ != 0) throw std::runtime_error("Problems configuring channel, err code " + std::to_string(current_error_));

};

/*******************************************************************/

void CaenN6725::configure_all_channels(CAEN_DGTZ_DPP_PHA_Params_t* params)
{
    // all bits set to 1
    uint32_t mask = 0xff;
    // channel mask 0xff means all channels ( 8bit set)
    current_error_ = CAEN_DGTZ_SetDPPParameters(handle_, mask, params);
    if (current_error_ != 0) throw std::runtime_error("Problems configuring all channels, err code " + std::to_string(current_error_));
    
}

/*******************************************************************/

void CaenN6725::start_acquisition()
{
    current_error_ = CAEN_DGTZ_SWStartAcquisition(handle_);
    if (current_error_ != 0) throw std::runtime_error("Problems configuring all channels, err code " + std::to_string(current_error_));
}

/*******************************************************************/

void CaenN6725::calibrate()
{
    current_error_ = CAENDGTZ_API CAEN_DGTZ_Calibrate(handle_);
    if (current_error_ != 0) throw std::runtime_error("Issue during calibration err code: " + std::to_string(current_error_));

}

/*******************************************************************/

int CaenN6725::get_handle() const
{
    return handle_;
}

/*******************************************************************/

// FIXME: pro;er close function
CaenN6725::~CaenN6725()
{
    CAEN_DGTZ_SWStopAcquisition(handle_);
    CAEN_DGTZ_FreeReadoutBuffer(&buffer_);
    //CAEN_DGTZ_FreeDPPEvents(handle_, &events_);
    //CAEN_DGTZ_FreeDPPWaveforms(handle_, waveform_);
    CAEN_DGTZ_CloseDigitizer(handle_);
    //    for (ch = 0; ch < MaxNChannels; ch++)
    //        if (EHisto[b][ch] != NULL)
    //            free(EHisto[b][ch]);
    //}   
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

/*******************************************************************/


