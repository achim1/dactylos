"""
Specific to the GAPS experiment
Prediction of the energy resolution with the custom ASIC
from the calibration with discrete preamps.
"""

import numpy as np
import hepbasestack as hep
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
    # FIXME logging
    if A_f < 0.74*1e-13: # this value comes from Mengjiao
        print (f"WARN: A_f too small {A_f}, will use 0.74!")
        A_f = 0.74*1e-13
    tau = 1e-6*tau
    C_2 = C**2
    A =  2*CONSTANTS.q*(I_L + ACONSTANTS.I_eff)*tau*ACONSTANTS.F_i
    B =  ACONSTANTS.S_w*(C_2)*ACONSTANTS.F_nu*(1/tau)
    C = 2*np.pi*A_f*ACONSTANTS.F_nuf*(C_2)*np.ones(len(tau))
    enc2 = A+B+C
    #print ((A[0],B[0], C[0], 'A,B,C'))
    return np.sqrt(enc2)*2.355*CONSTANTS.eps*(1/CONSTANTS.q)*1e-3
    #return enc2

####################################################################################

def asic_projection_plot(tau, asic,\
                         stripname,\
                         detid,\
                         fig=None,\
                         loglog=False):
    """
    Produce a plot with asic prediction vs shaping time

    Args:
        tau      (numpy.ndarray) : shaping times, x-values
        asic     (numpy.ndarray) : asic prediction, y-values
        stripname          (str) : strip identifier A-H on GAPS Si(Li) detector
        detid              (int) : detector id.
    Keyword Args:
        fig      (matplotlib.figure.Figure) : a pre-initialized figure instance
                                              to plot the data in. If None, new
                                              one gets created.
        loglog                        (bool): create a log-log plot(default False)

    """

    title = f'det {detid}'
    label = f'{stripname}'
    if fig is None:
        fig = p.figure(dpi=120,\
                       figsize=hep.layout.FIGSIZE_A4_LANDSCAPE_HALF_HEIGHT)

    ax = fig.gca()
    #ax.plot(nm.xs, asic, label=label, lw=1.2)
    ax.plot(tau, asic, label=label, lw=1.2)
    ax.set_xlim(left=0.49, right=2)
    ax.set_ylim(top=6)
    ax.set_title(title, loc='right')
    # ax.legend()
    # hep.visual.adjust_minor_ticks(ax, which='both')
    ax.grid(which='minor', color='gray', alpha=0.7)
    if loglog:
        ax.set_xscale('log')
        ax.set_yscale('log')
    ax.set_ylabel('proj. ASIC FWHM [keV]')
    ax.set_xlabel(r'$\tau$ [$\mu$s]')
    return fig

####################################################################################


