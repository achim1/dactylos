

from .gauss_shaper import GaussShaper

WAVEFORMTYPE='ARRAY'
try:
    from _trapezoidal_shaper import TrapezoidalFilter
    WAVEFORMTYPE='LIST'
except ImportError:
    print ("WARNING, can not compile c++ extension for TrapezoidalFilter, Trapezoidal filter will be SLOW!")
    from .trapezoidal_shaper import TrapezoidalFilter
