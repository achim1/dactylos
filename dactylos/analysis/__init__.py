"""
Analysis tools for data taken with dactylos.CaenN6725 digitizer software
"""

import numpy as _np
import pylab as _p
import hepbasestack as _hep

from .file_inspector import inspect_file
from .noisemodel import fit_noisemodel
from .utils import get_stripname

##########################################################

def fit_all_strips(data, detector_id,\
                   use_direct_model=False,\
                   use_minuit=False,\
                   debug_minuit=False,\
                   method='mc2',\
                   plotdir='.'):
    """
    Create the noisemodel fits for all strips for a certain detector

    Args:
        data ()                  : data obtained by reading in a summary file
        detector_id        (int) : unique detector identifier #
    Keyword Args:
        use_direct_model  (bool) : use a different way of fitting the noisemodel.
                                   Fit the noiseparameters directly
        use_minuit        (bool) : use iminuit for the fit.
                                   Alternative is scipy.optimize.lseastsq
        debug_minuit      (bool) : attach more informations to the noisemodel which 
                                   can help to resolve fitting isues
        method             (str) : the method which was used to create the data in the
                                   summaryfile
        plotdir            (str) : save plots in this directory
    """

    nmodels = []
    for i,strip in enumerate([f'{get_stripname(k)}' for k in range(8)]):
        ptime    = _np.asarray([k.ptime for k in data if k.strip == strip])
        fwhm     = _np.asarray([k.fwhm  for k in data if k.strip == strip])
        fwhm_err = _np.asarray([k.fwhme for k in data if k.strip == strip])
        fit_method = 'trapezoid'
        if data[0].mtype == 'WF':
            fit_method = 'gauss'
        try:
            fig, model =    fit_noisemodel(ptime, fwhm, fwhm_err,\
                                           i, detector_id, plotdir=plotdir,\
                                           use_minuit=use_minuit,
                                           use_direct_model=use_direct_model,\
                                           debug_minuit=debug_minuit,
                                           fig=_p.figure(figsize=\
                                                    _hep.layout.FIGSIZE_A4_LANDSCAPE,\
                                                    dpi=120),\
                                           method=fit_method
                                          )

        except Exception as e:
            print (f'Can not fit strip {i}, exception {e}')
            continue
        #fig.savefig(os.path.join(plotdir,f'nm-{detector_id}-{strip}-{method}.png'))
        nmodels.append(model)
        #break
    return fig, nmodels

