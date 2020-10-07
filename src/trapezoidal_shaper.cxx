#include <iostream>

#include "trapezoidal_shaper.h"

TrapezoidalFilter::TrapezoidalFilter(int ptime, int flat, int recordlength) : 
                                                     ptime(ptime),
                                                     flat(flat),
                                                     recordlength(recordlength) {
};


uint32_t TrapezoidalFilter::shape_it(const std::vector<int16_t> &waveform) const {
    
  int32_t  sum_rise(0), sum_down(0);
  float amp_filter(0), energy(0);

  int16_t nramp = ptime/4; //4ns per sample
  int16_t nflat = flat/4;
  int16_t ntot = 2*nramp+nflat;
  int16_t nrampnflat = nramp + nflat;

  for(int i=ntot;i<waveform.size();i++) {
    amp_filter = 0;
    for(int j=0;j<nramp;j++)
      {
        amp_filter += ((float)(waveform[i-j] - waveform[i-j-nrampnflat]))*(1/(float)nramp);
        //sum_down -= waveform[i-j-nrampnflat]*(1/nramp);
      }
    if(amp_filter > energy)
      {
        energy = amp_filter;  //energy calculation, need to be improved
      }
  }
  return energy;
};


