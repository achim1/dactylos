#include <stdexcept>
#include <fstream>
#include <cmath>

#include <CAENDigitizerType.h>
#include <CAENDigitizer.h>

#include "CaenN6725.hh"
#include "TTree.h"

// a string represenation of the error codes
std::string error_code_to_string(CAEN_DGTZ_ErrorCode err)
{
std::string retval;
switch((int)err) 
  {
    case 0 :
      { 
        retval =  "Err code : 0 -- 'Operation completed sucessfully'";
        break;
      }
    case -1:
      { 
        retval =  "Err code : -1 -- 'Communication error'";
        break;
      }
    case -2: 
      { 
        retval = "Err code : -2 -- 'Unspecified error'";
        break;
      }
    case -3: 
      { 
        retval = "Err code : -3 -- 'Invalid parameter'";
        break;
      }
    case -4:
      { 
        retval = "Err code : -4 -- 'Invalid link type'";
        break;
      }
    case -5: 
      { 
        retval = "Err code : -5 -- 'Invalid device handle'";
        break;
      }
    case -6: 
      { 
        retval = "Err code : -6 -- 'Maximum number of devices exceeded'";
        break;
      }
    case -7: 
      { 
        retval =  "Err code : -7 -- 'Operation not allowed on this board'";
        break;
      }
    case -8:
      { 
        retval = "Err code : -8 -- 'The interrupt level is bad'" ;
        break;
      }
    case -9: 
      { 
        retval = "Err code : -9 -- 'The event number is bad'";
        break;
      }
    case -10: 
      { 
        retval = "Err code : -10 -- 'Unable to read the registry'";
        break;
      }
    case -11:
      { 
        retval = "Err code : -11 -- 'Unable to write into the registry'";
        break;
      }
    case -13:
      { 
        retval = "Err code : -13 -- 'The channel number is invalid'";
        break;
      }
    case -14:
      { 
        retval = "Err code : -14 -- 'The channel is busy'";
        break;
      }
    case -15:
      { 
        retval = "Err code : -15 -- 'Invalid FPIO mode'";
        break;
      }
    case-16: 
      { 
        retval = "Err code : -16 -- 'Wrong acquisitiion mode'";
        break;
      }
    case -17: 
      { 
        retval = "Err code : -17 -- 'The function is not allowed for this module'";
        break;
      }
    case -18: 
      { 
        retval = "Err code : -18 -- 'Communication Timeout'";
        break;
      }
    case -19: 
      { 
        retval = "Err code : -19 -- 'The buffer is invalid'";
        break;
      }
    case -20: 
      { 
        retval = "Err code : -20 -- 'The event is not found'";
        break;
      }
    case -21: 
      { 
        retval = "Err code : -21 -- 'The event is invalid'";
        break;
      }
    case -22: 
      { 
        retval = "Err code : -22 -- 'Out of memory'";
        break;
      }
    case -23: 
      { 
        retval = "Err code : -23 -- 'Unable to calibrate the board'";
        break;
      }
    case -24: 
      { 
        retval = "Err code : -24 -- 'Unable to open the digitizer'";
        break;
      }
    case -25: 
      { 
        retval = "Err code : -25 -- 'The digitizer is already open'";
        break;
      }
    case -26:
      { 
        retval = "Err code : -26 -- 'The digitizer is not ready to operate'";
        break;
      }
    case -27:
      { 
        retval = "Err code : -27 -- 'The digitizer has not the IRQ configure'";
        break;
      }
    case -28:
      { 
        retval = "Err code : -28 -- 'The digitizer flash memory is corrupted'";
        break;
      }
    case -29: 
      { 
        retval = "Err code : -29 -- 'The digitizer dpp firmware is not supported in this library'";
        break;
      }
    case -30: 
      { 
        retval = "Err code : -30 -- 'Invalid firmware license'";
        break;
      }
    case -31:
      { 
        retval = "Err code : -31 -- 'The digitizer is found in a corrupted state'";
        break;
      }
    case -32: 
      { 
        retval = "Err code : -32 -- 'The given trace is not supported by the digitizer'";
        break;
      }
    case -33: 
      { 
        retval = "Err code : -33 -- 'The given probe is not supported for the given digitizer trace'";
        break;
      }
    case -34: 
      { 
        retval = "Err code : -34 -- 'The base address is not supported, is it a desktop device?'";
        break;
      }
    case -99: 
      { 
        retval = "Err code : -99 -- 'The function is not yet implemented'";
        break;
      }
    default:
      {
        retval = "UNREGISTERED Err code : " + std::to_string( (int)err) + " UNKNOWN ";
        break;
      }
  }
  return retval;
}

std::ostream& operator<<(std::ostream& os, const CAEN_DGTZ_ErrorCode& err)
{
  os << error_code_to_string(err);
  return os;
//switch((int)err) 
//  {
//    case 0 :
//        { 
//            os << "Err code : 0 -- 'Operation completed sucessfully'" << std::endl;
//            break;
//        }
//    case -1:
//        { 
//            os << "Err code : -1 -- 'Communication error'" << std::endl;
//            break;
//        }
//    case -2: 
//        { 
//            os << "Err code : -2 -- 'Unspecified error'" << std::endl;
//            break;
//        }
//    case -3: 
//        { 
//            os << "Err code : -3 -- 'Invalid parameter'" << std::endl;
//            break;
//        }
//    case -4:
//        { 
//            os << "Err code : -4 -- 'Invalid link type'" << std::endl;
//            break;
//        }
//    case -5: 
//        { 
//            os << "Err code : -5 -- 'Invalid device handle'" << std::endl;
//            break;
//        }
//    case -6: 
//        { 
//            os << "Err code : -6 -- 'Maximum number of devices exceeded'" << std::endl;
//            break;
//        }
//    case -7: 
//        { 
//            os << "Err code : -7 -- 'Operation not allowed on this board'" << std::endl;
//            break;
//        }
//    case -8:
//        { 
//            os << "Err code : -8 -- 'The interrupt level is bad'" << std::endl;
//            break;
//        }
//    case -9: 
//        { 
//            os << "Err code : -9 -- 'The event number is bad'" << std::endl;
//            break;
//        }
//    case -10: 
//        { 
//            os << "Err code : -10 -- 'Unable to read the registry'" << std::endl;
//            break;
//        }
//    case -11:
//        { 
//            os << "Err code : -11 -- 'Unable to write into the registry'" << std::endl;
//            break;
//        }
//    case -13:
//        { 
//            os << "Err code : -13 -- 'The channel number is invalid'" << std::endl;
//            break;
//        }
//    case -14:
//        { 
//            os << "Err code : -14 -- 'The channel is busy'" << std::endl;
//            break;
//        }
//    case -15:
//        { 
//            os << "Err code : -15 -- 'Invalid FPIO mode'" << std::endl;
//            break;
//        }
//    case-16: 
//        { 
//            os << "Err code : -16 -- 'Wrong acquisitiion mode'" << std::endl;
//            break;
//        }
//    case -17: 
//        { 
//            os << "Err code : -17 -- 'The function is not allowed for this module'" << std::endl;
//            break;
//        }
//    case -18: 
//        { 
//            os << "Err code : -18 -- 'Communication Timeout'" << std::endl;
//            break;
//        }
//    case -19: 
//        { 
//            os << "Err code : -19 -- 'The buffer is invalid'" << std::endl;
//            break;
//        }
//    case -20: 
//        { 
//            os << "Err code : -20 -- 'The event is not found'" << std::endl;
//            break;
//        }
//    case -21: 
//        { 
//            os << "Err code : -21 -- 'The event is invalid'" << std::endl;
//            break;
//        }
//    case -22: 
//        { 
//            os << "Err code : -22 -- 'Out of memory'" << std::endl;
//            break;
//        }
//    case -23: 
//        { 
//            os << "Err code : -23 -- 'Unable to calibrate the board'" << std::endl;
//            break;
//        }
//    case -24: 
//        { 
//            os << "Err code : -24 -- 'Unable to open the digitizer'" << std::endl;
//            break;
//        }
//    case -25: 
//        { 
//            os << "Err code : -25 -- 'The digitizer is already open'" << std::endl;
//            break;
//        }
//    case -26:
//        { 
//            os << "Err code : -26 -- 'The digitizer is not ready to operate'" << std::endl;
//            break;
//        }
//    case -27:
//        { 
//            os << "Err code : -27 -- 'The digitizer has not the IRQ configure'" << std::endl;
//            break;
//        }
//    case -28:
//        { 
//            os << "Err code : -28 -- 'The digitizer flash memory is corrupted'" << std::endl;
//            break;
//        }
//    case -29: 
//        { 
//            os << "Err code : -29 -- 'The digitizer dpp firmware is not supported in this library'" << std::endl;
//            break;
//        }
//    case -30: 
//        { 
//            os << "Err code : -30 -- 'Invalid firmware license'" << std::endl;
//            break;
//        }
//    case -31:
//        { 
//            os << "Err code : -31 -- 'The digitizer is found in a corrupted state'" << std::endl;
//            break;
//        }
//    case -32: 
//        { 
//            os << "Err code : -32 -- 'The given trace is not supported by the digitizer'" << std::endl;
//            break;
//        }
//    case -33: 
//        { 
//            os << "Err code : -33 -- 'The given probe is not supported for the given digitizer trace'" << std::endl;
//            break;
//        }
//    case -34: 
//        { 
//            os << "Err code : -34 -- 'The base address is not supported, is it a desktop device?'" << std::endl;
//            break;
//        }
//    case -99: 
//        { 
//            os << "Err code : -99 -- 'The function is not yet implemented'" << std::endl;
//            break;
//        }
//    default:
//        {
//            os << "UNREGISTERED Err code : " << (int)err << " UNKNOWN " <<std::endl;
//            break;
//        }
//  }
    return os;
}

/***************************************************************/

CaenN6725WF::CaenN6725WF()
{
}

/***************************************************************/

CaenN6725WF::CaenN6725WF(DigitizerParams_t pars)
{
}

/***************************************************************/

CaenN6725WF::~CaenN6725WF()
{
  if (is_connected_)
    {
      std::cout << "Closing digitizer..." << std::endl;
      CAEN_DGTZ_SWStopAcquisition(handle_);
      if (this_event_)
        {CAEN_DGTZ_FreeEvent(handle_, (void**)&this_event_);}
      if (allocated_size_ > 0) 
        {CAEN_DGTZ_FreeReadoutBuffer(&buffer_);}
      CAEN_DGTZ_CloseDigitizer(handle_);
    }
}

/***************************************************************/

bool CaenN6725WF::is_active(int channel) const
{
    return (active_channel_bitmask_ & (1<<channel)); 
}

/***************************************************************/

void CaenN6725WF::connect()
{
  // make this specific for our case
  // third 0 is VMEBaseAddress, which must be 0 for direct USB connections
  std::cout << "[INFO] - DACTYLOS" << ".. connecting digitizer instance with waveform recording firmware" << std::endl;
  current_error_ = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB, 0, 0, 0, &handle_);
  if (current_error_ == -1)
    {
      std::cout << "Can not find digitizer at USB bus 0, trying others" << std::endl;
      std::cout << "Trying ... ";
      for (int busnr=1; busnr<128; busnr++)
          {
           std::cout << busnr << "..";
           current_error_ = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB, busnr, 0, 0, &handle_);
           if (current_error_ == 0)  break;
          }
      std::cout << ".. found!" << std::endl;
    }
  if (current_error_ !=0 ) throw std::runtime_error("Can not open digitizer! " + error_code_to_string(current_error_));

  /* Reset the digitizer */
  current_error_ = CAEN_DGTZ_Reset(handle_);
  if (current_error_ !=0 ) throw std::runtime_error("Can not reset digitizer! " + error_code_to_string(current_error_));

  // The board configuration register
  // (trigger overlapping settings (bit 1)
  // (test pattern generator (bit 3)
  current_error_ = CAEN_DGTZ_WriteRegister(handle_, 0x8000, 2);
  if (current_error_ !=0 ) throw std::runtime_error("Writing the board configuration register failed! " + error_code_to_string(current_error_));

  // Set the digitizer acquisition mode (CAEN_DGTZ_SW_CONTROLLED or CAEN_DGTZ_S_IN_CONTROLLED)
  // That this software works as intended, that currently has to be software controlled
  current_error_ = CAEN_DGTZ_SetAcquisitionMode(handle_, CAEN_DGTZ_SW_CONTROLLED);
  if (current_error_ !=0 ) throw std::runtime_error("Can not set acquisition mode to SW_CONTROLLED! "
                                                     + error_code_to_string(current_error_));

  //see CAENDigitizer user manual, chapter "Trigger configuration" for details */
  current_error_ = CAEN_DGTZ_SetExtTriggerInputMode(handle_, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
  if (current_error_ !=0 ) throw std::runtime_error("Can not set trigger input mode ACQ_ONLY! "
                                                     + error_code_to_string(current_error_));

  /* Set the mode used to syncronize the acquisition between different boards.
  In this example the sync is disabled */
  current_error_ = CAEN_DGTZ_SetRunSynchronizationMode(handle_, CAEN_DGTZ_RUN_SYNC_Disabled);
  if (current_error_ !=0 ) throw std::runtime_error("Can not set run synchronisation mode to Disabled! " 
                                                     + error_code_to_string(current_error_));

  is_connected_ = true;
}


void CaenN6725WF::configure(DigitizerParams_t params)
{
  current_error_ = CAEN_DGTZ_SetRecordLength(handle_, params.RecordLength);
  if (current_error_ !=0 ) throw std::runtime_error("Can not set record length! "
                                                     + error_code_to_string(current_error_));
  current_error_ = CAEN_DGTZ_SetRecordLength(handle_, params.RecordLength,2);
  if (current_error_ !=0 ) throw std::runtime_error("Can not set record length for ch 2,3 "
                                                     + error_code_to_string(current_error_));
  current_error_ = CAEN_DGTZ_SetRecordLength(handle_, params.RecordLength,4);
  if (current_error_ !=0 ) throw std::runtime_error("Can not set record length for ch 4,5 "
                                                     + error_code_to_string(current_error_));
  current_error_ = CAEN_DGTZ_SetRecordLength(handle_, params.RecordLength,6);
  if (current_error_ !=0 ) throw std::runtime_error("Can not set record length for ch 6,7 "
                                                     + error_code_to_string(current_error_));

  //FIXME set reasonable number
  CAEN_DGTZ_SetMaxNumEventsBLT(handle_, 2);

  uint32_t posttrigs;
  // Post trigger size (that is the possiton of the trigger within the event
  current_error_ = CAEN_DGTZ_GetPostTriggerSize(handle_, &posttrigs);
  if (current_error_ !=0 ) throw std::runtime_error("Getting the post trigger size failed! " + error_code_to_string(current_error_));
  std::cout << "Found post trigger size of : " << std::to_string(posttrigs) << std::endl;

  std::cout << "Will set post trigger size to : " << params.PostTriggerPercent << std::endl;
  current_error_ = CAEN_DGTZ_SetPostTriggerSize(handle_, params.PostTriggerPercent);
  if (current_error_ !=0 ) throw std::runtime_error("Setting the post trigger size failed! " + error_code_to_string(current_error_));


  // also set the record length internally
  recordlength_ = params.RecordLength;

  // Set the I/O level (CAEN_DGTZ_IOLevel_NIM or CAEN_DGTZ_IOLevel_TTL)
  current_error_ = CAEN_DGTZ_SetIOLevel(handle_, params.IOlev);
  if (current_error_ !=0 ) throw std::runtime_error("Can not set IO level "
                                                     + error_code_to_string(current_error_));

  // Set the enabled channels
  // remember the active cannels
  active_channel_bitmask_  = params.ChannelMask;
  std::cout << "Setting channel mask " << params.ChannelMask << std::endl;
  current_error_ = CAEN_DGTZ_SetChannelEnableMask(handle_, params.ChannelMask);
  if (current_error_ !=0 ) throw std::runtime_error("Can not set channel enable mask! "
                                                     + error_code_to_string(current_error_));

  // self trigger mode for channels
  CAEN_DGTZ_SetChannelSelfTrigger(handle_, CAEN_DGTZ_TRGMODE_ACQ_ONLY, params.ChannelMask);
  //CAEN_DGTZ_SetChannelSelfTrigger(handle_, CAEN_DGTZ_TRGMODE_DISABLED, params.ChannelMask);
  if (current_error_ !=0 ) throw std::runtime_error("Can not set channel self trigger! "
                                                     + error_code_to_string(current_error_));

  CAEN_DGTZ_SetExtTriggerInputMode(handle_, CAEN_DGTZ_TRGMODE_DISABLED);
  if (current_error_ !=0 ) throw std::runtime_error("Can not disable external trigger mode!"
                                                     + error_code_to_string(current_error_));

  for(unsigned int i=0; i<max_n_channels_; i++) {
      if (is_active(i)) {
          // Set the polarity for the given channel (CAEN_DGTZ_PulsePolarityPositive or CAEN_DGTZ_PulsePolarityNegative)
          current_error_ = CAEN_DGTZ_SetChannelPulsePolarity(handle_, i, params.PulsePolarity);
          if (current_error_ !=0 ) throw std::runtime_error("Can not set pulse polarity! "
                                                             + error_code_to_string(current_error_));
      }
  }
//CAEN_DGTZ_SetTriggerPolarity (handle_, 0, CAEN_DGTZ_TriggerOnRisingEdge);

}

/***************************************************************/

void CaenN6725WF::set_handle(int handle)
{
  handle_ = handle;
}

/***************************************************************/

int CaenN6725WF::get_handle() const
{
  return handle_;
}

/***************************************************************/

/*! \fn      static long get_time()
*   \brief   Get time in milliseconds
*   \return  time in msec */
// stolen from CAEN examples
long CaenN6725WF::get_time() const
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

/**
 * Set a DC offset to the input signal to adapt it to digitizer's dynamic range
 * from the manual:
 * This function sets the 16-bit DAC that adds a DC offset to the input signal to adapt
 * it to the dynamic range of the ADC.
 * By default, the DAC is set to middle scale (0x7FFF) which corresponds to a DC offset
 * of -Vpp/2, where Vpp is the voltage
 * range (peak to peak) of the ADC. This means that the input signal
 * can range from -Vpp/2 to +Vpp/2. If the DAC is set to
 * 0x0000, then no DC offset is added, and the range of the input signal
 * goes from -Vpp to 0. Conversely, when the DAC is
 * set to 0xFFFF, the DC offset is –Vpp and the range goes from 0 to +Vpp.
 * The DC offset can be set on channel basis except
 * for the x740 in which it is set on group basis;
 * in this case, you must use the Set / GetGroupDCOffset functions.
 *
 * @param: offset - channel dc offset in 0-32767 (0 - 100%)
 */
void CaenN6725WF::set_channel_dc_offset(int channel, int offset)
{
  // FIXME: make sure the range of the dc offset is fine 
  //        and make it more clear that this is NOT in percent, but is 15 (!) bits
  //        (digitizer is 14 bit
  current_error_ = CAEN_DGTZ_SetChannelDCOffset(handle_, channel, offset);
  if (current_error_ !=0 ) throw std::runtime_error("Can not set channel dc offset! "
                                                     + error_code_to_string(current_error_));
}

/***************************************************************/

uint32_t CaenN6725WF::get_channel_dc_offset(int channel)
{
  uint32_t offset;
  current_error_ = CAEN_DGTZ_GetChannelDCOffset(handle_, channel, &offset);
  if (current_error_ != 0) throw std::runtime_error("Can not get baseline offset for ch "
                                                    + std::to_string(channel)
                                                    + " ! " + error_code_to_string(current_error_));
  return offset;
}

/***************************************************************/

void CaenN6725WF::set_rootfilename(std::string fname)
{
    rootfile_name_ = fname;
}

/***************************************************************/

void CaenN6725WF::set_channel_trigger_threshold(int channel, int threshold)
{
  if ((threshold < 0) || (threshold > 16383))
      {throw std::runtime_error("Trigger threshold has to be in the interval [0,16383]");};
  current_error_ = CAEN_DGTZ_SetChannelTriggerThreshold(handle_, channel, threshold);
  // FIXME - hoepfully this might be obsolete
  //switch (channel)
  //  {
  //    case 0: current_error_ = CAEN_DGTZ_WriteRegister(handle_, 0x1080, threshold); break;
  //    case 1: current_error_ = CAEN_DGTZ_WriteRegister(handle_, 0x1180, threshold); break;
  //    case 2: current_error_ = CAEN_DGTZ_WriteRegister(handle_, 0x1280, threshold); break;
  //    case 3: current_error_ = CAEN_DGTZ_WriteRegister(handle_, 0x1380, threshold); break;
  //    case 4: current_error_ = CAEN_DGTZ_WriteRegister(handle_, 0x1480, threshold); break;
  //    case 5: current_error_ = CAEN_DGTZ_WriteRegister(handle_, 0x1580, threshold); break;
  //    case 6: current_error_ = CAEN_DGTZ_WriteRegister(handle_, 0x1680, threshold); break;
  //    case 7: current_error_ = CAEN_DGTZ_WriteRegister(handle_, 0x1780, threshold); break;
  //    default: throw std::runtime_error("Can not identify channel " + std::to_string(channel));
  //  }
  //if (current_error_ != 0) throw std::runtime_error("Can not set trigger threshold for ch "
  //                               + std::to_string(channel)
  //                               + " ! "
  //                               + error_code_to_string(current_error_));
}

/***************************************************************/

CAEN_DGTZ_ErrorCode CaenN6725WF::get_last_error() const
{
  return current_error_;
;;}

/***************************************************************/

CAEN_DGTZ_BoardInfo_t CaenN6725WF::get_board_info()
{
  current_error_ = CAEN_DGTZ_GetInfo(handle_, &board_info_);
  if (current_error_ != 0) throw std::runtime_error("Error while getting board info! "
                                                     + error_code_to_string(current_error_));
  return board_info_;
}

/***************************************************************/

float CaenN6725WF::get_expected_baseline(int channel)
{
  uint32_t offset = get_channel_dc_offset(channel);
  float dco_percent = (float)offset / 65535.;
  float expected_baseline = pow(2, 14.0) * (1.0 - dco_percent);
  return expected_baseline;
}



/***************************************************************/

void CaenN6725WF::allocate_memory()
{
  current_error_ = CAEN_DGTZ_AllocateEvent(handle_,
                                (void**)&this_event_);
  if (current_error_ != 0) throw std::runtime_error("Error while allocating event! " + error_code_to_string(current_error_));
  current_error_ = CAEN_DGTZ_MallocReadoutBuffer(handle_, &buffer_, &allocated_size_);
  if (current_error_ != 0) throw std::runtime_error("Error while allocating readout buffer! " + error_code_to_string(current_error_));
  //std::cout << "Allocated .. " << allocated_size_ << std::endl;
}

/***************************************************************/

uint32_t CaenN6725WF::get_allocated_buffer_size()
{
  return allocated_size_;
}

/***************************************************************/

void CaenN6725WF::prepare_rootfile()
{
  int nchan = get_nchannels();

  // create a new root file
  root_file_   = new TFile(rootfile_name_.c_str(), "RECREATE");
  if (!root_file_) throw std::runtime_error("Problems creating/overwriting root file "
                                            + rootfile_name_);
  waveform_ch_.clear();
  waveform_ch_.reserve(nchan);
  channel_trees_.clear();
  channel_trees_.reserve(nchan);
  std::string ch_name = "ch";
  for (int k=0; k<get_nchannels(); k++)
      {waveform_ch_.push_back({});
       ch_name = "ch" + std::to_string(k);
       channel_trees_.push_back(new TTree(ch_name.c_str(), ch_name.c_str()));}
  for (int k=0;k<nchan;k++)
    {
      channel_trees_[k]->Branch("waveform",  &waveform_ch_[k]);
    } 
}

/***************************************************************/

void CaenN6725WF::start_acquisition()
{
  std::cout << "Preparing to start acquisition...";
  if (rootfile_name_ != "") {
    prepare_rootfile();
  } else {
    std::cout << ".. [WARN] : no rootfilename set, will not write to file ..";
  }
  n_events_acq_  = std::vector<long>(get_nchannels(), 0);
  current_error_ = CAEN_DGTZ_SWStartAcquisition(handle_);
  std::cout << "...started!" << std::endl;
}

/***************************************************************/

void CaenN6725WF::end_acquisition()
{
  CAEN_DGTZ_SWStopAcquisition(handle_);
  if (root_file_) {
    root_file_->Close();
  }
}

/***************************************************************/

int CaenN6725WF::get_nchannels() const
{
  return max_n_channels_;
}

/***************************************************************/

std::vector<int> CaenN6725WF::get_temperatures() const
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

/***************************************************************/

void CaenN6725WF::calibrate()
{
  current_error_ = CAENDGTZ_API CAEN_DGTZ_Calibrate(handle_);
  if (current_error_ != 0) throw std::runtime_error("Problem during calibration! "
                                                    + error_code_to_string(current_error_));
}

/***************************************************************/

void CaenN6725WF::set_input_dynamic_range(DynamicRange range)
{
  current_error_ = CAEN_DGTZ_WriteRegister(handle_,0x8028, (uint32_t) range);
  if (current_error_ != 0) throw std::runtime_error("Problems setting dynamic range! "
                                                     + error_code_to_string(current_error_));
}

/***************************************************************/

std::vector<uint32_t> CaenN6725WF::get_input_dynamic_range()
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

/***************************************************************/

std::vector<long> CaenN6725WF::get_n_events_tot()
{
    return n_events_acq_;
}

/***************************************************************/

void CaenN6725WF::reset_memory()
{
  //if current_error_ == CAEN_DGTZ_ErrorCode::CAEN_DGTZ_ErrorCode;
  if (current_error_ == -22)
    {
        // reset all memory
        //if (buffer_)
        // {
             //CAEN_DGTZ_ClearData(handle_);
             CAEN_DGTZ_FreeReadoutBuffer(&buffer_);
             allocate_memory();
        // }
    }
}

/***************************************************************/

void CaenN6725WF::readout_routine(bool write_root)
{
  //CAEN_DGTZ_SendSWtrigger(handle_);
  // check the readout status
  uint32_t acqstatus;
  std::vector<uint16_t> this_wf = {};
  this_wf.reserve(recordlength_);

  waveform_ch_.clear();
  for (int k=0;k<8;k++)
    {waveform_ch_.push_back({});}

  // FIXME acquisition status not working
  //current_error_ = CAEN_DGTZ_ReadRegister(handle_, 0x8104, &acqstatus);
  //if (! ( acqstatus & (1 << 3))) // the 3rd bit is the acquisition status
  //  {
  //      std::cout << "nothing acquired" << std::endl;
  //      return; // nothing to readout

  //  }
  //// wait till the buffer is full
  //if (! ( acqstatus & (1 << 4))) // the 3rd bit is the acquisition status
  //  {
  //      std::cout << "buffer not full yet" << std::endl;
  //      return; // no channel in full status
  //  }
  for (int k = 0; k<get_nchannels(); k++)
    {num_events_[k] = 0;}

  //std::cout << "Attempting to read data" << std::endl;
  current_error_ = CAEN_DGTZ_ReadData(handle_, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,
  //current_error_ = CAEN_DGTZ_ReadData(handle_, CAEN_DGTZ_POLLING_MBLT,
                                      buffer_, &buffer_size_);
  if (current_error_ != 0) 
    {
        // just inform the user, nothing dramatic if it only happens once
        //std::cout << "error while reading data " << current_error_ << std::endl;
        //CAEN_DGTZ_FreeReadoutBuffer(&buffer_);
        // ReadData can cause a -22 Memory error, and in consecutive sometimes 
        // a seqfault. The only explanation I have here is that it corrupts the
        // buffer pointer, in which we simply reset it. 
        // This will probably lead to a memory leak. Right now, we only focus on 
        // getting things going, so there is definitly room for improvement
        reset_memory();
        return;
    }
  if (buffer_size_ == 0)
    {
        //std::cout << "buffer empty" << std::endl;
        //CAEN_DGTZ_FreeReadoutBuffer(&buffer_);
        return;
    }
  uint32_t events_in_buffer = 0;
  //std::cout << "Attempting to get number of events" << std::endl;
  current_error_ =  CAEN_DGTZ_GetNumEvents(handle_,
                                           buffer_,
                                           buffer_size_,
                                           &events_in_buffer);
  //std::cout << "We found " << events_in_buffer << " events" << std::endl;

  if (current_error_ != 0)
    {
        // also not too dramatic yet, just inform the user
        //std::cout << "error while getting the number of events! " << current_error_ << std::endl;
        //CAEN_DGTZ_FreeReadoutBuffer(&buffer_);

        return;
    }

  //uint32_t channelmask = 0;
  int channel = 0;
  for (uint ev=0; ev<events_in_buffer; ev++)
    {
//      //std::cout << "Attempting to get event info" << std::endl;
      current_error_ = CAEN_DGTZ_GetEventInfo(handle_,
                                              buffer_,
                                              buffer_size_,
                                              ev,
                                              &event_info_,
                                              &evt_bytestream_);

      //channelmask =  event_info_.ChannelMask;
      //std::cout << "Attempting to decode event" << std::endl;
      if (!(evt_bytestream_)) continue;
      if (current_error_ != 0)
        {
            //std::cout << "error while getting event info! " << current_error_ << std::endl;
            continue;
        }
      current_error_ = CAEN_DGTZ_DecodeEvent(handle_,
                                             evt_bytestream_,
                                             (void**)&this_event_);

      // std::cout << "channel " << channel << " for mask " << channelmask << " "  << std::endl; 

      uint32_t channel_size;
      for (unsigned int ch=0; ch<get_nchannels(); ch++)
        {
          // check if the cannel has seen data
          if (!(this_event_)) continue;
          if (!(is_active(ch))) continue;
          channel_size = this_event_->ChSize[ch];
          std::cout << "Found channel size of " << channel_size << " for channel " << ch << std::endl;
          if (channel_size == 0) continue;

          //std::cout << "Trying to access event" << std::endl;
          this_wf = std::vector<uint16_t>(this_event_->DataChannel[ch],this_event_->DataChannel[ch] + this_event_->ChSize[ch]);
          waveform_ch_.at(ch) = this_wf;
          if (write_root)
            {
              channel_trees_[ch]->Fill(); 
              channel_trees_[ch]->Write();
            }
          n_events_acq_[ch] += 1;
        }
        if (this_event_ != nullptr)
          {
            current_error_ = CAEN_DGTZ_FreeEvent(handle_,
                                (void**)&this_event_);
          }
       //n_events_acq_[ch] += num_events_[ch];
     }
     //CAEN_DGTZ_FreeReadoutBuffer(&buffer_);
  // clear data for the next cycle
  current_error_ = CAEN_DGTZ_ClearData(handle_);

}

/***************************************************************/

void CaenN6725WF::readout_and_save(unsigned int seconds)
{

  // timing variables
  long now_time = get_time()/1000;
  long last_time = now_time;
  long delta_t = 0;
  std::cout << "Starting readout.." << std::endl;
  if (root_file_) root_file_->cd();
  while (delta_t < seconds)
    {
      readout_routine(true);
      now_time = get_time()/1000;
      delta_t +=  now_time  - last_time;
        
      //std::cout << ".";
      last_time = now_time;
    } // end while time loop    
  std::cout << "done!" << std::endl;
  root_file_->Write();
  root_file_->Close();
  return;
}

/***************************************************************/

std::vector<std::vector<uint16_t>> CaenN6725WF::readout_and_return()
{
  readout_routine(false);
  return waveform_ch_;
}

/***************************************************************/

//----------------------------------------------------------
// In the following, these are methods for the digitizer
// with the DPP-PHA firmware installed
//
//
//---------------------------------------------------------

/***************************************************************/

CaenN6725DPPPHA::CaenN6725DPPPHA()
{
  rootfile_name_ = "";
};

/***************************************************************/

CaenN6725DPPPHA::CaenN6725DPPPHA(DigitizerParams_t params) : CaenN6725DPPPHA()
{
    std::cout << "[INFO] - DACTYLOS" << ".. connecting digitizer instance with DPP-PHA firmware" << std::endl;
    rootfile_name_ = "";
    connect();
    configure(params);
};

/***************************************************************/

int CaenN6725DPPPHA::get_handle()
{
    return handle_;
}

/***************************************************************/

void CaenN6725DPPPHA::set_handle(int handle)
{
    handle_ = handle;
}

/***************************************************************/

void CaenN6725DPPPHA::connect()
{
    // make this specific for our case
    // third 0 is VMEBaseAddress, which must be 0 for direct USB connections
    current_error_ = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB, 0, 0, 0, &handle_);
    if (current_error_ == -1)
        {
            std::cout << "Can not find digitizer at USB bus 0, trying others" << std::endl;
            std::cout << "Trying ... ";
            for (int busnr=1; busnr<128; busnr++)
                {
                 std::cout << busnr << "..";
                 current_error_ = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB, busnr, 0, 0, &handle_);
                 if (current_error_ == 0)  break;
                }
            std::cout << ".. found!" << std::endl;
        }
    if (current_error_ !=0 ) throw std::runtime_error("Can not open digitizer err code: " + std::to_string(current_error_));

    /* Reset the digitizer */
    current_error_ = CAEN_DGTZ_Reset(handle_);
    if (current_error_ !=0 ) throw std::runtime_error("Can not reset digitizer err code:" + std::to_string(current_error_));

    // Register 0x8000 is "board configuration"
    // trigger overlap mainly - what to do if triggers overlap?
    // disabled for now 
    //
    //current_error_ = CAEN_DGTZ_WriteRegister(handle_, 0x8000, 0x01000114);  // Channel Control Reg (indiv trg, seq readout) ??
    //if (current_error_ !=0 ) throw std::runtime_error("Can not write register err code:" + std::to_string(current_error_));

    // by default, we just want to acquire the energy, that is all
    // technically, this can be configured with the parameters, however
    // that might be confusing.
    current_error_ = CAEN_DGTZ_SetDPPAcquisitionMode(handle_, CAEN_DGTZ_DPP_ACQ_MODE_Mixed, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
    //current_error_ = CAEN_DGTZ_SetDPPAcquisitionMode(handle_, CAEN_DGTZ_DPP_ACQ_MODE_Oscilloscope, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
    //current_error_ = CAEN_DGTZ_SetDPPAcquisitionMode(handle_, CAEN_DGTZ_DPP_ACQ_MODE_List, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP acquisition mode err code:" + std::to_string(current_error_));
    
    // Set the digitizer acquisition mode (CAEN_DGTZ_SW_CONTROLLED or CAEN_DGTZ_S_IN_CONTROLLED)
    // That this software works as intended, that currently has to be software controlled
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
    //set_virtualprobe1(DPPVirtualProbe1::Input);
    //set_virtualprobe1(CAEN_DGTZ_DPP_VIRTUALPROBE_Input);
    //current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, ANALOG_TRACE_1, CAEN_DGTZ_DPP_VIRTUALPROBE_Input);
    //if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP virtual probe trace 1 err ode: " + std::to_string(current_error_));

    // set the second one the the trapezoid
    //set_virtualprobe2(DPPVirtualProbe2::TrapezoidReduced);
    //current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, ANALOG_TRACE_2, CAEN_DGTZ_DPP_VIRTUALPROBE_TrapezoidReduced);
    //if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP virtual probe trace 2 err ode: " + std::to_string(current_error_));

    // set the digitial trace to the peaking time, the other digital trace will always be 
    // the trigger
    //set_digitalprobe1(DPPDigitalProbe1::Peaking);
    //current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, DIGITAL_TRACE_1, CAEN_DGTZ_DPP_DIGITALPROBE_Peaking);
    //if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP virtual probe trace 2 err ode: " + std::to_string(current_error_));
    is_connected_ = true;
}

/***************************************************************/

void CaenN6725DPPPHA::configure(DigitizerParams_t params)
{
    std::cout << "Setting record length for DPP-PHA " << params.RecordLength << std::endl;
    // Set the number of samples for each waveform
    // Not sure, but this might be needed to be done per each channel pair
    //current_error_ = CAEN_DGTZ_SetRecordLength(handle_, (uint32_t)params.RecordLength);
    //if (current_error_ !=0 ) throw std::runtime_error("Can not set record length err code: " + error_code_to_string(current_error_));
    current_error_ = CAEN_DGTZ_SetRecordLength(handle_, (uint32_t)params.RecordLength,0);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set record length for channel pair 0 err code: " + error_code_to_string(current_error_));
    current_error_ = CAEN_DGTZ_SetRecordLength(handle_, (uint32_t)params.RecordLength,2);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set record length for channel pair 2 err code: " + error_code_to_string(current_error_));
    current_error_ = CAEN_DGTZ_SetRecordLength(handle_, params.RecordLength,4);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set record length or channel pair 4 err code: " + std::to_string(current_error_));
    current_error_ = CAEN_DGTZ_SetRecordLength(handle_, params.RecordLength,6);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set record length for channel pair 6 err code: " + std::to_string(current_error_));


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
            if (current_error_ !=0 ) throw std::runtime_error("Can not set dpp pre-trigger sixe err code:" + std::to_string(current_error_));

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
    event_aggregate_ = params.EventAggr;
    current_error_ = CAEN_DGTZ_SetDPPEventAggregation(handle_, params.EventAggr, 0);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set dpp event agregation err code:" + std::to_string(current_error_));
}

/***************************************************************/

void CaenN6725DPPPHA::show_supported_probes()
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

void CaenN6725DPPPHA::set_virtualprobe1(DPPVirtualProbe1 vprobe1)
{
    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, ANALOG_TRACE_1, (int)vprobe1);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP virtual probe for trace 1 err ode: " + std::to_string(current_error_));
}

/***************************************************************/

void CaenN6725DPPPHA::set_virtualprobe2(DPPVirtualProbe2 vprobe2)
{
    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, ANALOG_TRACE_2, (int)vprobe2);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP virtual probe for trace 2 err ode: " + std::to_string(current_error_));
}

/***************************************************************/

void CaenN6725DPPPHA::set_digitalprobe1(DPPDigitalProbe1 dprobe1)
{
    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, DIGITAL_TRACE_1, (int)dprobe1);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP digital probe for dtrace 1 err ode: " + std::to_string(current_error_));
}
/***************************************************************/

void CaenN6725DPPPHA::set_digitalprobe2(DPPDigitalProbe2 dprobe2)
{
    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, DIGITAL_TRACE_2, (int)dprobe2);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP ditial probe for dtrace 2 err ode: " + std::to_string(current_error_));
}

/***************************************************************/

CAEN_DGTZ_ErrorCode CaenN6725DPPPHA::get_last_error() const
{
    return current_error_;
}

/***************************************************************/

inline void CaenN6725DPPPHA::fill_analog_trace1_()
{
    atrace1_ = waveform_->Trace1;
    analog_trace1_ = std::vector<int16_t>(atrace1_, atrace1_ + trace_ns_);
}

/***************************************************************/

inline void CaenN6725DPPPHA::fill_analog_trace2_()
{
    atrace2_ = waveform_->Trace2;
    analog_trace2_ = std::vector<int16_t>(atrace2_, atrace2_ + trace_ns_);
}

/***************************************************************/

inline void CaenN6725DPPPHA::fill_digital_trace1_()
{
    dtrace1_ = waveform_->DTrace1;
    digital_trace1_ = std::vector<uint8_t>(dtrace1_, dtrace1_ + trace_ns_);
}

/***************************************************************/

inline void CaenN6725DPPPHA::fill_digital_trace2_()
{
    dtrace2_ = waveform_->DTrace2;
    digital_trace2_ = std::vector<uint8_t>(dtrace2_, dtrace2_ + trace_ns_);
}

/***************************************************************/

int CaenN6725DPPPHA::get_trigger_point()
{
    for (int k=0; k<digital_trace2_.size(); k++)
        {   //std::cout << digital_trace2_[k] << std::endl;
            if (digital_trace2_[k] > 0) return k;
        }
    return digital_trace2_.size();
}

/***************************************************************/

CAEN_DGTZ_BoardInfo_t CaenN6725DPPPHA::get_board_info()
{
    current_error_ = CAEN_DGTZ_GetInfo(handle_, &board_info_);
    if (current_error_ != 0) throw std::runtime_error("Error while getting board infoe, err code " + std::to_string(current_error_));
    return board_info_;
}

/***************************************************************/

std::vector<int16_t> CaenN6725DPPPHA::get_analog_trace1()
{
    return analog_trace1_;
}
/***************************************************************/

std::vector<int16_t> CaenN6725DPPPHA::get_analog_trace2()
{
    return analog_trace2_;
}

/***************************************************************/

std::vector<uint8_t> CaenN6725DPPPHA::get_digital_trace1()
{
    return digital_trace1_;
}

/***************************************************************/

std::vector<uint8_t> CaenN6725DPPPHA::get_digital_trace2()
{
    return digital_trace2_;
}

/***************************************************************/

uint16_t CaenN6725DPPPHA::get_energy()
{
    return energy_;
}

/***************************************************************/

void CaenN6725DPPPHA::set_channel_dc_offset(int channel, int offset)
{
    // Set a DC offset to the input signal to adapt it to digitizer's dynamic range
    // from the manual:
    // This function sets the 16-bit DAC that adds a DC offset to the input signal to adapt it to the dynamic range of the ADC.
    // By default, the DAC is set to middle scale (0x7FFF) which corresponds to a DC offset of -Vpp/2, where Vpp is the voltage
    // range (peak to peak) of the ADC. This means that the input signal can range from -Vpp/2 to +Vpp/2. If the DAC is set to
    // 0x0000, then no DC offset is added, and the range of the input signal goes from -Vpp to 0. Conversely, when the DAC is
    // set to 0xFFFF, the DC offset is –Vpp and the range goes from 0 to +Vpp. The DC offset can be set on channel basis except
    // for the x740 in which it is set on group basis; in this case, you must use the Set / GetGroupDCOffset functions.
    //current_error_ = CAEN_DGTZ_SetChannelDCOffset(handle_, channel, 0x8000);
    current_error_ = CAEN_DGTZ_SetChannelDCOffset(handle_, channel, offset);
    if (current_error_ !=0 ) throw std::runtime_error("Can not set channel dc offset err code:" + std::to_string(current_error_));
}



/***************************************************************/

void CaenN6725DPPPHA::allocate_memory()
{
    // This is for Mengjiao's test of the trapezoidal shaper!
    // The post trigger size is in percent.
    //uint32_t posttrigs;
    //current_error_ = CAEN_DGTZ_GetPostTriggerSize(handle_, &posttrigs);
    //if (current_error_ !=0 ) throw std::runtime_error("Getting the post trigger size failed! err code:" + std::to_string(current_error_));
    //std::cout << "Post trigger size has been found yo be " << std::to_string(posttrigs);

    //current_error_ = CAEN_DGTZ_SetPostTriggerSize(handle_, posttrigs);
    //if (current_error_ !=0 ) throw std::runtime_error("Setting the post trigger size failed! err code:" + std::to_string(current_error_));
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

uint32_t CaenN6725DPPPHA::get_allocated_buffer_size()
{
    return allocated_size_;
}

/***************************************************************/

std::vector<std::vector<CAEN_DGTZ_DPP_PHA_Event_t>> CaenN6725DPPPHA::read_data(bool fill_histogram)
{
    // check the readout status
    uint32_t acqstatus;
    // fixme: maybe 0xEF04 is better since it is dpp_pha? (event_ready)
    current_error_ = CAEN_DGTZ_ReadRegister(handle_, 0x8104, &acqstatus);
    while (! ( acqstatus & (1 << 3))) // the 3rd bit is the acquisition status
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
            std::cout << "error while reading data" << current_error_ << std::endl;
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
            std::cout << "error while getting DPP events" << current_error_ << std::endl;
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
                    if (fill_histogram)
                        {
                            if (energy_ > 16384) 
                                {
                                    fail_events_[ch] += 1;
                                } else {
                                    energy_histogram_[ch][energy_] += 1; 
                                }
                                
                        }
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

            if (root_file_)
                {
                    channel_trees_[ch]->Write();
                    n_events_acq_[ch] += num_events_[ch]; 
                }
            thisevents.push_back(channel_events);
        }
    //CAEN_DGTZ_DPP_PHA_Event_t (*thisevents)[]
    return thisevents;
}

/***************************************************************/

std::vector<int16_t> CaenN6725DPPPHA::get_last_waveform(int channel)
{
    return waveform_ch_[channel];
}


/***************************************************************/

std::vector<int> CaenN6725DPPPHA::get_n_events()
{
    std::vector<int> n_events({});
    for (uint ch=0; ch<get_nchannels(); ch++)
        {
            n_events.push_back(num_events_[ch]);
        }
    return n_events;
}

/***************************************************************/

std::vector<long> CaenN6725DPPPHA::get_n_events_tot()
{
    return n_events_acq_;
}


/***************************************************************/

std::vector<long> CaenN6725DPPPHA::get_n_triggers_tot()
{
    return channel_triggers_;
}

/***************************************************************/

std::vector<long> CaenN6725DPPPHA::get_n_lost_triggers_tot()
{
    return channel_lost_triggers_;
}

/***************************************************************/

void CaenN6725DPPPHA::fast_readout_()
{
    // check the readout status
    uint32_t acqstatus;
    
    current_error_ = CAEN_DGTZ_ReadRegister(handle_, 0x8104, &acqstatus);
    if (! ( acqstatus & (1 << 3))) // the 3rd bit is the acquisition status
        {
            return; // nothing to readout
        }
    // wait till the buffer is full
    if (! ( acqstatus & (1 << 4))) // the 3rd bit is the acquisition status
        {
            return; // no channel in full status
        }
    for (int k = 0; k<get_nchannels(); k++)
        {num_events_[k] = 0;}

    current_error_ = CAEN_DGTZ_ReadData(handle_, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer_, &buffer_size_);
    if (current_error_ != 0) 
        {
            std::cout << "error while reading data" << current_error_ << std::endl;
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
            std::cout << "error while getting DPP data" << current_error_ << std::endl;
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
        saturated_ch_.clear();
        saturated_ch_.reserve(get_nchannels());

        for (int k=0; k<get_nchannels(); k++)
            {waveform_ch_.push_back({});
             trigger_ch_.push_back(-1);
             saturated_ch_.push_back(0);}
      }
    for (int ch=0;ch<get_nchannels();ch++)
      {
        if (!(is_active(ch))) continue;
        for (int ev=0;ev<num_events_[ch];ev++)
          {
            // this is ok, because the energy gets then
            // written to the root file imediatly
            energy_ch_[ch] = events_[ch][ev].Energy;
            if (events_[ch][ev].Extras & (1<<4))
                    {saturated_ch_[ch] = 1;}
            //energy_        = events_[ch][ev].Energy;
            if (decode_waveforms_)
              {
                  CAEN_DGTZ_DecodeDPPWaveforms(handle_, &events_[ch][ev], waveform_);
                  // fast mode, only do trace1
                  trace_ns_ = waveform_->Ns;
                  // set the saturated flag if saturated
                  fill_analog_trace1_();
                  fill_digital_trace2_();
                  trigger_ch_.at(ch)  = get_trigger_point(); 
                  //std::cout << get_trigger_point() << std::endl;
                  waveform_ch_.at(ch) = get_analog_trace1();
                  channel_trees_[ch]->Fill();
              }
            else 
              {
                  channel_trees_[ch]->Fill();
              }
          }
        //channel_trees_[ch]->Fill();
        channel_trees_[ch]->Write();
        n_events_acq_[ch] += num_events_[ch];
      }
    return;
}

/***************************************************************/

void CaenN6725DPPPHA::end_acquisition()
{
    CAEN_DGTZ_SWStopAcquisition(handle_);
    if (root_file_) {
      root_file_->Close();
    }
}

/***************************************************************/

int CaenN6725DPPPHA::get_nchannels() const
{
    return max_n_channels_;
}

/***************************************************************/

bool CaenN6725DPPPHA::is_active(int channel) const
{
    return (active_channel_bitmask_ & (1<<channel)); 
}

/***************************************************************/

std::vector<int> CaenN6725DPPPHA::get_temperatures() const
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
long CaenN6725DPPPHA::get_time() const
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

void CaenN6725DPPPHA::enable_waveform_decoding()
{
  decode_waveforms_ = true;
  // set a number of settings
  current_error_ = CAEN_DGTZ_SetDPPAcquisitionMode(handle_, CAEN_DGTZ_DPP_ACQ_MODE_Mixed, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);    
  if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP acquisition mode err code:" + std::to_string(current_error_));
  // this has to be set after acquisition mode
  current_error_ = CAEN_DGTZ_SetDPPEventAggregation(handle_, event_aggregate_, 0);
  if (current_error_ !=0 ) throw std::runtime_error("Can not set dpp event agregation err code:" + std::to_string(current_error_));
  // set the virtual probes (what waveforms get stored)
  set_virtualprobe1(DPPVirtualProbe1::Input);
  set_virtualprobe2(DPPVirtualProbe2::TrapezoidReduced);
  set_digitalprobe1(DPPDigitalProbe1::Peaking);

}

/*******************************************************************/

void CaenN6725DPPPHA::set_input_dynamic_range(DynamicRange range)
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

std::vector<uint32_t>CaenN6725DPPPHA::get_input_dynamic_range()
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

void CaenN6725DPPPHA::configure_channel(unsigned int channel,CAEN_DGTZ_DPP_PHA_Params_t* params)
{
    // channel mask 0xff means all channels ( 8bit set)
    if (channel > 7) throw std::runtime_error("Channel has to be < 8");
    unsigned int channelmask = pow(2, channel);
    current_error_ = CAEN_DGTZ_SetDPPParameters(handle_, channelmask, params);
    if (current_error_ != 0) throw std::runtime_error("Problems configuring channel, err code " + std::to_string(current_error_));

};


/*******************************************************************/

void CaenN6725DPPPHA::start_acquisition()
{
    std::cout << "dpp-pha start acquistion..." << std::endl;
    if (current_error_ != 0) throw std::runtime_error("Problems configuring all channels, err code " + std::to_string(current_error_));
    root_file_        = nullptr;
    if (rootfile_name_ != "")
        {root_file_   = new TFile(rootfile_name_.c_str(), "RECREATE");
         if (!root_file_) throw std::runtime_error("Problems with root file " + rootfile_name_);
        }
    channel_trees_.clear();
    energy_ch_.clear();
    waveform_ch_.clear();
    trigger_ch_.clear();
    saturated_ch_.clear();
    energy_ch_.reserve(8);
    waveform_ch_.reserve(8);
    channel_trees_.reserve(8);
    trigger_ch_.reserve(8);
    saturated_ch_.reserve(8);
    std::string ch_name = "ch";
    for (int k=0;k<8;k++)
        {
            ch_name = std::string("ch") + std::to_string(k);           
            channel_trees_.push_back(new TTree(ch_name.c_str(), ch_name.c_str()));
            channel_trees_[k]->Branch("energy", &energy_ch_[k]);
            if (decode_waveforms_)
                {
                    channel_trees_[k]->Branch("waveform",  &waveform_ch_[k]);
                    channel_trees_[k]->Branch("trigger",   &trigger_ch_[k]);
                    channel_trees_[k]->Branch("saturated", &saturated_ch_[k]);
                }
        } 
    n_events_acq_ = std::vector<long>(get_nchannels(), 0);
    current_error_ = CAEN_DGTZ_SWStartAcquisition(handle_);
}

/*******************************************************************/

int CaenN6725DPPPHA::get_current_sampling_rate() 
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

void CaenN6725DPPPHA::calibrate()
{
    current_error_ = CAENDGTZ_API CAEN_DGTZ_Calibrate(handle_);
    if (current_error_ != 0) throw std::runtime_error("Issue during calibration err code: " + std::to_string(current_error_));

}


/*******************************************************************/

uint32_t CaenN6725DPPPHA::get_channel_dc_offset(int channel)
{
    // offset has to be in DAC values!
    uint32_t offset;
    current_error_ = CAEN_DGTZ_GetChannelDCOffset(handle_,channel, &offset);
    if (current_error_ != 0) throw std::runtime_error("Can not get baseline offset for ch " + std::to_string(channel) + " err code: " + std::to_string(current_error_));
    return offset;
}

/*******************************************************************/

void CaenN6725DPPPHA::set_channel_trigger_threshold(int channel, int threshold)
{
    if ((threshold < 0) || (threshold > 16383))
        {throw std::runtime_error("Trigger threshold has to be in the interval [0,16383]");};
    current_error_ = CAEN_DGTZ_SetChannelTriggerThreshold(handle_, channel, threshold);
    switch (channel)
      {
        case 0: CAEN_DGTZ_WriteRegister(handle_, 0x1080, threshold); break;
        case 1: CAEN_DGTZ_WriteRegister(handle_, 0x1180, threshold); break;
        case 2: CAEN_DGTZ_WriteRegister(handle_, 0x1280, threshold); break;
        case 3: CAEN_DGTZ_WriteRegister(handle_, 0x1380, threshold); break;
        case 4: CAEN_DGTZ_WriteRegister(handle_, 0x1480, threshold); break;
        case 5: CAEN_DGTZ_WriteRegister(handle_, 0x1580, threshold); break;
        case 6: CAEN_DGTZ_WriteRegister(handle_, 0x1680, threshold); break;
        case 7: CAEN_DGTZ_WriteRegister(handle_, 0x1780, threshold); break;
        default: throw std::runtime_error("Can not identify channel " + std::to_string(channel));
      }
    if (current_error_ != 0) throw std::runtime_error("Can not set trigger threshold forfor ch " + std::to_string(channel) + " err code: " + std::to_string(current_error_));
}

/*******************************************************************/

void CaenN6725DPPPHA::continuous_readout(unsigned int seconds)
{
    // this is meant for fast continueous readout
    // set second vprobe to None, so we get the full 
    // sampling rate for the waveform
    current_error_ = CAEN_DGTZ_SetDPP_VirtualProbe(handle_, ANALOG_TRACE_2, CAEN_DGTZ_DPP_VIRTUALPROBE_None);
    if (current_error_ != 0) throw std::runtime_error("Can not set virtual probe to None, err code: "  + std::to_string(current_error_));
    
    //if (!decode_waveforms_)
    //    {
    //        //current_error_ = CAEN_DGTZ_SetDPPAcquisitionMode(handle_, CAEN_DGTZ_DPP_ACQ_MODE_List, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);    
    //        current_error_ = CAEN_DGTZ_SetDPPAcquisitionMode(handle_, CAEN_DGTZ_DPP_ACQ_MODE_Mixed, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);    
    //        if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP acquisition mode err code:" + std::to_string(current_error_));
    //    }
    //else 
    //    {
    //        current_error_ = CAEN_DGTZ_SetDPPAcquisitionMode(handle_, CAEN_DGTZ_DPP_ACQ_MODE_Mixed, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);    
    //        if (current_error_ !=0 ) throw std::runtime_error("Can not set DPP acquisition mode err code:" + std::to_string(current_error_));

    //    }
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

void CaenN6725DPPPHA::set_rootfilename(std::string fname)
{
    rootfile_name_ = fname;

}

/*******************************************************************/

void CaenN6725DPPPHA::clear_energy_histogram()
{
    energy_histogram_.clear();
    // 14bit digitizer
    std::vector<uint32_t> histo = std::vector<uint32_t>(16384,0);
    energy_histogram_ = std::vector<std::vector<uint32_t>>();
    fail_events_ = std::vector<uint32_t>(8,0);
    for (int k=0; k<get_nchannels(); k++)
        {energy_histogram_.push_back(histo);}
}

/*******************************************************************/

std::vector<uint32_t> CaenN6725DPPPHA::get_energy_histogram(int channel)
{
    return energy_histogram_[channel];
}

/*******************************************************************/

// FIXME: pro;er close function
CaenN6725DPPPHA::~CaenN6725DPPPHA()
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


