#ifndef TRAPEZOIDAL_SHAPER_H_INCLUDED
#define TRAPEZOIDAL_SHAPER_H_INCLUDED

#include <vector>
#include <stdint.h>

/**
 * Following up on Mengjiaos original implementation from 
 * Spring 2020 (and that basically follows up on discussions 
 * with Alex). Basically slide a Trapezoidal window over the
 * waveform and check for the difference in the rising and the 
 * falling part. 
 * Implemented here since even the vectorized version in numpy
 * seems to be a bit sluggish
 */
class TrapezoidalFilter{

  public:
    /**
     *
     * @param : ptime        - peaking/shaping time in ns
     * @param : flat         - flat top area
     * @param : recordlength - number of samples in the waveform  
     */
    TrapezoidalFilter(int ptime, int flat = 1000, int recordlength = 50000); 
    uint32_t shape_it(std::vector<int16_t> const &waveform) const;


  public:
    int ptime;
    int flat;
    int recordlength;
};

#endif



