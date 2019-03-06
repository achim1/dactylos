
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

/***************************************************************/

typedef std::vector<TH1I*> HistogramSeries;

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
    parser.AddOption<int>("n-events-ch1", "Acquire N events for channel 1", 1000); 
    parser.AddOption<int>("n-events-ch2", "Acquire N events for channel 2", 1000); 
    parser.AddOption<int>("n-events-ch3", "Acquire N events for channel 3", 1000); 
    parser.AddOption<int>("n-events-ch4", "Acquire N events for channel 4", 1000); 
    parser.AddOption<int>("n-events-ch5", "Acquire N events for channel 5", 1000); 
    parser.AddOption<int>("n-events-ch6", "Acquire N events for channel 6", 1000); 
    parser.AddOption<int>("n-events-ch7", "Acquire N events for channel 7", 1000); 
    parser.AddOption<int>("n-events-ch8", "Acquire N events for channel 8", 1000); 
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
    CaenN6725 digitizer;

    DigitizerParams_t thisParams = digitizer.InitializeDigitizerForPulseGenerator(parser);
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

    digitizer.ProgramDigitizer(handle, thisParams);

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
    std::vector<int> to_acquire({});
    to_acquire.push_back(parser.GetOption<int>("n-events-ch1"));
    to_acquire.push_back(parser.GetOption<int>("n-events-ch2"));
    to_acquire.push_back(parser.GetOption<int>("n-events-ch3"));
    to_acquire.push_back(parser.GetOption<int>("n-events-ch4"));
    to_acquire.push_back(parser.GetOption<int>("n-events-ch5"));
    to_acquire.push_back(parser.GetOption<int>("n-events-ch6"));
    to_acquire.push_back(parser.GetOption<int>("n-events-ch7"));
    to_acquire.push_back(parser.GetOption<int>("n-events-ch8"));
    int n_events(0);
    int tmpchannel=0;
    for (int n : to_acquire)
         {
            INFO("Will acquire " << n << "events for channel " << tmpchannel);
            n_events += n;
            tmpchannel++;
         }
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
