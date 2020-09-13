

from .gauss_shaper import GaussShaper

USE_CXX_EXTENSION=True

if USE_CXX_EXTENSION:
    try:
        #from _trapezoidal_shaper import TrapezoidalFilter
        from dactylos._pyCaenN6725 import TrapezoidalFilter
    except ImportError as e:
        print (f"WARNING, can not import c++  extension for TrapezoidalFilter, Trapezoidal filter will be SLOW!. Exception {e}")
        from .trapezoidal_shaper import TrapezoidalFilter
else:
    from .trapezoidal_shaper import TrapezoidalFilter
    print (f"WARNING, not using c++  extension for TrapezoidalFilter, Trapezoidal filter will be SLOW!.")

