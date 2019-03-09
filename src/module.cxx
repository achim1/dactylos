#include "CLCAEN6725.hh"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>


namespace py = pybind11;

std::string to_string(char c_string[])
{
    return std::string(c_string);
}


//uint32_t trigger_threshold; //100 (number of bins y axis)
//uint32_t trapezoidal_rise_time; //4 microsec // shaping time
//uint32_t trapezoidal_flat_top; //1 microsec
//uint32_t input_decay_time; //10microsec
//uint32_t flat_top_delay; //80 per cent of trapezoid flat top
//uint32_t input_signal_rise_time; // 50ns
//// different for detector set to 80microsec, for muon 100
//uint32_t trigger_filter_smoothing_factor; //16
//uint32_t trigger_hold_off; // to avoid pile up the longer the better 5microsec     
//uint32_t nsamples_baseline;// 5        
//uint32_t peak_mean; //2 
//uint32_t peak_holdoff; //5 microsec
//uint32_t baseline_holdoff;// - unknown
//float    energy_normalization;// 1.0
//uint32_t decimation;// 0
//uint32_t decimation_gain;// 0 
//uint32_t otrej; //unknown
//uint32_t enable_rise_time_discrimination;//
//uint32_t rise_time_validation_window;
PYBIND11_MODULE(pyCaenN6725, m) {
    m.doc() = "pybind example";

    // all the stuff from the original library
    py::class_<CAEN_DGTZ_DPP_PHA_Event_t>(m, "DPPEvent")
        .def(py::init())
        .def_readwrite("format", &CAEN_DGTZ_DPP_PHA_Event_t::Format)
        .def_readwrite("time_tag", &CAEN_DGTZ_DPP_PHA_Event_t::TimeTag)
        .def_readwrite("energy", &CAEN_DGTZ_DPP_PHA_Event_t::Energy)
        .def_readwrite("extras", &CAEN_DGTZ_DPP_PHA_Event_t::Extras)
        .def_readwrite("waveforms", &CAEN_DGTZ_DPP_PHA_Event_t::Waveforms)
        .def_readwrite("extras2", &CAEN_DGTZ_DPP_PHA_Event_t::Extras2);

    py::enum_<CAEN_DGTZ_ErrorCode>(m, "CaenErrorCode")
        .value("CAEN_DGTZ_Success"                    , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_Success) 
        .value("CAEN_DGTZ_CommError"                  , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_CommError) 
        .value("CAEN_DGTZ_GenericError"               , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_GenericError) 
        .value("CAEN_DGTZ_InvalidParam"               , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_InvalidParam) 
        .value("CAEN_DGTZ_InvalidLinkType"            , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_InvalidLinkType) 
        .value("CAEN_DGTZ_InvalidHandle"              , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_InvalidHandle) 
        .value("CAEN_DGTZ_MaxDevicesError"            , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_MaxDevicesError) 
        .value("CAEN_DGTZ_BadBoardType"               , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_BadBoardType)
        .value("CAEN_DGTZ_BadInterruptLev"            , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_BadInterruptLev) 
        .value("CAEN_DGTZ_BadEventNumber"             , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_BadEventNumber)
        .value("CAEN_DGTZ_ReadDeviceRegisterFail"     , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_ReadDeviceRegisterFail) 
        .value("CAEN_DGTZ_WriteDeviceRegisterFail"    , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_WriteDeviceRegisterFail)
        .value("CAEN_DGTZ_InvalidChannelNumber"       , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_InvalidChannelNumber) 
        .value("CAEN_DGTZ_ChannelBusy"                , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_ChannelBusy)
        .value("CAEN_DGTZ_FPIOModeInvalid"            , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_FPIOModeInvalid)
        .value("CAEN_DGTZ_WrongAcqMode"               , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_WrongAcqMode) 
        .value("CAEN_DGTZ_FunctionNotAllowed"         , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_FunctionNotAllowed) 
        .value("CAEN_DGTZ_Timeout"                    , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_Timeout)
        .value("CAEN_DGTZ_InvalidBuffer"              , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_InvalidBuffer) 
        .value("CAEN_DGTZ_EventNotFound"              , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_EventNotFound) 
        .value("CAEN_DGTZ_InvalidEvent"               , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_InvalidEvent) 
        .value("CAEN_DGTZ_OutOfMemory"                , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_OutOfMemory)
        .value("CAEN_DGTZ_CalibrationError"           , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_CalibrationError)
        .value("CAEN_DGTZ_DigitizerNotFound"          , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_DigitizerNotFound) 
        .value("CAEN_DGTZ_DigitizerAlreadyOpen"       , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_DigitizerAlreadyOpen)
        .value("CAEN_DGTZ_DigitizerNotReady"          , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_DigitizerNotReady)
        .value("CAEN_DGTZ_InterruptNotConfigured"     , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_InterruptNotConfigured)
        .value("CAEN_DGTZ_DigitizerMemoryCorrupted"   , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_DigitizerMemoryCorrupted) 
        .value("CAEN_DGTZ_DPPFirmwareNotSupported"    , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_DPPFirmwareNotSupported) 
        .value("CAEN_DGTZ_InvalidLicense"             , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_InvalidLicense) 
        .value("CAEN_DGTZ_InvalidDigitizerStatus"     , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_InvalidDigitizerStatus) 
        .value("CAEN_DGTZ_UnsupportedTrace"           , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_UnsupportedTrace) 
        .value("CAEN_DGTZ_InvalidProbe"               , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_InvalidProbe)
        .value("CAEN_DGTZ_UnsupportedBaseAddress"     , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_UnsupportedBaseAddress)
        .value("CAEN_DGTZ_NotYetImplemented"          , CAEN_DGTZ_ErrorCode::CAEN_DGTZ_NotYetImplemented)
        .export_values();        


    // FIXME: the commented methods need setters/getters
    py::class_<CAEN_DGTZ_BoardInfo_t>(m, "BoardInfo")
        .def(py::init())
        .def_readwrite("model_name",&CAEN_DGTZ_BoardInfo_t::ModelName)
        .def_readwrite("model", &CAEN_DGTZ_BoardInfo_t::Model)
        .def_readwrite("channels", &CAEN_DGTZ_BoardInfo_t::Channels)
        .def_readwrite("form_factor",&CAEN_DGTZ_BoardInfo_t::FormFactor)
        .def_readwrite("family_code",&CAEN_DGTZ_BoardInfo_t::FamilyCode)
//        .def_readwrite("roc_firmware_rel",&CAEN_DGTZ_BoardInfo_t::ROC_FirmwareRel)
//        .def_readwrite("amc_firmware_rel",&CAEN_DGTZ_BoardInfo_t::AMC_FirmwareRel)
        .def_readwrite("serial_number",&CAEN_DGTZ_BoardInfo_t::SerialNumber)
//        .def_readwrite("mezzanine_ser_num",&CAEN_DGTZ_BoardInfo_t::MezzanineSerNum)       //used only for x743 boards
        .def_readwrite("pcb_revisiion",&CAEN_DGTZ_BoardInfo_t::PCB_Revision)
        .def_readwrite("adc_bits",&CAEN_DGTZ_BoardInfo_t::ADC_NBits)
        .def_readwrite("sam_correction_data_loaded",&CAEN_DGTZ_BoardInfo_t::SAMCorrectionDataLoaded)        //used only for x743 boards
        .def_readwrite("comm_handle",&CAEN_DGTZ_BoardInfo_t::CommHandle)
        .def_readwrite("vme_handle",&CAEN_DGTZ_BoardInfo_t::VMEHandle);
//        .def_readwrite("licencse",&CAEN_DGTZ_BoardInfo_t::License);
////} CAEN_DGTZ_BoardInfo_t;
//


    py::enum_<CHANNEL>(m, "CHANNEL")
        .value("CH0", CHANNEL::CH0)
        .value("CH1", CHANNEL::CH1)
        .value("CH2", CHANNEL::CH2)
        .value("CH3", CHANNEL::CH3)
        .value("CH4", CHANNEL::CH4)
        .value("CH5", CHANNEL::CH5)
        .value("CH6", CHANNEL::CH6)
        .value("CH7", CHANNEL::CH7)
        .export_values();

    py::class_<DigitizerParams_t>(m, "DigitizerParams")
        .def(py::init())
        .def_readwrite("LinkType", &DigitizerParams_t::LinkType)
        .def_readwrite("VMEBaseAddress", &DigitizerParams_t::VMEBaseAddress)
        .def_readwrite("RecordLength", &DigitizerParams_t::RecordLength)
        .def_readwrite("ChannelMask", &DigitizerParams_t::ChannelMask)
        .def_readwrite("EventAggr", &DigitizerParams_t::EventAggr)
        .def_readwrite("PulsePolarity", &DigitizerParams_t::PulsePolarity)
        .def_readwrite("AcqMode", &DigitizerParams_t::AcqMode)
        .def_readwrite("IOlev", &DigitizerParams_t::IOlev)
        .def_readwrite("DPPParams", &DigitizerParams_t::DPPParams);


    py::class_<ChannelParams_t>(m, "ChannelParams")
        .def(py::init())
        .def_readwrite("trigger_threshold", &ChannelParams_t::trigger_threshold)
        .def_readwrite("trapezoidal_rise_time", &ChannelParams_t::trapezoidal_rise_time)
        .def_readwrite("trapezoidal_flat_top", &ChannelParams_t::trapezoidal_flat_top)
        .def_readwrite("input_decay_time", &ChannelParams_t::input_decay_time)
        .def_readwrite("flat_top_delay", &ChannelParams_t::flat_top_delay)
        .def_readwrite("input_signal_rise_time", &ChannelParams_t::input_signal_rise_time)
        .def_readwrite("trigger_filter_smoothing_factor", &ChannelParams_t::trigger_filter_smoothing_factor)
        .def_readwrite("trigger_hold_off", &ChannelParams_t::trigger_hold_off)
        .def_readwrite("nsamples_baseline", &ChannelParams_t::nsamples_baseline)
        .def_readwrite("peak_mean", &ChannelParams_t::peak_mean)
        .def_readwrite("peak_holdoff", &ChannelParams_t::peak_holdoff)
        .def_readwrite("baseline_holdoff", &ChannelParams_t::baseline_holdoff)
        .def_readwrite("energy_normalization", &ChannelParams_t::energy_normalization)
        .def_readwrite("decimation", &ChannelParams_t::decimation)
        .def_readwrite("decimation_gain", &ChannelParams_t::decimation_gain)
        .def_readwrite("otrej", &ChannelParams_t::otrej)
        .def_readwrite("enable_rise_time_discrimination", &ChannelParams_t::enable_rise_time_discrimination)
        .def_readwrite("rise_time_validation_window", &ChannelParams_t::rise_time_validation_window);
        //.def(py::init<const std::string &>())
        //.def("setName", &Pet::setName)
        //.def("getName", &Pet::getName);

    py::class_<CaenN6725>(m, "CaenN6725")
        .def(py::init())
        .def("get_time", &CaenN6725::get_time)
        .def("get_last_error", &CaenN6725::get_last_error)
        .def("get_board_info", &CaenN6725::get_board_info)
        .def("allocate_memory", &CaenN6725::allocate_memory)
        .def("start_acquisition", &CaenN6725::start_acquisition)
        .def("end_acquisition", &CaenN6725::end_acquisition)
        .def("get_nchannels", &CaenN6725::get_nchannels)
        .def("get_temperatures", &CaenN6725::get_temperatures)
        .def("configure_channels", &CaenN6725::configure_channels)
        .def("calibrate", &CaenN6725::calibrate)
        .def("read_data", &CaenN6725::read_data)
        .def("get_n_events", &CaenN6725::get_n_events);
};
