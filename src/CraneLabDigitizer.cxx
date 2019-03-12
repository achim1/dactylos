
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

#include "CLCAEN6725.hh"

// constants
// digitizer channels - we have 14 bit, so that 16834 channels
static const int NBINS(16834);

/***************************************************************/

typedef std::vector<TH1I*> HistogramSeries;

/***************************************************************/

void extract_channel_pars(GOptionParser parser, CAEN_DGTZ_DPP_PHA_Params_t* DPPParams, int nchannels = 8)
{
    for (unsigned int ch=0; ch<nchannels; ch++)
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
};

/***************************************************************/

void readout_loop(CaenN6725* digitizer,std::vector<int> &n_acquired, HistogramSeries &histos)
{

    auto events = digitizer->read_data();
    auto nevents = digitizer->get_n_events();
    if (events.size() != 0)
    {
      for (unsigned int ch=0; ch<digitizer->get_nchannels(); ch++)
        {
         n_acquired[ch] += nevents[ch];
         for (int ev=0; ev<nevents[ch]; ev++)
             {
                 auto energy = events[ch][ev].Energy;
                 INFO(energy);
                 histos.at(ch)->Fill(energy);
             }
        }
    }

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
    parser.AddOption<int>("runtime", "in s [DEFAULT = -1, run as long as needed to acquire given number of events]", -1);
    parser.AddOption<int>("n-events", "Each active channel will acquire this amoutn of events", -1); 
    parser.AddOption<int>("n-events-ch1", "Acquire N events for channel 1", 0); 
    parser.AddOption<int>("n-events-ch2", "Acquire N events for channel 2", 0); 
    parser.AddOption<int>("n-events-ch3", "Acquire N events for channel 3", 0); 
    parser.AddOption<int>("n-events-ch4", "Acquire N events for channel 4", 0); 
    parser.AddOption<int>("n-events-ch5", "Acquire N events for channel 5", 0); 
    parser.AddOption<int>("n-events-ch6", "Acquire N events for channel 6", 0); 
    parser.AddOption<int>("n-events-ch7", "Acquire N events for channel 7", 0); 
    parser.AddOption<int>("n-events-ch8", "Acquire N events for channel 8", 0); 
    parser.Parse();
    

    CAEN_DGTZ_DPP_PHA_Params_t* dpp_params = new CAEN_DGTZ_DPP_PHA_Params_t();
    extract_channel_pars(parser, dpp_params); 

    // initializer values for the digitizer
    DigitizerParams_t digi_params;
    digi_params.LinkType = CAEN_DGTZ_USB;  // Link Type
    digi_params.VMEBaseAddress = 0;  // For direct USB connection, VMEBaseAddress must be 0
    digi_params.IOlev = CAEN_DGTZ_IOLevel_NIM;

    /****************************\
    *  Acquisition parameters    *
    \****************************/
    digi_params.AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Mixed;          // CAEN_DGTZ_DPP_ACQ_MODE_List or CAEN_DGTZ_DPP_ACQ_MODE_Oscilloscope
    digi_params.RecordLength = 2000;                              // Num of samples of the waveforms (only for Oscilloscope mode)
    digi_params.ChannelMask = CHANNEL::ALL;                               // Channel enable mask
    digi_params.EventAggr = 0;                                   // number of events in one aggregate (0=automatic)
    digi_params.PulsePolarity = CAEN_DGTZ_PulsePolarityPositive; // Pulse Polarity (this parameter can be individual)


    // initialize the digitizer
    CaenN6725 digitizer(digi_params);

    // initialize histograms
    HistogramSeries histos;
    histos.reserve(8);
    std::string histname;
    std::string histtitle;
    for (unsigned int ch=0; ch<digitizer.get_nchannels(); ch++)
        {
            histname = "ehistch" + std::to_string(ch);
            histtitle = "DigitizerChannel Ch" + std::to_string(ch);
            histos.push_back(new TH1I(histname.c_str(), histtitle.c_str(), NBINS, 0, NBINS-1));
        }

    // prepare output file
    std::string outfile = parser.GetOption<std::string>("output-file");
    INFO("Writing to root file " << outfile);
    TFile* output = new TFile(outfile.c_str(),"RECREATE");

    CAEN_DGTZ_BoardInfo_t board_info = digitizer.get_board_info(); 

    INFO("\nConnected to CAEN Digitizer Model " << board_info.ModelName <<", recognized as board 0\n");
    INFO("ROC FPGA Release is " << board_info.ROC_FirmwareRel);
    INFO("AMC FPGA Release is " << board_info.AMC_FirmwareRel);
    std::vector<int> temps = digitizer.get_temperatures();
    for (uint ch=0; ch<digitizer.get_nchannels(); ch++)
        {
            INFO("Ch " << ch << " ADC temperature: " <<  temps[ch]);
        }
    digitizer.configure_channels(dpp_params);
    /* WARNING: The mallocs MUST be done after the digitizer programming,
    because the following functions needs to know the digitizer configuration
    to allocate the right memory amount */
    /* Allocate memory for the readout buffer */
    digitizer.allocate_memory();

    // from here on, acquisition
    digitizer.start_acquisition();
    int run_time = parser.GetOption<int>("runtime");
    std::vector<int> n_acquired = {};
    for (int k=0; k<digitizer.get_nchannels(); k++)
      {
          n_acquired.push_back(0);
      }

    // run time mode
    if (run_time > 0)
      {
        INFO("Will acquire data for " << run_time << " seconds");
        long timeDelta = 0;
        long currentTime = digitizer.get_time(); // time in millisec
        while (timeDelta < 1000*run_time){
          readout_loop(&digitizer, n_acquired, histos);
          long newTime = digitizer.get_time();
          timeDelta += newTime - currentTime;
          currentTime = newTime;
        }
      }
    else
      {
        int nevents_per_channel = parser.GetOption<int>("n-events");
        std::vector<int> to_acquire({});
        if (nevents_per_channel > 0)
          {
          for (int k=0; k<digitizer.get_nchannels(); k++)
            {
                to_acquire.push_back(nevents_per_channel);
            }

          }
        else 
          {
            to_acquire.push_back(parser.GetOption<int>("n-events-ch1"));
            to_acquire.push_back(parser.GetOption<int>("n-events-ch2"));
            to_acquire.push_back(parser.GetOption<int>("n-events-ch3"));
            to_acquire.push_back(parser.GetOption<int>("n-events-ch4"));
            to_acquire.push_back(parser.GetOption<int>("n-events-ch5"));
            to_acquire.push_back(parser.GetOption<int>("n-events-ch6"));
            to_acquire.push_back(parser.GetOption<int>("n-events-ch7"));
            to_acquire.push_back(parser.GetOption<int>("n-events-ch8"));
          }

        //while (n_acquired < nevents_per_channel)
        while (n_acquired[0] < to_acquire[0] ||
               n_acquired[1] < to_acquire[1] ||
               n_acquired[2] < to_acquire[2] ||
               n_acquired[3] < to_acquire[3] ||
               n_acquired[4] < to_acquire[4] ||
               n_acquired[5] < to_acquire[5] ||
               n_acquired[6] < to_acquire[6] ||
               n_acquired[7] < to_acquire[7])
          {
              readout_loop(&digitizer, n_acquired, histos);
          }
      } // end else

    digitizer.end_acquisition();
    std::string acquired_events_display("Acquired: ");
    for (unsigned int ch=0; ch<digitizer.get_nchannels(); ch++)
      {
        acquired_events_display += std::to_string(n_acquired[ch]);
        acquired_events_display += " events for channel ";
        acquired_events_display += std::to_string(ch);
        acquired_events_display += " \n"; 
      }
    INFO( acquired_events_display);
    output->cd();
    for (auto h : histos)
        {h->Write();}
    output->Write();
    return EXIT_SUCCESS;
}
