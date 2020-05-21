"""
The noisemodel, which is a prediction of x-ray resolution
for an arbitrary shaping time
"""
from dataclasses import dataclass
import numpy as np
import pylab as p
import os
import os.path

import HErmes as he
import hepbasestack as hep

from .utils import get_stripname
logger = hep.logger.get_logger(10)

########################################################################


@dataclass
class Constants:
    q     : float = 1.6e-19   # electron charge
    k     : float = 1.38e-23  # Boltzmann constant
    eps   : float = 3.6       # ionization energy of silicon, eV
    Rp    : float = 100e6     # parallel resistance of preamp, 100 MOhm
    gm    : float = 18e-3     # transconductance in FET, 18 ms
    Bita  : float = 1
    Fi    : float = 0.367     # noise form factor
    Fv    : float = 1.15      # noise form factor
    Fvf   : float = 0.226     # noise form factor
    pi    : float = 3.1415926
    Ctot  : float = 70e-12    #pF

    def Rs_denom(self, temp_cels):
        return self.Fv/self.Ctot/self.Ctot/(4*self.k*self.T(temp_cels))-self.Bita/self.gm;

    @property
    def Af_denom(self):
        return self.Ctot/self.Ctot/self.Fvf/2/self.pi 

    # FIXME: While not tecnnically a constant, 
    # we try to keep it as constant as possible
    # I know...    
    def T(self,temp_cels):
        return temp_cels+273

CONSTANTS = Constants()


########################################################################

def noise_model(xs, par0, par1, par2):
    """
    Noise model as to be fit to the resolution vs shaping times
    plot.
    
    Args:
        xs (ndarray)    : input data
        par0 (float)
        par1 (float)
        par2 (float)
    """
    # from root script 
    # sqrt([0]*x*1e-6+[1]/(x*1e-6)+[2])

    # mus -> s
    xs = 1e-6*xs
    result = np.sqrt((par0*xs) + (par1/xs) + (np.ones(len(xs))*par2))
    #u = 1/np.sqrt(xs)
    # transform into u space
    #result = np.sqrt(par0)*u**2 + np.sqrt(par1)* u + np.sqrt(par2)
    return result

########################################################################

def construct_error_belt(nm, xs=None):
    """ 
    Get the maximum and minimum values for the noisemodel

    Args:
        nm (HErmes.fitting.Model)   : the fitted noisemodel

    Keyword Args:
        xs (numpy.ndarray)          : x data. If none, take noisemodel.xs
    """
    minmaxpar0 = (nm.best_fit_params[0] + nm.errors['par00'],
                  nm.best_fit_params[0] - nm.errors['par00'])
    minmaxpar1 = (nm.best_fit_params[1] + nm.errors['par10'],
                  nm.best_fit_params[1] - nm.errors['par10'])
    minmaxpar2 = (nm.best_fit_params[2] + nm.errors['par10'],
                  nm.best_fit_params[2] - nm.errors['par10'])

    if xs is None:
        xs = nm.xs
    currentmaximum = noise_model(xs, minmaxpar0[1],\
                                     minmaxpar1[1],\
                                     minmaxpar2[1])
    currentminimum = noise_model(xs, minmaxpar0[0],\
                                     minmaxpar1[0],\
                                     minmaxpar2[0])
    for i in range(2):
        for j in range(2):
            for l in range(2):
                thisnoisemodel = noise_model(xs, minmaxpar0[i],\
                                                 minmaxpar1[i],\
                                                 minmaxpar2[i])
                currentminimum = np.minimum(thisnoisemodel, currentminimum)
                currentmaximum = np.maximum(thisnoisemodel, currentmaximum) 
                thisnoisemodel = noise_model(xs,  minmaxpar0[j],\
                                                  minmaxpar1[i],\
                                                  minmaxpar2[i]) 
                currentminimum = np.minimum(thisnoisemodel, currentminimum)
                currentmaximum = np.maximum(thisnoisemodel, currentmaximum) 
                thisnoisemodel = noise_model(xs, minmaxpar0[j],\
                                                 minmaxpar1[j],\
                                                 minmaxpar2[i]) 
                currentminimum = np.minimum(thisnoisemodel, currentminimum)
                currentmaximum = np.maximum(thisnoisemodel, currentmaximum) 
                thisnoisemodel = noise_model(xs, minmaxpar0[j],\
                                                 minmaxpar1[j],\
                                                 minmaxpar2[j]) 
                currentminimum = np.minimum(thisnoisemodel, currentminimum)
                currentmaximum = np.maximum(thisnoisemodel, currentmaximum) 
    return currentminimum, currentmaximum

########################################################################

def fit_noisemodel(xs, ys, ys_err,  ch, detid,\
                   plotdir='.', fig=None,\
                   use_minuit=True,\
                   debug_minuit=False):
    """
    Perform the fit of the xray resolutions over peaking time
    with the noisemodel

    Args:
        xs (ndarray)         : peaking times (microseconds)
        ys (ndarray)         : x-ray resolutions (fwhm)
        ys_err (ndarray)     : fwhm associated fit errors
        ch (int)             : digitizer channel/detector strip
        detid (int)          : detector id (needed for the filename to save the plot)

    Keyword Args:
        fig (Figure)         : use a pre-exisiting figure instance. If None, create new
        plotdir (str)        : directory to save the plot in
        use_minuit (bool)    : Use minuit for the minimization (recommended) 
        debug_minuit (bool)  : pass this parameter to the model. It attaches the 
                               iminuit instance to the model, so it is accessible for 
                               later debugging
    Returns:
        None
    """
    # channel noise model fit
    logger.info(f'Performing noise model fit for channel {ch}')
    noisemodel = he.fitting.Model(noise_model)
    noisemodel.startparams = (5e5, 1e-5,1)
    noisemodel.add_data(ys, xs=xs/1000,\
                        data_errs=ys_err,
                        create_distribution=False)
    noisemodel.fit_to_data(use_minuit=use_minuit,\
                           debug_minuit=debug_minuit,\
                           errors=(1,1,1),\
                           limits=((1e4,1e7), \
                                   (1e-7,1e-4), \
                                   (0,100)))
    logger.info(f'Noisemodel fitted, best fit pars: {noisemodel.best_fit_params}')
    params = extract_parameters_from_noisemodel(noisemodel)
    logger.info(f'Extracted {params} from noisemodel')
    #print (params['Ileak'])
    # create a textbox with some output
    infotext  = r"\begin{tabular}{lll}"
    infotext += r"noisemodel fit:&&\\ "
    infotext += r"p0 & {:4.2e} &$\pm$ {:4.2e}\\".format(params['p0'], params['ep0'])
    infotext += r"p1 & {:4.2e} &$\pm$ {:4.2e}\\".format(params['p1'], params['ep1'])
    infotext += r"p2 & {:4.2e} &$\pm$ {:4.2e}\\".format(params['p2']
                                                       , params['ep2'])
    infotext += r"$I_l$ & {:4.2e} &$\pm$ {:4.2e} [nA]\\".format(params['Ileak'], params['eIleak'])
    infotext += r"$A_f$ & {:4.2e} &$\pm$ {:4.2e} $x1e13$ [$V^2$]\\".format(params['Af'], params['eAf'])
    infotext += r"$R_S$ & {:4.2e} &$\pm$ {:4.2e} [$\Omega$]\\".format(params['Rs'], params['eRs'])
    #infotext += r"$C_{tot}$ & {:4.2f} [pF]\\ ".format(peakmod.best_fit_params[4], errdict['fwhm1'])
    infotext += r"$\chi^2/ndf$ &{:4.2f} & \\ ".format(noisemodel.chi2_ndf)
    infotext += r"\end{tabular}"

    if fig is None:
        noisemodel_fig = p.figure()
    else:
        noisemodel_fig = fig
    ax = noisemodel_fig.gca()
    xs_for_plt = np.linspace(min(noisemodel.xs), max(noisemodel.xs), 10000)
    ax.plot(xs_for_plt, noise_model(xs_for_plt, *noisemodel.best_fit_params),\
            color='r', lw=1.2, zorder=1)
    minmodel, maxmodel = construct_error_belt(noisemodel, xs=xs_for_plt)
    ax.fill_between(xs_for_plt, minmodel, y2=maxmodel,\
                    color='r', alpha=0.4, zorder=1)
    ax.errorbar(xs/1000, ys, yerr=ys_err, \
                marker='.', mfc='k', mec='k', ms=1.2, \
                #fmt='none',\
                fmt='.', \
                ecolor='k',\
                lw=1.2,\
                zorder=2,\
               )
    ax = hep.visual.adjust_minor_ticks(ax, which='both')
    ax.set_xscale('log')
    ax.set_yscale('log')
    ax.set_ylim(bottom=1)
    ax.grid(which='minor', color='gray', alpha=0.7)
    title = f'det{detid}-{get_stripname(ch)}-nmfit'
    ax.set_title(title, loc='right')
    ax.set_xlabel('peaking time [$\mu$s]')
    ax.set_ylabel('xray res. (FWHM) [keV]')
    ax.text(0.3, 0.3, infotext, \
            horizontalalignment='center', \
            verticalalignment='center', \
            transform=ax.transAxes, \
            size='xx-small', \
            bbox=dict(facecolor='white', alpha=0.7, edgecolor=None), \
            )
    pngfilename = os.path.join(plotdir, title + '.png')
    noisemodel_fig.savefig(pngfilename)
    logger.info(f'Saved file {pngfilename}')
    return noisemodel_fig, noisemodel

########################################################################

def extract_parameters_from_noisemodel(noisemodel):
    """
    After fitting the noisemodel, get the detector
    relevant parameters out of the fit parameters
    Multiply with the orders of magnitude to get reasonable units.

    Args:
        noisemodel (HErmes.fiting.Model) : noisemodel after fitting

    Returns (dict)                       : the relevant paramters
    """

    factor = (2.355*CONSTANTS.eps*1e-3/CONSTANTS.q)*(2.355*CONSTANTS.eps*1e-3/CONSTANTS.q)

    # this is necessary in case we did not use
    # minuit for the fitting, then we do have 
    # a different structure for the error dict
    print (noisemodel.errors)
    if isinstance(noisemodel.errors, list):
        tmpdict = dict()
        tmpdict['par00'] = noisemodel.errors[0]
        tmpdict['par10'] = noisemodel.errors[1]
        tmpdict['par20'] = noisemodel.errors[2]
        noisemodel.errors = tmpdict

    p0  = noisemodel.best_fit_params[0]/factor;
    ep0 = noisemodel.errors['par00']/factor;
    p1  = noisemodel.best_fit_params[1]/factor;
    ep1 = noisemodel.errors['par10']/factor;
    p2  = noisemodel.best_fit_params[2]/factor;
    ep2 = noisemodel.errors['par20']/factor;
    
    Ileak = (p0/CONSTANTS.Fi-4*CONSTANTS.k*CONSTANTS.T(-37)/CONSTANTS.Rp)/2/CONSTANTS.q;  # A
    eIleak = ep0/p0*Ileak;  # A
    Rs = p1/CONSTANTS.Rs_denom(-37) # temperature -37
    if Rs < 0 : Rs = 0
    eRs = ep1/p1*Rs  # Ohm

    Af = p2/CONSTANTS.Ctot/CONSTANTS.Ctot/CONSTANTS.Fvf/2/CONSTANTS.pi 
    eAf = ep2/p2*Af


    result = {'p0'     : p0*factor,\
              'ep0'    : ep0*factor,\
              'p1'     : p1*factor,\
              'ep1'    : ep1*factor,\
              'p2'     : p2*factor,\
              'ep2'    : ep2*factor,\
              'Ileak'  : Ileak*1e9,\
              'eIleak' : eIleak*1e9,\
              'Rs'     : Rs,\
              'eRs'    : eRs,\
              'Af'     : Af*1e13,\
              'eAf'    : eAf*1e13\
             }
    return result

