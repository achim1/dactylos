#include "CLCAEN6725.hh"

#include <pybind11/pybind11.h>

namespace py = pybind11;

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

    py::class_<DigitizerParams_t>(m, "DigitizerParams")
        .def(py::init());


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
};
