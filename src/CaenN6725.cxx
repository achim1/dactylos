#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cmath>

#include <CAENDigitizerType.h>
#include <CAENDigitizer.h>

#include "CaenN6725.hh"
#include "TTree.h"

/***************************************************************/

CaenN6725::CaenN6725()
{
};

/***************************************************************/

CaenN6725::CaenN6725(DigitizerParams_t params) : CaenN6725()
{
    connect();
    current_error_ = CAEN_DGTZ_SetDPPAcquisitionMode(handle_, params.AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP acquisition mode err code:" + std::to_string(current_error_));
    std::cout << "Setting record length " << params.RecordLength << std::endl;
    // Set the number of samples for each waveform
    current_error_ = CAEN_DGTZ_SetRecordLength(handle_, params.RecordLength);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set record length err code: " + std::to_string(current_error_));

    // also set the record length internally
    recordlength_ = params.RecordLength;

    // Set the I/O level (CAEN_DGTZ_IOLevel_NIM or CAEN_DGTZ_IOLevel_TTL)
    current_error_ = CAEN_DGTZ_SetIOLevel(handle_, params.IOlev);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set IO level err code:" + std::to_string(current_error_));

    // Set the enabled channels
    // remember the active cannels
    active_channel_bitmask_  = params.ChannelMask;
    std::cout << "Setting channel mask " << params.ChannelMask << std::endl;
    current_error_ = CAEN_DGTZ_SetChannelEnableMask(handle_, params.ChannelMask);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set channel enable mask err code:" + std::to_string(current_error_));

    for(unsigned int i=0; i<max_n_channels_; i++) {
        //if (params.ChannelMask & (1<<i)) {
        if (is_active(i)) {
            // Set the Pre-Trigger size (in samples)
            current_error_ = CAEN_DGTZ_SetDPPPreTriggerSize(handle_, i, 1000);
            if (current_error_ !=0 ) throw std::runtime_error("Can not set dpp trigger sixe err code:" + std::to_string(current_error_));

            // Set the polarity for the given channel (CAEN_DGTZ_PulsePolarityPositive or CAEN_DGTZ_PulsePolarityNegative)
            current_error_ = CAEN_DGTZ_SetChannelPulsePolarity(handle_, i, params.PulsePolarity);
            if (current_error_ !=0 ) throw std::runtime_error("Can not set pulse polarity err code:" + std::to_string(current_error_));
            // Check the number of events per aggregate
            //uint32_t num_events;
            //current_error_ = CAEN_DGTZ_SetNumEventsPerAggregate (handle_,32 , i);
            //if (current_error_ !=0 ) throw std::runtime_error("Can not get events per aggregate err code:" + std::to_string(current_error_));
            //current_error_ = CAEN_DGTZ_GetNumEventsPerAggregate (handle_,&num_events , i);
            //if (current_error_ !=0 ) throw std::runtime_error("Can not get events per aggregate err code:" + std::to_string(current_error_));
            //std::cout << "Get " << num_events << " per aggregate for ch " << i << std::endl;

        }
    }

    // Set how many events to accumulate in the board memory before being available for readout
    current_error_ = CAEN_DGTZ_SetDPPEventAggregation(handle_, params.EventAggr, 0);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set dpp event agregation err code:" + std::to_string(current_error_));

};

/***************************************************************/

int CaenN6725::get_handle()
{
    return handle_;
}

/***************************************************************/

void CaenN6725::set_handle(int handle)
{
    handle_ = handle;
}

/***************************************************************/

void CaenN6725::connect()
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
                 current_error_ = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB, busnr, 0, 0, &handle_);
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

    // setting the virtual probes determines what we see in the different 
    // registers for the traces. The digitzer can hold 2 registers each for 
    // analog and digital traces which can show different traces. 
    // This can also be set later on
    set_virtualprobe1(DPPVirtualProbe1::Input);
    //set_virtualprobe1(CAEN_DGTZ_DPP_VIRTUALPROBE_Input);
    //current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, ANALOG_TRACE_1, CAEN_DGTZ_DPP_VIRTUALPROBE_Input);
    //if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP virtual probe trace 1 err ode: " + std::to_string(current_error_));

    // set the second one the the trapezoid
    set_virtualprobe2(DPPVirtualProbe2::TrapezoidReduced);
    //current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, ANALOG_TRACE_2, CAEN_DGTZ_DPP_VIRTUALPROBE_TrapezoidReduced);
    //if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP virtual probe trace 2 err ode: " + std::to_string(current_error_));

    // set the digitial trace to the peaking time, the other digital trace will always be 
    // the trigger
    set_digitalprobe1(DPPDigitalProbe1::Peaking);
    //current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, DIGITAL_TRACE_1, CAEN_DGTZ_DPP_DIGITALPROBE_Peaking);
    //if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP virtual probe trace 2 err ode: " + std::to_string(current_error_));
    is_connected_ = true;
}

/***************************************************************/

void CaenN6725::show_supported_probes()
{
    // get information about the available virtual probes
    int probes[MAX_SUPPORTED_PROBES];
    int numprobes;
    current_error_ = CAEN_DGTZ_GetDPP_SupportedVirtualProbes(handle_,ANALOG_TRACE_1,  probes, &numprobes);
    if (current_error_ !=0 ) throw std::runtime_error("Can not get available virtual probes for trace 1 err ode: " + std::to_string(current_error_));
    for (auto k : probes)
        {std::cout << "trace 1 probe : " << k << std::endl; }
    std::cout << "trace 1 allows for " << numprobes << " virtual probes" << std::endl;

    current_error_ = CAEN_DGTZ_GetDPP_SupportedVirtualProbes(handle_,ANALOG_TRACE_2,  probes, &numprobes);
    if (current_error_ !=0 ) throw std::runtime_error("Can not get available virtual probes for trace 2 err ode: " + std::to_string(current_error_));
    for (auto k : probes)
        {std::cout << "trace 2 probe : " << k << std::endl; }
    std::cout << "trace 2 allows for " << numprobes << " virtual probes" << std::endl;

    current_error_ = CAEN_DGTZ_GetDPP_SupportedVirtualProbes(handle_,DIGITAL_TRACE_1,  probes, &numprobes);
    if (current_error_ !=0 ) throw std::runtime_error("Can not get available virtual probes for trace 1 err ode: " + std::to_string(current_error_));
    for (auto k : probes)
        {std::cout << "dtrace 1 probe : " << k << std::endl; }
    std::cout << "dtrace 1 allows for " << numprobes << " virtual probes" << std::endl;

    current_error_ = CAEN_DGTZ_GetDPP_SupportedVirtualProbes(handle_,DIGITAL_TRACE_2,  probes, &numprobes);
    if (current_error_ !=0 ) throw std::runtime_error("Can not get available virtual probes for trace 2 err ode: " + std::to_string(current_error_));
    for (auto k : probes)
        {std::cout << "dtrace 2 probe : " << k << std::endl; }
    std::cout << "dtrace 2 allows for " << numprobes << " virtual probes" << std::endl;
}

/***************************************************************/

void CaenN6725::set_virtualprobe1(DPPVirtualProbe1 vprobe1)
{
    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, ANALOG_TRACE_1, (int)vprobe1);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP virtual probe for trace 1 err ode: " + std::to_string(current_error_));
}

/***************************************************************/

void CaenN6725::set_virtualprobe2(DPPVirtualProbe2 vprobe2)
{
    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, ANALOG_TRACE_2, (int)vprobe2);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP virtual probe for trace 2 err ode: " + std::to_string(current_error_));
}

/***************************************************************/

void CaenN6725::set_digitalprobe1(DPPDigitalProbe1 dprobe1)
{
    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, DIGITAL_TRACE_1, (int)dprobe1);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP digital probe for dtrace 1 err ode: " + std::to_string(current_error_));
}
/***************************************************************/

void CaenN6725::set_digitalprobe2(DPPDigitalProbe2 dprobe2)
{
    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, DIGITAL_TRACE_2, (int)dprobe2);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP ditial probe for dtrace 2 err ode: " + std::to_string(current_error_));
}

/***************************************************************/

CAEN_DGTZ_ErrorCode CaenN6725::get_last_error() const
{
    return current_error_;
}

/***************************************************************/

inline void CaenN6725::fill_analog_trace1_()
{
    atrace1_ = waveform_->Trace1;
    analog_trace1_ = std::vector<int16_t>(atrace1_, atrace1_ + trace_ns_);
}

/***************************************************************/

inline void CaenN6725::fill_analog_trace2_()
{
    atrace2_ = waveform_->Trace2;
    analog_trace2_ = std::vector<int16_t>(atrace2_, atrace2_ + trace_ns_);
}

/***************************************************************/

inline void CaenN6725::fill_digital_trace1_()
{
    dtrace1_ = waveform_->DTrace1;
    digital_trace1_ = std::vector<uint8_t>(dtrace1_, dtrace1_ + trace_ns_);
}

/***************************************************************/

inline void CaenN6725::fill_digital_trace2_()
{
    dtrace2_ = waveform_->DTrace2;
    digital_trace2_ = std::vector<uint8_t>(dtrace2_, dtrace2_ + trace_ns_);
}

/***************************************************************/

int CaenN6725::get_trigger_point()
{
    for (int k=0; k<digital_trace2_.size(); k++)
        {   //std::cout << digital_trace2_[k] << std::endl;
            if (digital_trace2_[k] > 0) return k;
        }
    return digital_trace2_.size();
}

/***************************************************************/

CAEN_DGTZ_BoardInfo_t CaenN6725::get_board_info()
{
    current_error_ = CAEN_DGTZ_GetInfo(handle_, &board_info_);
    if (current_error_ != 0) throw std::runtime_error("Error while getting board infoe, err code " + std::to_string(current_error_));
    return board_info_;
}

/***************************************************************/

std::vector<int16_t> CaenN6725::get_analog_trace1()
{
    return analog_trace1_;
}
/***************************************************************/

std::vector<int16_t> CaenN6725::get_analog_trace2()
{
    return analog_trace2_;
}

/***************************************************************/

std::vector<uint8_t> CaenN6725::get_digital_trace1()
{
    return digital_trace1_;
}

/***************************************************************/

std::vector<uint8_t> CaenN6725::get_digital_trace2()
{
    return digital_trace2_;
}

/***************************************************************/

uint16_t CaenN6725::get_energy()
{
    return energy_;
}

/***************************************************************/

void CaenN6725::set_channel_dc_offset(int channel, int offset)
{
    // Set a DC offset to the input signal to adapt it to digitizer's dynamic range
    // from the manual:
    // This function sets the 16-bit DAC that adds a DC offset to the input signal to adapt it to the dynamic range of the ADC.
    // By default, the DAC is set to middle scale (0x7FFF) which corresponds to a DC offset of -Vpp/2, where Vpp is the voltage
    // range (peak to peak) of the ADC. This means that the input signal can range from -Vpp/2 to +Vpp/2. If the DAC is set to
    // 0x0000, then no DC offset is added, and the range of the input signal goes from -Vpp to 0. Conversely, when the DAC is
    // set to 0xFFFF, the DC offset is –Vpp and the range goes from 0 to +Vpp. The DC offset can be set on channel basis except
    // for the x740 in which it is set on group basis; in this case, you must use the Set / GetGroupDCOffset functions.
    current_error_ = CAEN_DGTZ_SetChannelDCOffset(handle_, channel, 0x8000);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set channel dc offset err code:" + std::to_string(current_error_));
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

std::vector<std::vector<CAEN_DGTZ_DPP_PHA_Event_t>> CaenN6725::read_data(int display_channel)
{
    // check the readout status
    uint32_t acqstatus;
    current_error_ = CAEN_DGTZ_ReadRegister(handle_, 0x8104, &acqstatus);
    while (! ( acqstatus && (1 << 3))) // the 3rd bit is the acquisition status
        {
            // nothing to readout
            current_error_ = CAEN_DGTZ_ReadRegister(handle_, 0x8104, &acqstatus);
        }
    //if (! ( acqstatus && (1 << 4))) // the 3rd bit is the acquisition status
    //    {
    //        return; // no channel in full status
    //    }


    for (int k = 0; k<get_nchannels(); k++)
        {num_events_[k] = 0;}

    std::vector<CAEN_DGTZ_DPP_PHA_Event_t> channel_events;
    std::vector<std::vector<CAEN_DGTZ_DPP_PHA_Event_t>> thisevents;
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

    if (root_file_) root_file_->cd();

    for (int ch=0;ch<get_nchannels();ch++)
        {
            channel_events = {};
            if (decode_waveforms_)
                {
                    waveform_ch_.clear();
                    for (int k=0;k<8;k++)
                        {waveform_ch_.push_back({});}
                }
    
            for (int ev=0;ev<num_events_[ch];ev++)
                {
                    channel_events.push_back(events_[ch][ev]);
                    energy_ch_[ch] = events_[ch][ev].Energy;
                    energy_        = events_[ch][ev].Energy;
                    if (decode_waveforms_)
                        {
                            CAEN_DGTZ_DecodeDPPWaveforms(handle_, &events_[ch][ev], waveform_);
                            trace_ns_ = waveform_->Ns;
                            fill_analog_trace1_();
                            fill_analog_trace2_();
                            fill_digital_trace1_();
                            fill_digital_trace2_();
                            waveform_ch_.at(ch) = get_analog_trace1();
                            //channel_trees_[ch]->Write();
                            //++traceId;
                        }
                    if (root_file_) channel_trees_[ch]->Fill();
                }

            channel_trees_[ch]->Write();
            n_events_acq_[ch] += num_events_[ch]; 
            thisevents.push_back(channel_events);
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

std::vector<long> CaenN6725::get_n_events_tot()
{
    return n_events_acq_;
}


/***************************************************************/

void CaenN6725::fast_readout_()
{
    // check the readout status
    uint32_t acqstatus;
    current_error_ = CAEN_DGTZ_ReadRegister(handle_, 0x8104, &acqstatus);
    if (! ( acqstatus && (1 << 3))) // the 3rd bit is the acquisition status
        {
            return; // nothing to readout
        }
    // wait till the buffer is full
    if (! ( acqstatus && (1 << 4))) // the 3rd bit is the acquisition status
        {
            return; // no channel in full status
        }
    for (int k = 0; k<get_nchannels(); k++)
        {num_events_[k] = 0;}

    current_error_ = CAEN_DGTZ_ReadData(handle_, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer_, &buffer_size_);
    if (current_error_ != 0) 
        {
            std::cout << "error while getting data" << current_error_ << std::endl;
            return;
        }
    if (buffer_size_ == 0)
        {
            return;
        }
    //if (current_error_ != 0) throw std::runtime_error("Error while reading data from the digitizer, err code " + std::to_string(current_error_));
    current_error_ =  CAEN_DGTZ_GetDPPEvents(handle_, buffer_, buffer_size_, (void**)(events_),num_events_);
    if (current_error_ != 0)
        {
            std::cout << "error while getting data" << current_error_ << std::endl;
            return;
        }

    if (root_file_) root_file_->cd();
    if (decode_waveforms_)
    {
        waveform_ch_.clear();
        waveform_ch_.reserve(get_nchannels());
        //trigger_ch_ = std::vector<int>(8,0);
        trigger_ch_.clear();
        trigger_ch_.reserve(get_nchannels());
        for (int k=0; k<get_nchannels(); k++)
            {waveform_ch_.push_back({});
             trigger_ch_.push_back(-1);}
    }
    for (int ch=0;ch<get_nchannels();ch++)
      {
        for (int ev=0;ev<num_events_[ch];ev++)
          {
            energy_ch_[ch] = events_[ch][ev].Energy;
            //energy_        = events_[ch][ev].Energy;
            if (decode_waveforms_)
              {
                  CAEN_DGTZ_DecodeDPPWaveforms(handle_, &events_[ch][ev], waveform_);
                  // fast mode, only do trace1
                  trace_ns_ = waveform_->Ns;
                  fill_analog_trace1_();
                  fill_digital_trace2_();
                  trigger_ch_.at(ch)  = get_trigger_point(); 
                  //std::cout << get_trigger_point() << std::endl;
                  waveform_ch_.at(ch) = get_analog_trace1();
                  channel_trees_[ch]->Fill();
              }
          }
        channel_trees_[ch]->Write();
        n_events_acq_[ch] += num_events_[ch];
      }
    return;
}

/***************************************************************/

void CaenN6725::end_acquisition()
{
    CAEN_DGTZ_SWStopAcquisition(handle_);
    root_file_->Close();
}

/***************************************************************/

int CaenN6725::get_nchannels() const
{
    return max_n_channels_;
}

/***************************************************************/

bool CaenN6725::is_active(int channel) const
{
    return (active_channel_bitmask_ & (1<<channel)); 
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

void CaenN6725::configure_channel(unsigned int channel,CAEN_DGTZ_DPP_PHA_Params_t* params)
{
    // channel mask 0xff means all channels ( 8bit set)
    if (channel > 7) throw std::runtime_error("Channel has to be < 8");
    unsigned int channelmask = pow(2, channel);
    current_error_ = CAEN_DGTZ_SetDPPParameters(handle_, channelmask, params);
    if (current_error_ != 0) throw std::runtime_error("Problems configuring channel, err code " + std::to_string(current_error_));
};


/*******************************************************************/

void CaenN6725::start_acquisition()
{
    if (current_error_ != 0) throw std::runtime_error("Problems configuring all channels, err code " + std::to_string(current_error_));
    root_file_   = new TFile(rootfile_name_.c_str(), "RECREATE");
    if (!root_file_) throw std::runtime_error("Problems with root file " + rootfile_name_);
    channel_trees_.clear();
    energy_ch_.clear();
    waveform_ch_.clear();
    trigger_ch_.clear();
    energy_ch_.reserve(8);
    waveform_ch_.reserve(8);
    channel_trees_.reserve(8);
    trigger_ch_.reserve(8);
    std::string ch_name = "ch";
    for (int k=0;k<8;k++)
        {
            ch_name = std::string("ch") + std::to_string(k);           
            channel_trees_.push_back(new TTree(ch_name.c_str(), ch_name.c_str()));
            channel_trees_[k]->Branch("energy", &energy_ch_[k]);
            if (decode_waveforms_)
                {
                    channel_trees_[k]->Branch("waveform", &waveform_ch_[k]);
                    channel_trees_[k]->Branch("trigger", &trigger_ch_[k]);
                }
        } 
    n_events_acq_ = std::vector<long>(get_nchannels(), 0);
    current_error_ = CAEN_DGTZ_SWStartAcquisition(handle_);
}

/*******************************************************************/

int CaenN6725::get_current_sampling_rate() 
{
    int sampling_rate = 250e6;
    int probe;
    current_error_ = CAEN_DGTZ_GetDPP_VirtualProbe(handle_, ANALOG_TRACE_2, &probe);
    if (current_error_ != 0) throw std::runtime_error("Can not get virtual probe 2, err code: "  + std::to_string(current_error_));
    // for dual trace mode, the sampling rate is only half
    if (probe != CAEN_DGTZ_DPP_VIRTUALPROBE_None)
        {sampling_rate = sampling_rate/2;}
    return sampling_rate;
}


/*******************************************************************/

void CaenN6725::calibrate()
{
    current_error_ = CAENDGTZ_API CAEN_DGTZ_Calibrate(handle_);
    if (current_error_ != 0) throw std::runtime_error("Issue during calibration err code: " + std::to_string(current_error_));

}


/*******************************************************************/

uint32_t CaenN6725::get_channel_dc_offset(int channel)
{
    // offset has to be in DAC values!
    uint32_t offset;
    current_error_ = CAEN_DGTZ_GetChannelDCOffset(handle_,channel, &offset);
    if (current_error_ != 0) throw std::runtime_error("Can not get baseline offset for ch " + std::to_string(channel) + " err code: " + std::to_string(current_error_));
    return offset;
}

/*******************************************************************/

void CaenN6725::continuous_readout(unsigned int seconds)
{
    // this is meant for fast continueous readout
    // set second vprobe to None, so we get the full 
    // sampling rate for the waveform
    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, ANALOG_TRACE_2, CAEN_DGTZ_DPP_VIRTUALPROBE_None);
    if (current_error_ != 0) throw std::runtime_error("Can not set virtual probe to None, err code: "  + std::to_string(current_error_));
    
    if (!decode_waveforms_)
        {
            current_error_ = CAEN_DGTZ_SetDPPAcquisitionMode(handle_, CAEN_DGTZ_DPP_ACQ_MODE_List, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);    
            if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP acquisition mode err code:" + std::to_string(current_error_));
        }
    else 
        {
            current_error_ = CAEN_DGTZ_SetDPPAcquisitionMode(handle_, CAEN_DGTZ_DPP_ACQ_MODE_Mixed, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);    
            if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP acquisition mode err code:" + std::to_string(current_error_));

        }
    long now_time = get_time()/1000;
    long last_time = now_time;
    long delta_t = 0;
    //GProgressBar progress(seconds);
    int progress_step = 0;
    int last_progress_step = 0;
    std::cout << "Starting readout" << std::endl;
    while (delta_t < seconds)
        {
            fast_readout_();
            now_time = get_time()/1000;
            delta_t +=  now_time  - last_time;
            last_time = now_time;
            progress_step += delta_t;
            if (progress_step - last_progress_step > 5)
                {
                    //for (int j=0; j<5;j++)
                    //    {++progress;}
                    last_progress_step = progress_step;
                }
        }
}


/*******************************************************************/

void CaenN6725::set_rootfilename(std::string fname)
{
    rootfile_name_ = fname;

}

/*******************************************************************/

// FIXME: pro;er close function
CaenN6725::~CaenN6725()
{
    if (is_connected_)
    {
        std::cout << "Closing digitizer..." << std::endl;
        CAEN_DGTZ_SWStopAcquisition(handle_);
        CAEN_DGTZ_FreeReadoutBuffer(&buffer_);
        //CAEN_DGTZ_FreeDPPEvents(handle_, &events_);
        //CAEN_DGTZ_FreeDPPWaveforms(handle_, waveform_);
        CAEN_DGTZ_CloseDigitizer(handle_);
        //root_file_->Write();
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
    }
};


