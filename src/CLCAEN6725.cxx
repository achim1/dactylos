#include <stdexcept>

#include <CAENDigitizerType.h>
#include <CAENDigitizer.h>

#include "CLCAEN6725.hh"

#include "gaps/GOptionParser.hh"

#include <fstream>

/***************************************************************/

CaenN6725::CaenN6725()
{
    // make this specific for our case
    // third 0 is VMEBaseAddress, which must be 0 for direct USB connections
    current_error_ = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB, 0, 0, 0, &handle_);
    if (current_error_ == -1)
        {
            std::cout << "Can not find digitizer at USB bus 0, trying others" << std::endl;
            for (int busnr=1; busnr<20; busnr++)
            //while (current_error_ !=0 )
                {
                 std::cout << "Trying ..." << busnr << std::endl;
                 current_error_ = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB, 1, 0, 0, &handle_);
                 if (current_error_ == 0)  break;
                }
        }
    if (current_error_ !=0 ) throw std::runtime_error("Can not open digitizer err code: " + std::to_string(current_error_));

    /* Reset the digitizer */
    current_error_ = CAEN_DGTZ_Reset(handle_);
    if (current_error_ !=0 ) throw std::runtime_error("Can not reset digitizer err code:" + std::to_string(current_error_));

    // FIXME: WHat is this doing?
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

//    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, ANALOG_TRACE_1, CAEN_DGTZ_DPP_VIRTUALPROBE_Delta2);


    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, ANALOG_TRACE_1, CAEN_DGTZ_DPP_VIRTUALPROBE_Trapezoid);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP virtual probe trace 1 err ode: " + std::to_string(current_error_));

    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, ANALOG_TRACE_2, CAEN_DGTZ_DPP_VIRTUALPROBE_Input);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP virtual probe trace 2 err ode: " + std::to_string(current_error_));

    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, DIGITAL_TRACE_1, CAEN_DGTZ_DPP_DIGITALPROBE_Peaking);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP virtual probe trace 2 err ode: " + std::to_string(current_error_));
    root_file_   = new TFile("digitizer_output.root", "RECREATE");
    energy_ch_.reserve(8);
    waveform_ch_.reserve(8);
    std::string ch_name = "ch";
    for (int k=0;k<8;k++)
        {
            ch_name = std::string("ch") + std::to_string(k);           
            channel_trees_.push_back(new TTree(ch_name.c_str(), ch_name.c_str()));
            channel_trees_[k]->Branch("energy", &energy_ch_[k]);
            channel_trees_[k]->Branch("waveform", &waveform_ch_[k]);
        } 
};

/***************************************************************/

CaenN6725::CaenN6725(DigitizerParams_t params) : CaenN6725()
{
    // remember the active cannels
    active_channels_  = params.ChannelMask;

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

std::vector<std::vector<CAEN_DGTZ_DPP_PHA_Event_t>> CaenN6725::read_data()
{
    for (int k = 0; k<get_nchannels(); k++)
        {num_events_[k] = 0;}

    std::vector<CAEN_DGTZ_DPP_PHA_Event_t> channel_events;
    std::vector<std::vector<CAEN_DGTZ_DPP_PHA_Event_t>> thisevents;
    int waveform_size;
    int16_t *waveform_trace;
    //uint8_t *waveform_trace;
    current_error_ = CAEN_DGTZ_ReadData(handle_, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer_, &buffer_size_);
    if (current_error_ != 0) 
        {
            std::cout << "error while getting data" << current_error_ << std::endl;
            return thisevents;
        }
    if (buffer_size_ == 0)
        {
            return thisevents;
        }
    //if (current_error_ != 0) throw std::runtime_error("Error while reading data from the digitizer, err code " + std::to_string(current_error_));
    current_error_ =  CAEN_DGTZ_GetDPPEvents(handle_, buffer_, buffer_size_, (void**)(events_),num_events_);
    if (current_error_ != 0)
        {
            std::cout << "error while getting data" << current_error_ << std::endl;
            return thisevents;
        }


    root_file_->cd();
    std::vector<int16_t> thiswf;
    //thiswf.reserve(8)
    uint traceId(0);
    for (int ch=0;ch<get_nchannels();ch++)
        {
            channel_events = {};
            waveform_ch_.clear();
            for (int k=0;k<8;k++)
                {waveform_ch_.push_back(std::vector<int16_t>{});}
            for (int ev=0;ev<num_events_[ch];ev++)
                {
                    channel_events.push_back(events_[ch][ev]);
                    

                    if (decode_waveforms_)
                        {
                            //if (events_[ch][ev].Energy == 0)
                            //    {continue;}
                            CAEN_DGTZ_DecodeDPPWaveforms(handle_, &events_[ch][ev], waveform_);
                        energy_ch_[ch] = events_[ch][ev].Energy;
                        waveform_size = (int) waveform_->Ns; // number of samples
                        waveform_trace = waveform_->Trace2;
                        thiswf = {};
                        int end_wf = sizeof(waveform_trace)/sizeof(waveform_trace[0]);
                        thiswf.reserve(waveform_size);    
                        //for(int i=0; i<waveform_size; i++)
                        //    {thiswf.push_back(waveform_trace[i]);}
                        //for (int16_t i : *waveform_trace)
                        //    {thiswf.push_back(i);}
                        thiswf = std::vector<int16_t>(waveform_trace, waveform_trace + waveform_size);
                        waveform_ch_.at(ch) = thiswf;
                        channel_trees_[ch]->Fill();
                        //channel_trees_[ch]->Write();
                        ++traceId;
                        //waveform_->Trace2;
                        //waveform_->DTrace1;
                        //waveform_->DTrace2;

                        }
                }
            channel_trees_[ch]->Write();
            
            thisevents.push_back(channel_events);
            //thisevents[ch] = events_[ch];
        }
    //CAEN_DGTZ_DPP_PHA_Event_t (*thisevents)[]
    return thisevents;
}

/***************************************************************/

std::vector<int> CaenN6725::get_n_events()
{
    std::vector<int> n_events({});
    for (uint ch=0; ch<get_nchannels(); ch++)
        {
            n_events.push_back(num_events_[ch]);
        }
    return n_events;
}

/***************************************************************/

void CaenN6725::end_acquisition()
{
    CAEN_DGTZ_SWStopAcquisition(handle_);
}

/***************************************************************/

int CaenN6725::get_nchannels() const
{
    return max_n_channels_;
}

/***************************************************************/

std::vector<int> CaenN6725::get_temperatures() const
{
    std::vector<int> temperatures({});
    uint32_t temp;
    for (uint ch=0; ch<get_nchannels(); ch++)
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

void CaenN6725::enable_waveform_decoding()
{
  decode_waveforms_ = true;
}

/*******************************************************************/

void CaenN6725::set_input_dynamic_range(DynamicRange range)
{
    // 32 bit mask, but only bit0 caries information
    // settings for bit 0
    // 0 = 2Vpp
    // 1 = 0.5Vpp
    //uint32_t drange = -1;
    //if (range == DynamicRange::VPP2)
    //    {uint32_t drange = 0;}
    //if (range == DynamicRange::VPP05)
    //    {uint32_t drange = 1}
    //if (range == -1)
    //    {throw std::runtime_error("Can not understand input dynamic range value!");}    
    current_error_ = CAEN_DGTZ_WriteRegister(handle_,0x8028, (uint32_t) range);
    if (current_error_ != 0) throw std::runtime_error("Problems setting dynamic range, err code " + std::to_string(current_error_));
}

/*******************************************************************/

std::vector<uint32_t>CaenN6725::get_input_dynamic_range()
{
    uint32_t drange;
    std::vector<uint32_t> channel_registers({0x1028, 0x1128, 0x1228, 0x1328, 0x1428, 0x1528, 0x1628, 0x1728});
    std::vector<uint32_t> dranges;
    for (auto ch : channel_registers)
        {
            current_error_ = CAEN_DGTZ_ReadRegister(handle_, ch, &drange);
            if (current_error_ != 0) throw std::runtime_error("Can not get  dynamic range for ch address " + std::to_string(ch) + " err code " + std::to_string(current_error_));
            dranges.push_back(drange);
         }
    return dranges;
}


/*******************************************************************/

void CaenN6725::configure_channels(CAEN_DGTZ_DPP_PHA_Params_t* params)
{
    // channel mask 0xff means all channels ( 8bit set)
    current_error_ = CAEN_DGTZ_SetDPPParameters(handle_, active_channels_, params);
    if (current_error_ != 0) throw std::runtime_error("Problems configuring channel, err code " + std::to_string(current_error_));
};


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

void CaenN6725::set_baseline_offset(int channel, int offset)
{
    // from the manual
    /*
    This function sets the 16-bit DAC that adds a DC offset to the input signal to adapt it to the dynamic range of the ADC.
By default, the DAC is set to middle scale (0x7FFF) which corresponds to a DC offset of -Vpp/2, where Vpp is the voltage
range (peak to peak) of the ADC. This means that the input signal can range from -Vpp/2 to +Vpp/2. If the DAC is set to
0x0000, then no DC offset is added, and the range of the input signal goes from -Vpp to 0. Conversely, when the DAC is
set to 0xFFFF, the DC offset is –Vpp and the range goes from 0 to +Vpp. The DC offset can be set on channel basis except
for the x740 in which it is set on group basis; in this case, you must use the Set / GetGroupDCOffset functions.
    */
    // offset has to be in DAC values!
    current_error_ = CAEN_DGTZ_SetChannelDCOffset(handle_,channel, offset);
    if (current_error_ != 0) throw std::runtime_error("Can not set baseline offset for ch " + std::to_string(channel) + " err code: " + std::to_string(current_error_));
}

/*******************************************************************/

uint32_t CaenN6725::get_baseline_offset(int channel)
{
    // offset has to be in DAC values!
    uint32_t offset;
    current_error_ = CAEN_DGTZ_GetChannelDCOffset(handle_,channel, &offset);
    if (current_error_ != 0) throw std::runtime_error("Can not get baseline offset for ch " + std::to_string(channel) + " err code: " + std::to_string(current_error_));
    return offset;
}

/*******************************************************************/

// FIXME: pro;er close function
CaenN6725::~CaenN6725()
{
    std::cout << "Closing digitizer..." << std::endl;
    CAEN_DGTZ_SWStopAcquisition(handle_);
    CAEN_DGTZ_FreeReadoutBuffer(&buffer_);
    //CAEN_DGTZ_FreeDPPEvents(handle_, &events_);
    //CAEN_DGTZ_FreeDPPWaveforms(handle_, waveform_);
    CAEN_DGTZ_CloseDigitizer(handle_);
    //root_file_->Write();
    root_file_->Close();
    //delete root_file_;
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
int CaenN6725::SaveWaveform(int size, int16_t *WaveData)
{
    for(int i=0; i<size; i++)
    //    fprintf(fh, "%d\s", WaveData[i]); //&((1<<MAXNBITS)-1)
        outfile_ << WaveData[i] << " ";
    outfile_ << "\n";
    //fprintf(fh, "\n");
    //fclose(fh);
    return 0;
}

