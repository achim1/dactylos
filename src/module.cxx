#include "CaenN6725.hh"
#include "trapezoidal_shaper.h" 

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>


namespace py = pybind11;

//std::string to_string(char c_string[])
//{
//    return std::string(c_string);
//}

PYBIND11_MODULE(_pyCaenN6725, m) {
    m.doc() = "Pybindings for CaenN6725 digitizer library";

    py::enum_<DynamicRange>(m, "DynamicRange")
        .value("VPP2", DynamicRange::VPP2)
        .value("VPP05", DynamicRange::VPP05)
        .export_values();

    py::enum_<CAEN_DGTZ_PulsePolarity_t>(m, "PulsePolarity")
        .value("Positive", CAEN_DGTZ_PulsePolarity_t::CAEN_DGTZ_PulsePolarityPositive)
        .value("Negative", CAEN_DGTZ_PulsePolarity_t::CAEN_DGTZ_PulsePolarityNegative)
        .export_values();

    py::enum_<CAEN_DGTZ_AcquisitionMode_t>(m, "AcquisitionMode")
        .value("STANDARD", CAEN_DGTZ_AcquisitionMode_t::CAEN_DGTZ_AcquisitionMode_STANDARD)
        .value("DPP_CI", CAEN_DGTZ_AcquisitionMode_t::CAEN_DGTZ_AcquisitionMode_DPP_CI)
        .export_values();   

    py::enum_<CAEN_DGTZ_IOLevel_t>(m, "IOLevel") 
        .value("NIM", CAEN_DGTZ_IOLevel_t::CAEN_DGTZ_IOLevel_NIM)
        .value("TTL", CAEN_DGTZ_IOLevel_t::CAEN_DGTZ_IOLevel_TTL) 
        .export_values();

    py::enum_<CAEN_DGTZ_ConnectionType>(m, "ConnectionType")
        .value("USB",              CAEN_DGTZ_ConnectionType::CAEN_DGTZ_USB)
        .value("OptionalLink",     CAEN_DGTZ_ConnectionType::CAEN_DGTZ_OpticalLink) 
        .value("PCI_OpticalLink",  CAEN_DGTZ_ConnectionType::CAEN_DGTZ_PCI_OpticalLink)
        .value("PCIE_OpticalLink", CAEN_DGTZ_ConnectionType::CAEN_DGTZ_PCIE_OpticalLink)
        .value("PCIE_EmbeddedDigitizer", CAEN_DGTZ_ConnectionType::CAEN_DGTZ_PCIE_OpticalLink)
        .export_values();

    py::enum_<CAEN_DGTZ_DPP_AcqMode_t>(m, "DPPAcqMode")
        .value("Oscilloscope", CAEN_DGTZ_DPP_AcqMode_t::CAEN_DGTZ_DPP_ACQ_MODE_Oscilloscope)        
        .value("List",         CAEN_DGTZ_DPP_AcqMode_t::CAEN_DGTZ_DPP_ACQ_MODE_List)
        .value("Mixed",        CAEN_DGTZ_DPP_AcqMode_t::CAEN_DGTZ_DPP_ACQ_MODE_Mixed)
        .export_values(); 

    py::class_<CAEN_DGTZ_DPP_PHA_Params_t>(m, "DPPPHAParams")
        .def(py::init())
        //int M           [MAX_DPP_PHA_CHANNEL_SIZE]; // Signal Decay Time Constant
        .def_property("M",      [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.M, std::end(p.M));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.M[i] = data[i];
                                    }
                                })
        //int m           [MAX_DPP_PHA_CHANNEL_SIZE]; // Trapezoid Flat Top
        .def_property("m",      [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.m, std::end(p.m));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.m[i] = data[i];
                                    }
                                })
        //int k           [MAX_DPP_PHA_CHANNEL_SIZE]; // Trapezoid Rise Time
        .def_property("k",      [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.k, std::end(p.k));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.k[i] = data[i];
                                    }
                                })
        //int ftd         [MAX_DPP_PHA_CHANNEL_SIZE]; //
        .def_property("ftd",    [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.ftd, std::end(p.ftd));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.ftd[i] = data[i];
                                    }
                                })
        //int a           [MAX_DPP_PHA_CHANNEL_SIZE]; // Trigger Filter smoothing factor
        .def_property("a",      [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.a, std::end(p.a));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.a[i] = data[i];
                                    }
                                })
        //int b           [MAX_DPP_PHA_CHANNEL_SIZE]; // Input Signal Rise time
        .def_property("b",      [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.b, std::end(p.b));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.b[i] = data[i];
                                    }
                                })
        //int thr         [MAX_DPP_PHA_CHANNEL_SIZE]; // Trigger Threshold
        .def_property("thr",    [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.thr, std::end(p.thr));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.thr[i] = data[i];
                                    }
                                })
        //int nsbl        [MAX_DPP_PHA_CHANNEL_SIZE]; // Number of Samples for Baseline Mean
        .def_property("nsbl",   [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.nsbl, std::end(p.nsbl));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.nsbl[i] = data[i];
                                    }
                                })
        //int nspk        [MAX_DPP_PHA_CHANNEL_SIZE]; // Number of Samples for Peak Mean Calculation
        .def_property("nspk",   [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<float> data(p.nspk, std::end(p.nspk));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<float> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.nspk[i] = data[i];
                                    }
                                })
        //int pkho        [MAX_DPP_PHA_CHANNEL_SIZE]; // Peak Hold Off
        .def_property("pkho",   [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.pkho, std::end(p.pkho));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.pkho[i] = data[i];
                                    }
                                })
        //int blho        [MAX_DPP_PHA_CHANNEL_SIZE]; // Base Line Hold Off
        .def_property("blho",   [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.blho, std::end(p.blho));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.blho[i] = data[i];
                                    }
                                })
        //int otrej       [MAX_DPP_PHA_CHANNEL_SIZE]; // 
        .def_property("otrej",  [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.otrej, std::end(p.otrej));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.otrej[i] = data[i];
                                    }
                                })
        //int trgho       [MAX_DPP_PHA_CHANNEL_SIZE]; // Trigger Hold Off
        .def_property("trgho",  [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.trgho, std::end(p.trgho));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.trgho[i] = data[i];
                                    }
                                })
        //int twwdt       [MAX_DPP_PHA_CHANNEL_SIZE]; // 
        .def_property("twwdt",  [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.twwdt, std::end(p.twwdt));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.twwdt[i] = data[i];
                                    }
                                })
        //int trgwin      [MAX_DPP_PHA_CHANNEL_SIZE]; //
        .def_property("trgwin", [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.trgwin, std::end(p.trgwin));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.trgwin[i] = data[i];
                                    }
                                })
        //int dgain       [MAX_DPP_PHA_CHANNEL_SIZE]; // Digital Probe Gain
        .def_property("dgain",  [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.dgain, std::end(p.dgain));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.dgain[i] = data[i];
                                    }
                                })
        //float enf       [MAX_DPP_PHA_CHANNEL_SIZE]; // Energy Nomralization Factor
        .def_property("enf",    [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<float> data(p.enf, std::end(p.enf));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<float> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.enf[i] = data[i];
                                    }
                                })
        //int decimation  [MAX_DPP_PHA_CHANNEL_SIZE]; // Decimation of Input Signal
        .def_property("decimation", [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.decimation, std::end(p.decimation));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.decimation[i] = data[i];
                                    }
                                })
        //int enskim      [MAX_DPP_PHA_CHANNEL_SIZE]; // Enable energy skimming
        .def_property("enskim", [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.enskim, std::end(p.enskim));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.enskim[i] = data[i];
                                    }
                                })
        //int eskimlld    [MAX_DPP_PHA_CHANNEL_SIZE]; // LLD    energy skimming
        .def_property("eskimlld",[](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.eskimlld, std::end(p.eskimlld));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.eskimlld[i] = data[i];
                                    }
                                })
        //int eskimuld    [MAX_DPP_PHA_CHANNEL_SIZE]; // ULD    energy skimming
        .def_property("eskimuld",[](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.eskimuld, std::end(p.eskimuld));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.eskimuld[i] = data[i];
                                    }
                                })
        //int blrclip     [MAX_DPP_PHA_CHANNEL_SIZE]; // Enable baseline restorer clipping
        .def_property("blrclip",[](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.blrclip, std::end(p.blrclip));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.blrclip[i] = data[i];
                                    }
                                })
        //int dcomp       [MAX_DPP_PHA_CHANNEL_SIZE]; // tt_filter compensation
        .def_property("dcomp", [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.dcomp, std::end(p.dcomp));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.dcomp[i] = data[i];
                                    }
                                })
        //int trapbsl     [MAX_DPP_PHA_CHANNEL_SIZE]; // trapezoid baseline adjuster
        .def_property("trapbsl", [](const CAEN_DGTZ_DPP_PHA_Params_t &p) {
                                std::vector<int> data(p.trapbsl, std::end(p.trapbsl));
                                return data;
                                },
                                [](CAEN_DGTZ_DPP_PHA_Params_t &p, std::vector<int> data) {
                                for (int i=0; i<data.size(); i++)
                                    {
                                        p.trapbsl[i] = data[i];
                                    }
                                });

    //py::class_<CAEN_DGTZ_DPP_PHA_Params_t>(m, "DPPPHAParams")
    //    .def(py::init())
    //    .def_readwrite("M",      &CAEN_DGTZ_DPP_PHA_Params_t::M) 
    //    .def_readwrite("m",      &CAEN_DGTZ_DPP_PHA_Params_t::m) 
    //    .def_readwrite("k",      &CAEN_DGTZ_DPP_PHA_Params_t::k) 
    //    .def_readwrite("ftd",    &CAEN_DGTZ_DPP_PHA_Params_t::ftd) 
    //    .def_readwrite("a",      &CAEN_DGTZ_DPP_PHA_Params_t::a) 
    //    .def_readwrite("b",      &CAEN_DGTZ_DPP_PHA_Params_t::b) 
    //    .def_readwrite("thr",    &CAEN_DGTZ_DPP_PHA_Params_t::thr) 
    //    .def_readwrite("nsbl",   &CAEN_DGTZ_DPP_PHA_Params_t::nsbl) 
    //    .def_readwrite("nspk",   &CAEN_DGTZ_DPP_PHA_Params_t::nspk) 
    //    .def_readwrite("pkho",   &CAEN_DGTZ_DPP_PHA_Params_t::pkho) 
    //    .def_readwrite("blho",   &CAEN_DGTZ_DPP_PHA_Params_t::blho) 
    //    .def_readwrite("otrej",  &CAEN_DGTZ_DPP_PHA_Params_t::otrej) 
    //    .def_readwrite("trgho",  &CAEN_DGTZ_DPP_PHA_Params_t::trgho) 
    //    .def_readwrite("twwdt",  &CAEN_DGTZ_DPP_PHA_Params_t::twwdt) 
    //    .def_readwrite("trgwin", &CAEN_DGTZ_DPP_PHA_Params_t::trgwin) 
    //    .def_readwrite("dgain",  &CAEN_DGTZ_DPP_PHA_Params_t::dgain) 
    //    .def_readwrite("enf",    &CAEN_DGTZ_DPP_PHA_Params_t::enf) 
    //    .def_readwrite("decimation", &CAEN_DGTZ_DPP_PHA_Params_t::decimation) 
    //    .def_readwrite("enskim",     &CAEN_DGTZ_DPP_PHA_Params_t::enskim) 
    //    .def_readwrite("eskimlld",&CAEN_DGTZ_DPP_PHA_Params_t::eskimlld) 
    //    .def_readwrite("eskimuld",&CAEN_DGTZ_DPP_PHA_Params_t::eskimuld) 
    //    .def_readwrite("blrclip", &CAEN_DGTZ_DPP_PHA_Params_t::blrclip) 
    //    .def_readwrite("dcomp",   &CAEN_DGTZ_DPP_PHA_Params_t::dcomp) 
    //    .def_readwrite("trapbsl", &CAEN_DGTZ_DPP_PHA_Params_t::trapbsl); 

    /*
    typedef struct
    {
        int M           [MAX_DPP_PHA_CHANNEL_SIZE]; // Signal Decay Time Constant
        int m           [MAX_DPP_PHA_CHANNEL_SIZE]; // Trapezoid Flat Top
        int k           [MAX_DPP_PHA_CHANNEL_SIZE]; // Trapezoid Rise Time
        int ftd         [MAX_DPP_PHA_CHANNEL_SIZE]; //
        int a           [MAX_DPP_PHA_CHANNEL_SIZE]; // Trigger Filter smoothing factor
        int b           [MAX_DPP_PHA_CHANNEL_SIZE]; // Input Signal Rise time
        int thr         [MAX_DPP_PHA_CHANNEL_SIZE]; // Trigger Threshold
        int nsbl        [MAX_DPP_PHA_CHANNEL_SIZE]; // Number of Samples for Baseline Mean
        int nspk        [MAX_DPP_PHA_CHANNEL_SIZE]; // Number of Samples for Peak Mean Calculation
        int pkho        [MAX_DPP_PHA_CHANNEL_SIZE]; // Peak Hold Off
        int blho        [MAX_DPP_PHA_CHANNEL_SIZE]; // Base Line Hold Off
        int otrej       [MAX_DPP_PHA_CHANNEL_SIZE]; // 
        int trgho       [MAX_DPP_PHA_CHANNEL_SIZE]; // Trigger Hold Off
        int twwdt       [MAX_DPP_PHA_CHANNEL_SIZE]; // 
        int trgwin      [MAX_DPP_PHA_CHANNEL_SIZE]; //
        int dgain       [MAX_DPP_PHA_CHANNEL_SIZE]; // Digital Probe Gain
        float enf       [MAX_DPP_PHA_CHANNEL_SIZE]; // Energy Nomralization Factor
        int decimation  [MAX_DPP_PHA_CHANNEL_SIZE]; // Decimation of Input Signal
        int enskim      [MAX_DPP_PHA_CHANNEL_SIZE]; // Enable energy skimming
        int eskimlld    [MAX_DPP_PHA_CHANNEL_SIZE]; // LLD    energy skimming
        int eskimuld    [MAX_DPP_PHA_CHANNEL_SIZE]; // ULD    energy skimming
        int blrclip     [MAX_DPP_PHA_CHANNEL_SIZE]; // Enable baseline restorer clipping
        int dcomp       [MAX_DPP_PHA_CHANNEL_SIZE]; // tt_filter compensation
        int trapbsl     [MAX_DPP_PHA_CHANNEL_SIZE]; // trapezoid baseline adjuster
    } CAEN_DGTZ_DPP_PHA_Params_t;
    */

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
        //.def_readwrite("model_name",&CAEN_DGTZ_BoardInfo_t::ModelName)
        .def("get_model_name",
                [](const CAEN_DGTZ_BoardInfo_t &b) {
                return std::string(b.ModelName);
                })
        .def("__repr__",
                [](const CAEN_DGTZ_BoardInfo_t &b) {
                return "<Caen digitizer model name  '" + std::string(b.ModelName) + "'>";
                })
        .def_readwrite("model", &CAEN_DGTZ_BoardInfo_t::Model)
        .def_readwrite("channels", &CAEN_DGTZ_BoardInfo_t::Channels)
        .def_readwrite("form_factor",&CAEN_DGTZ_BoardInfo_t::FormFactor)
        .def_readwrite("family_code",&CAEN_DGTZ_BoardInfo_t::FamilyCode)
//        .def_readwrite("roc_firmware_rel",&CAEN_DGTZ_BoardInfo_t::ROC_FirmwareRel)
//        .def_readwrite("amc_firmware_rel",&CAEN_DGTZ_BoardInfo_t::AMC_FirmwareRel)
        .def("get_model_roc_firmware_rel",
                [](const CAEN_DGTZ_BoardInfo_t &b) {
                return std::string(b.ROC_FirmwareRel);
                })
        .def("get_amc_firmware_rel",
                [](const CAEN_DGTZ_BoardInfo_t &b) {
                return std::string(b.AMC_FirmwareRel);
                })
        .def_readwrite("serial_number",&CAEN_DGTZ_BoardInfo_t::SerialNumber)

        // FIXME mezzanine is an array of arrays I think
        //.def("get_mezzanine_ser_num",
        //        [](const CAEN_DGTZ_BoardInfo_t &b) {
        //        return std::string(b.MezzanineSerNum);
        //        })
//        .def_readwrite("mezzanine_ser_num",&CAEN_DGTZ_BoardInfo_t::MezzanineSerNum)       //used only for x743 boards
        .def_readwrite("pcb_revisiion",&CAEN_DGTZ_BoardInfo_t::PCB_Revision)
        .def_readwrite("adc_bits",&CAEN_DGTZ_BoardInfo_t::ADC_NBits)
        .def_readwrite("sam_correction_data_loaded",&CAEN_DGTZ_BoardInfo_t::SAMCorrectionDataLoaded)        //used only for x743 boards
        .def_readwrite("comm_handle",&CAEN_DGTZ_BoardInfo_t::CommHandle)
        .def_readwrite("vme_handle",&CAEN_DGTZ_BoardInfo_t::VMEHandle)
        .def("get_license",
                [](const CAEN_DGTZ_BoardInfo_t &b) {
                return std::string(b.License);
                });
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
        .value("ALL", CHANNEL::ALL)
        .export_values();

    py::class_<DigitizerParams_t>(m, "DigitizerParams")
        .def(py::init())
        .def_readwrite("LinkType", &DigitizerParams_t::LinkType)
        .def_readwrite("VMEBaseAddress", &DigitizerParams_t::VMEBaseAddress)
        .def_readwrite("RecordLength", &DigitizerParams_t::RecordLength)
        .def_readwrite("ChannelMask", &DigitizerParams_t::ChannelMask)
        .def_readwrite("EventAggr", &DigitizerParams_t::EventAggr)
        .def_readwrite("PostTriggerPercent", &DigitizerParams_t::PostTriggerPercent)
        .def_readwrite("PulsePolarity", &DigitizerParams_t::PulsePolarity)
        .def_readwrite("DPPAcqMode", &DigitizerParams_t::AcqMode)
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

    // own enums to help with the virtual/digital probes
    py::enum_<DPPVirtualProbe1>(m, "DPPVirtualProbe1")
        .value("Input",     DPPVirtualProbe1::Input)
        .value("Delta",     DPPVirtualProbe1::Delta)
        .value("Delta2",    DPPVirtualProbe1::Delta2)
        .value("Trapezoid", DPPVirtualProbe1::Trapezoid)
        .export_values();

    py::enum_<DPPVirtualProbe2>(m, "DPPVirtualProbe2")
        .value("Input",            DPPVirtualProbe2::Input)
        .value("TrapezoidReduced", DPPVirtualProbe2::TrapezoidReduced)
        .value("Baseline",         DPPVirtualProbe2::Baseline)
        .value("Threshold",        DPPVirtualProbe2::Threshold)
        .value("NONE",             DPPVirtualProbe2::None)
        .export_values();

    py::enum_<DPPDigitalProbe1>(m, "DPPDigitalProbe1")
        .value("TRGWin",     DPPDigitalProbe1::TRGWin)
        .value("Armed",      DPPDigitalProbe1::Armed)
        .value("PkRun",      DPPDigitalProbe1::PkRun)
        .value("Peaking",    DPPDigitalProbe1::Peaking)
        .value("CoincWin",   DPPDigitalProbe1::CoincWin)
        .value("TRGHoldoff", DPPDigitalProbe1::TRGHoldoff)
        .value("ACQVeto",    DPPDigitalProbe1::ACQVeto)
        .value("BFMVeto",    DPPDigitalProbe1::BFMVeto)
        .value("ExtTRG",     DPPDigitalProbe1::ExtTRG)
        .value("Busy",       DPPDigitalProbe1::Busy)
        .value("PrgVeto",    DPPDigitalProbe1::PrgVeto)
        .value("PileUp",     DPPDigitalProbe1::PileUp)
        .value("BLFreeze",   DPPDigitalProbe1::BLFreeze)
        .export_values();

    py::enum_<DPPDigitalProbe2>(m, "DPPDigitalProbe2")
        .value("Trigger", DPPDigitalProbe2::Trigger)
        .export_values();

    py::class_<CaenN6725DPPPHA>(m, "CaenN6725DPPPHA")
        .def(py::init<DigitizerParams_t>())
        .def(py::init())
        .def("get_handle",                    &CaenN6725DPPPHA::get_handle)
        .def("set_handle",                    &CaenN6725DPPPHA::set_handle)
        .def("connect",                       &CaenN6725DPPPHA::connect)
        .def("configure",                     &CaenN6725DPPPHA::configure)
        .def("get_time",                      &CaenN6725DPPPHA::get_time)
        .def("enable_waveform_decoding",      &CaenN6725DPPPHA::enable_waveform_decoding)
        .def("get_last_error",                &CaenN6725DPPPHA::get_last_error)
        .def("get_board_info",                &CaenN6725DPPPHA::get_board_info)
        .def("allocate_memory",               &CaenN6725DPPPHA::allocate_memory)
        .def("get_allocated_buffer_size",     &CaenN6725DPPPHA::get_allocated_buffer_size)
        .def("start_acquisition",             &CaenN6725DPPPHA::start_acquisition)
        .def("end_acquisition",               &CaenN6725DPPPHA::end_acquisition)
        .def("get_nchannels",                 &CaenN6725DPPPHA::get_nchannels)
        .def("get_temperatures",              &CaenN6725DPPPHA::get_temperatures)
        .def("configure_channel",             &CaenN6725DPPPHA::configure_channel)
        .def("calibrate",                     &CaenN6725DPPPHA::calibrate)
        .def("read_data",                     &CaenN6725DPPPHA::read_data)
        .def("get_last_waveform",             &CaenN6725DPPPHA::get_last_waveform)
        .def("clear_energy_histogram",        &CaenN6725DPPPHA::clear_energy_histogram)
        .def("get_energy_histogram",          &CaenN6725DPPPHA::get_energy_histogram)
        .def("get_current_sampling_rate",     &CaenN6725DPPPHA::get_current_sampling_rate)
        .def("get_analog_trace1",             &CaenN6725DPPPHA::get_analog_trace1)
        .def("get_analog_trace2",             &CaenN6725DPPPHA::get_analog_trace2)
        .def("get_digital_trace1",            &CaenN6725DPPPHA::get_digital_trace1)
        .def("get_digital_trace2",            &CaenN6725DPPPHA::get_digital_trace2)
        .def("get_trigger_point",             &CaenN6725DPPPHA::get_trigger_point)
        .def("set_vprobe1",                   &CaenN6725DPPPHA::set_virtualprobe1)
        .def("set_vprobe2",                   &CaenN6725DPPPHA::set_virtualprobe2)
        .def("set_dprobe1",                   &CaenN6725DPPPHA::set_digitalprobe1)
        .def("set_dprobe2",                   &CaenN6725DPPPHA::set_digitalprobe2)
        .def("continuous_readout",            &CaenN6725DPPPHA::continuous_readout)
        .def("is_active",                     &CaenN6725DPPPHA::is_active)
        .def("set_rootfilename",              &CaenN6725DPPPHA::set_rootfilename)
        .def("set_channel_dc_offset",         &CaenN6725DPPPHA::set_channel_dc_offset)
        .def("get_channel_dc_offset",         &CaenN6725DPPPHA::get_channel_dc_offset)
        .def("set_channel_trigger_threshold", &CaenN6725DPPPHA::set_channel_trigger_threshold)
        .def("get_n_events",                  &CaenN6725DPPPHA::get_n_events)
        .def("get_n_events_tot",              &CaenN6725DPPPHA::get_n_events_tot)
        .def("get_n_triggers_tot",            &CaenN6725DPPPHA::get_n_triggers_tot)
        .def("get_n_lost_triggers_tot",       &CaenN6725DPPPHA::get_n_lost_triggers_tot)
        .def("get_energy",                    &CaenN6725DPPPHA::get_energy)
        .def("set_input_dynamic_range",       &CaenN6725DPPPHA::set_input_dynamic_range)
        .def("get_input_dynamic_range",       &CaenN6725DPPPHA::get_input_dynamic_range);

    py::class_<CaenN6725WF>(m, "CaenN6725WF")
        .def(py::init<DigitizerParams_t>())
        .def(py::init())
        .def("get_handle",                    &CaenN6725WF::get_handle)
        .def("set_handle",                    &CaenN6725WF::set_handle)
        .def("connect",                       &CaenN6725WF::connect)
        .def("configure",                     &CaenN6725WF::configure)
        .def("get_time",                      &CaenN6725WF::get_time)
        //.def("enable_waveform_decoding",      &CaenN6725WF::enable_waveform_decoding)
        .def("get_last_error",                &CaenN6725WF::get_last_error)
        .def("get_board_info",                &CaenN6725WF::get_board_info)
        .def("allocate_memory",               &CaenN6725WF::allocate_memory)
        .def("get_allocated_buffer_size",     &CaenN6725WF::get_allocated_buffer_size)
        .def("start_acquisition",             &CaenN6725WF::start_acquisition)
        .def("end_acquisition",               &CaenN6725WF::end_acquisition)
        .def("get_nchannels",                 &CaenN6725WF::get_nchannels)
        .def("get_temperatures",              &CaenN6725WF::get_temperatures)
        .def("get_expected_baseline",         &CaenN6725WF::get_expected_baseline)
        //.def("configure_channel",             &CaenN6725WF::configure_channel)
        .def("calibrate",                     &CaenN6725WF::calibrate)
        //.def("read_data",                     &CaenN6725WF::read_data)
        //.def("get_current_sampling_rate",     &CaenN6725WF::get_current_sampling_rate)
        //.def("get_analog_trace1",             &CaenN6725WF::get_analog_trace1)
        //.def("get_analog_trace2",             &CaenN6725WF::get_analog_trace2)
        //.def("get_digital_trace1",            &CaenN6725WF::get_digital_trace1)
        //.def("get_digital_trace2",            &CaenN6725WF::get_digital_trace2)
        //.def("get_trigger_point",             &CaenN6725WF::get_trigger_point)
        //.def("set_vprobe1",                   &CaenN6725WF::set_virtualprobe1)
        //.def("set_vprobe2",                   &CaenN6725WF::set_virtualprobe2)
        //.def("set_dprobe1",                   &CaenN6725WF::set_digitalprobe1)
        //.def("set_dprobe2",                   &CaenN6725WF::set_digitalprobe2)
        //.def("continuous_readout",            &CaenN6725WF::continuous_readout)
        //.def("is_active",                     &CaenN6725WF::is_active)
        .def("set_rootfilename",              &CaenN6725WF::set_rootfilename)
        .def("set_channel_dc_offset",         &CaenN6725WF::set_channel_dc_offset)
        .def("get_channel_dc_offset",         &CaenN6725WF::get_channel_dc_offset)
        .def("set_channel_trigger_threshold", &CaenN6725WF::set_channel_trigger_threshold)
        //.def("get_n_events",                  &CaenN6725WF::get_n_events)
        .def("get_n_events_tot",              &CaenN6725WF::get_n_events_tot)
        //.def("get_n_triggers_tot",            &CaenN6725WF::get_n_triggers_tot)
        //.def("get_n_lost_triggers_tot",       &CaenN6725WF::get_n_lost_triggers_tot)
        //.def("get_energy",                    &CaenN6725WF::get_energy)
        .def("readout_and_save",              &CaenN6725WF::readout_and_save)
        .def("readout_and_return",            &CaenN6725WF::readout_and_return)
        .def("set_input_dynamic_range",       &CaenN6725WF::set_input_dynamic_range)
        .def("get_input_dynamic_range",       &CaenN6725WF::get_input_dynamic_range);

    // the trapezoidal filter
    py::class_<TrapezoidalFilter>(m, "TrapezoidalFilter")
        .def(py::init<int, int, int>())//,
            // FIXME: the proper constructor with keyword support 
            // py::arg("ptime"), py::kwarg("flat")=1000., py::kwarg("recordlength")=50000
            //)
        .def("shape_it", &TrapezoidalFilter::shape_it)
        // we need __getstate__ and __setstate__ so that we are capable of pickling our class
        // - this is important for the use with python multiprocessing module, 
        // since this requires pickleable objects.
        .def("__getstate__", [](const TrapezoidalFilter &t) {
            return py::make_tuple(t.ptime, t.flat, t.recordlength);
        })
        .def("__setstate__", [](TrapezoidalFilter &trap, py::tuple t) {
            if (t.size() != 3)
                throw std::runtime_error("Invalid state!");
            new (&trap) TrapezoidalFilter(t[0].cast<int>(),t[1].cast<int>(),t[2].cast<int>());
        });



};
