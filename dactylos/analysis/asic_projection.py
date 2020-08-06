"""
Specific to the GAPS experiment
Prediction of the energy resolution with the custom ASIC
from the calibration with discrete preamps.
"""

import numpy as np
from dataclasses import dataclass
from .noisemodel import Constants



@dataclass
class ASICConstants : 
    S_w   : float = 0.54*1e-18 #V2/Hz
    A_f   : float = 7.3*1e-14 #V2
    I_eff : float = 2.5*1e-9 # A
    # RC CR2 coefficients
    F_i   : float = 0.64 
    F_nu  : float = 0.853
    F_nuf : float = 0.543


CONSTANTS  = Constants()
ACONSTANTS = ASICConstants()

def enc2_asic(tau, I_L,  A_f, C=70*1e-12):
    """
    ASIC prediction from fitted noisemodel
    values from the preamp measurement.

    Args:
        tau (iterable) : xs - the shaping/peaking time
        I_L (float)    : fitted leakage current from the preamp 
                         measurement
        A_f (float)    : 

    Keyword Args:
        C (float)      : fitted capacity**2 from the preamp
                         measurementi - looks like this is always
                         70 pF
        
    """
    tau = 1e-6*tau
    C_2 = C**2
    A =  2*CONSTANTS.q*(I_L + ACONSTANTS.I_eff)*tau*ACONSTANTS.F_i
    B =  ACONSTANTS.S_w*(C_2)*ACONSTANTS.F_nu*(1/tau)
    C = 2*np.pi*A_f*ACONSTANTS.F_nuf*(C_2)*np.ones(len(tau))
    enc2 = A+B+C
    print ((A[0],B[0], C[0], 'A,B,C'))
    return np.sqrt(enc2)*2.355*CONSTANTS.eps*(1/CONSTANTS.q)*1e-3
    #return enc2
