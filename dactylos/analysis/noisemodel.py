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

import hjson

from .utils import get_stripname
logger = hep.logger.get_logger(10)

########################################################################

@dataclass
class Constants:
    q      : float = 1.6e-19   # electron charge
    k      : float = 1.38e-23  # Boltzmann constant
    eps    : float = 3.6       # ionization energy of silicon, eV
    Rp     : float = 100e6     # parallel resistance of preamp, 100 MOhm
    gm     : float = 18e-3     # transconductance in FET, 18 ms
    Bita   : float = 1
    Fi     : float = 0.367     # noise form factor
    Fv     : float = 1.15      # noise form factor
    Fvf    : float = 0.226     # noise form factor
    Ctot   : float = 70e-12    #pF

    Fi_g4  : float = 0.45      # noise form factor for gauss order 4
    Fv_g4  : float = 1.02      # noise form factor for gauss order 4
    Fvf_g4 : float = 0.52      # noise form factor for gauss order 4
    
    Fi_tr  : float = 0.83      # noise form factor for trapezoid
    Fv_tr  : float = 1         # noise form factor for trapezoid 
    Fvf_tr : float = 0.69      # noise form factor for trapezoid

    def Rs_denom(self, temp_cels):
        return self.Fv/self.Ctot/self.Ctot/(4*self.k*self.T(temp_cels))-self.Bita/self.gm;

    @property
    def Af_denom(self):
        return self.Ctot/self.Ctot/self.Fvf/2/np.pi 

    # FIXME: While not tecnnically a constant, 
    # we try to keep it as constant as possible
    # I know...    
    def T(self,temp_cels):
        return temp_cels+273

CONSTANTS = Constants()

########################################################################

def serialize_noisemodel(noisemodel,\
                         filename):
    """
    Save the noisemodel to a file, serialize via json

    Args:
        noisemodel (HErmes.fitting.model.Model) : the fitted noisemodel
        filename   (str)                        : full path to file
    """
    nm_data = dict()
    nm_data['I_L']      = noisemodel.I_L
    nm_data['A_f']      = noisemodel.A_f
    nm_data['R_S']      = noisemodel.R_S
    nm_data['chi2/ndf'] = noisemodel.chi2_ndf
    nm_data['npoint']   = len(noisemodel.data)
    nm_data['detid']    = noisemodel.detid     
    nm_data['strip']    = noisemodel.stripname 
    logger.info(f'Serializing noisemodel to {filename}')
    hjson.dump(nm_data, open(filename,'w'))
    return None

########################################################################

def extract_parameters_from_noisemodel(noisemodel,\
                                       method='trapezoid'):
    """
    After fitting the noisemodel, get the detector
    relevant parameters out of the fit parameters
    Multiply with the orders of magnitude to get reasonable units.

    Args:
        noisemodel (HErmes.fiting.Model) : noisemodel after fitting

    Keyword Args:
        method     (str)                 : this describes the functional form of the 
                                           used shaping function. Which can be 
                                           either 'trapezoid' or 'gauss'
                                           This affects the weighting factors for Il, Af and R.
    Returns (dict)                       : the relevant paramters
    """

    factor = (2.355*CONSTANTS.eps*1e-3/CONSTANTS.q)*(2.355*CONSTANTS.eps*1e-3/CONSTANTS.q)

    # use the weighting factors for either trapezoid or Gauss 4th order
    if method == 'trapezoid':
        Fi   = CONSTANTS.Fi  
        Fv   = CONSTANTS.Fv
        Fvf  = CONSTANTS.Fvf
    elif method == 'gauss':
        Fi   = CONSTANTS.Fi_g4
        Fv   = CONSTANTS.Fv_g4
        Fvf  = CONSTANTS.Fvf_g4
    else:
        raise ValueError(f"Can't understand {method}. Has to be either 'gauss' or 'trapezoid'")

    # this is necessary in case we did not use
    # minuit for the fitting, then we do have 
    # a different structure for the error dict
    if isinstance(noisemodel.errors, list):
        tmpdict = dict()
        tmpdict['par00'] = noisemodel.errors[0]
        tmpdict['par10'] = noisemodel.errors[1]
        tmpdict['par20'] = noisemodel.errors[2]
        noisemodel.errors = tmpdict


    p0  = noisemodel.best_fit_params[0]/factor
    ep0 = noisemodel.errors['par00']/factor
    p1  = noisemodel.best_fit_params[1]/factor
    ep1 = noisemodel.errors['par10']/factor
    p2  = noisemodel.best_fit_params[2]/factor
    ep2 = noisemodel.errors['par20']/factor
    
    Ileak = (p0/Fi-4*CONSTANTS.k*CONSTANTS.T(-37)/CONSTANTS.Rp)/2/CONSTANTS.q  # A
    eIleak = ep0/p0*Ileak  # A
    Rs = (p1 /Fv / CONSTANTS.Ctot / CONSTANTS.Ctot / (
                4 * CONSTANTS.k * CONSTANTS.T(-37))) - (CONSTANTS.Bita / CONSTANTS.gm)

    Rs_const = Fv*CONSTANTS.Ctot*CONSTANTS.Ctot*(4*CONSTANTS.k*CONSTANTS.T(-37))
    Rs_const_diff =  -CONSTANTS.Bita/CONSTANTS.gm
    if Rs < 0:
        logger.warn(f'Rs: {Rs}')
        logger.warn(f'p1/Rs_const : {p1/Rs_const - Rs_const_diff}')
        logger.warn(f'.. > 0 {p1/Rs_const > 0}')
        logger.warn(f'Rs_const : {Rs_const > 0}')
        logger.warn(f'p1 : {p1}')
        logger.warn(f'{p1 > 0}')
        logger.warn(f'{p1/factor}')
        logger.warn(f'{CONSTANTS.Fv/CONSTANTS.Ctot/CONSTANTS.Ctot/(4*CONSTANTS.k*CONSTANTS.T(-37))-CONSTANTS.Bita/CONSTANTS.gm}')
        logger.warn(f'{p1 > 0}')
        logger.warn('Rs is < 0!')
    eRs = ep1/p1*Rs  # Ohm

    CTOT  = np.sqrt(p1/Fv/(4*CONSTANTS.k*CONSTANTS.T(-37))/(Rs+CONSTANTS.Bita/CONSTANTS.gm));
    logger.warn(f'Calculated {CTOT}')

    Af = p2/CONSTANTS.Ctot/CONSTANTS.Ctot/Fvf/2/np.pi


    #Af = p2/CTOT/CTOT/CONSTANTS.Fvf#/2/CONSTANTS.pi
    eAf = ep2/p2*Af

    result = {'p0'     : p0   *factor,\
              'ep0'    : ep0  *factor,\
              'p1'     : p1   *factor,\
              'ep1'    : ep1  *factor,\
              'p2'     : p2   *factor,\
              'ep2'    : ep2  *factor,\
              'Ileak'  : Ileak*1e9,\
              'eIleak' : eIleak*1e9,\
              'Rs'     : Rs,\
              'eRs'    : eRs,\
              'Af'     : Af*1e13,\
              'eAf'    : eAf*1e13\
             }
    return result

########################################################################

def preamp_noise_model(tau, IL, Rs, Af):
    """
    Direct representation of the ENC for the preamps

    Aregs:
        tau (ndarray)  : shaping times
        IL (float)    : leakage current
        Rs (float)    : serial resistance
        Af (float)    :
    """
    tau = 1e-6 * tau
    #A_f = A_f*1e13
    #I_L = I_L*1e9
    GAMMA = 1 # Field
    A = CONSTANTS.Fi*(CONSTANTS.q*2*IL + (4*CONSTANTS.k*CONSTANTS.T(-37)*(1/CONSTANTS.Rp)))
    B = 4*CONSTANTS.k*CONSTANTS.T(-37)*CONSTANTS.Fv*CONSTANTS.Ctot*CONSTANTS.Ctot*\
        (Rs + (GAMMA/CONSTANTS.gm))
    C = Af*CONSTANTS.Ctot*CONSTANTS.Ctot*CONSTANTS.Fvf
    #print ((A,B,C, 'Refined nm A,B,C'))
    result = A*tau + B/tau + C*np.ones(len(tau))
    result = np.sqrt(result)*2.355*CONSTANTS.eps*(1/CONSTANTS.q)*1e-3
    return result

########################################################################

def noise_model(xs, par0, par1, par2):
    """
    Noise model as to be fit to the resolution vs shaping times
    plot.
    
    Args:
        xs (ndarray)    : input data
        par0 (float)    : substition, will get mapped to I_L
        par1 (float)    : substitution, will get mapped to R_S
        par2 (float)    : substitution, will get mapped to A_f
    """
    # from root script 
    # sqrt([0]*x*1e-6+[1]/(x*1e-6)+[2])

    # mus -> s
    xs = 1e-6*xs
    result = np.sqrt((par0*xs) + (par1/xs) + (np.ones(len(xs))*par2))
    #result = (par0 * xs) + (par1 / xs) + (np.ones(len(xs)) * par2)
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
    try:
        eI_L = nm.errors['par00']
    except:
        eI_L = nm.errors['IL0']
    try:
        eR_s = nm.errors['par10']
    except:
        eR_s = nm.errors['Rs0']
    try:
        eA_f = nm.errors['par20']
    except:
        eA_f = nm.errors['Af0']

    minpar0 = nm.best_fit_params[0] - eI_L
    if minpar0 < 0: minpar0 = 0
    minpar1 = nm.best_fit_params[1] - eR_s
    if minpar1 < 0: minpar1 = 0
    minpar2 = nm.best_fit_params[2] - eA_f
    if minpar2 < 0: minpar2 = 0
    minmaxpar0 = (nm.best_fit_params[0] + eI_L,
                  minpar0)
    minmaxpar1 = (nm.best_fit_params[1] + eR_s,
                  minpar1)
    minmaxpar2 = (nm.best_fit_params[2] + eA_f,
                  minpar2)

    if xs is None:
        xs = nm.xs
    currentmaximum = nm(xs, minmaxpar0[1],\
                            minmaxpar1[1],\
                            minmaxpar2[1])
    currentminimum = nm(xs, minmaxpar0[0],\
                            minmaxpar1[0],\
                            minmaxpar2[0])
    for i in range(2):
        for j in range(2):
            for l in range(2):
                thisnoisemodel = nm(xs, minmaxpar0[i],\
                                                 minmaxpar1[i],\
                                                 minmaxpar2[i])
                currentminimum = np.minimum(thisnoisemodel, currentminimum)
                currentmaximum = np.maximum(thisnoisemodel, currentmaximum) 
                thisnoisemodel = nm(xs,  minmaxpar0[j],\
                                                  minmaxpar1[i],\
                                                  minmaxpar2[i]) 
                currentminimum = np.minimum(thisnoisemodel, currentminimum)
                currentmaximum = np.maximum(thisnoisemodel, currentmaximum) 
                thisnoisemodel = nm(xs, minmaxpar0[j],\
                                                 minmaxpar1[j],\
                                                 minmaxpar2[i]) 
                currentminimum = np.minimum(thisnoisemodel, currentminimum)
                currentmaximum = np.maximum(thisnoisemodel, currentmaximum) 
                thisnoisemodel = nm(xs, minmaxpar0[j],\
                                                 minmaxpar1[j],\
                                                 minmaxpar2[j]) 
                currentminimum = np.minimum(thisnoisemodel, currentminimum)
                currentmaximum = np.maximum(thisnoisemodel, currentmaximum) 
    return currentminimum, currentmaximum

########################################################################

def fit_noisemodel(xs, ys, ys_err,  ch, detid,\
                   plotdir='.', fig=None,\
                   use_minuit=True,\
                   use_direct_model=False,\
                   debug_minuit=False,
                   method='trapezoid'):
    """
    Perform the fit of the xray resolutions over peaking time
    with the noisemodel

    Args:
        xs           (ndarray)   : peaking times (microseconds)
        ys           (ndarray)   : x-ray resolutions (fwhm)
        ys_err       (ndarray)   : fwhm associated fit errors
        ch               (int)   : digitizer channel/detector strip
        detid            (int)   : detector id (needed for the filename to save the plot)

    Keyword Args:
        fig (Figure)             : use a pre-exisiting figure instance. If None, create new
        plotdir (str)            : directory to save the plot in
        use_minuit       (bool)  : Use minuit for the minimization (recommended)
        use_direct_model (bool)  : use a slightly different implempentation of the noise
                                   model. The difference is that the 3 physics parameters
                                   I_L, R_S and A_f are fitted directly.
        debug_minuit     (bool)  : pass this parameter to the model. It attaches the
                                   iminuit instance to the model, so it is accessible for
                                   later debugging
        fig (matplotlib.Figure)  : A predefined figure install to use
        method            (str)  : Tell the noisemodel what method was used to obtain the 
                                   data, either a trapezoidal shaper or a gaussian. 
                                   This affects the weights for the physics parameters. 
    Returns:
        tuple (matplotlib.figure, model)               
    """
    # channel noise model fit
    logger.info(f'Performing noise model fit for channel {ch}')
    if use_direct_model:
        logger.info(f'Using direct model, fitting I_L, R_S, A_f...')
        noisemodel = he.fitting.Model(preamp_noise_model)
        noisemodel.startparams = (2e-9,1, 8e-13)
    else:
        noisemodel = he.fitting.Model(noise_model)
        noisemodel.startparams = (5e5, 1e-5,1)

    noisemodel.add_data(ys, xs=xs/1000,\
                        data_errs=ys_err,
                        create_distribution=False)
    if use_direct_model:
        noisemodel.fit_to_data(use_minuit=use_minuit, \
                               debug_minuit=debug_minuit, \
                               errors=(1, 1, 1), \
                               bounds=([1e-10,0,1e-14],[1e-8,20,1e-12]),\
                               limits=((1e-10, 1e-8),\
                                       (0, 20),\
                                       (1e-14, 1e-12))
                                      )


    else:
        noisemodel.fit_to_data(use_minuit=use_minuit,\
                           debug_minuit=debug_minuit,\
                           errors=(1,1,1),\
                           bounds =([1e4,1e-7,0],[1e7,1e-4,100]),\
                           limits=((1e4,1e7), \
                                   #(1e-7,1e-4), \
                                   (1.1450365642091253e-05,1e-4),\
                                   (0,100)))

    logger.info(f'Noisemodel fitted, best fit pars: {noisemodel.best_fit_params}')
    if use_direct_model:
        if isinstance(noisemodel.errors, list):
            tmpdict = dict()
            tmpdict['par00'] = noisemodel.errors[0]
            tmpdict['par10'] = noisemodel.errors[1]
            tmpdict['par20'] = noisemodel.errors[2]
            noisemodel.errors = tmpdict
        I_L = noisemodel.best_fit_params[0]
        R_S = noisemodel.best_fit_params[1]
        A_f = noisemodel.best_fit_params[2]
        print (noisemodel.errors)
        try:
            eI_L = noisemodel.errors['par00']
        except:
            eI_L = noisemodel.errors['IL0']
        try:
            eR_S = noisemodel.errors['par10']
        except:
            eR_S = noisemodel.errors['Rs0']
        try:
            eA_f = noisemodel.errors['par20']
        except:
            eA_f = noisemodel.errors['Af0']
        #
        #eR_S = noisemodel.errors['R_s0']
        #eA_f = noisemodel.errors['A_f0']
        infotext = r"\begin{tabular}{lll}"
        infotext += r"noisemodel fit:&&\\ "
        infotext += r"$I_l$ & {:4.2e} &$\pm$ {:4.2e} [nA]\\".format(I_L, eI_L)
        infotext += r"$A_f$ & {:4.2e} &$\pm$ {:4.2e} [$V^2$]\\".format(A_f, eA_f)
        infotext += r"$R_S$ & {:4.2e} &$\pm$ {:4.2e} [$\Omega$]\\".format(R_S, eR_S)
        infotext += r"$\chi^2/ndf$ &{:4.2f} & \\ ".format(noisemodel.chi2_ndf)
        infotext += r"\end{tabular}"
    else:
        params = extract_parameters_from_noisemodel(noisemodel, method=method)
        logger.info(f'Extracted {params} from noisemodel')
        logger.debug(f'..I_L {params["Ileak"]}')
        logger.debug(f'..A_f {params["Af"]}')
        logger.debug(f'..R_s {params["Rs"]}')
        I_L = params["Ileak"]
        A_f = params["Af"]
        R_S = params["Rs"]

        #print (params['Ileak'])
        # create a textbox with some output
        infotext  = r"\begin{tabular}{lll}"
        infotext += r"noisemodel fit:&&\\ "
        infotext += r"p0 & {:4.2e} &$\pm$ {:4.2e}\\".format(params['p0'], params['ep0'])
        infotext += r"p1 & {:4.2e} &$\pm$ {:4.2e}\\".format(params['p1'], params['ep1'])
        infotext += r"p2 & {:4.2e} &$\pm$ {:4.2e}\\".format(params['p2'], params['ep2'])
        infotext += r"$I_l$ & {:4.2e} &$\pm$ {:4.2e} [nA]\\".format(params['Ileak'], params['eIleak'])
        infotext += r"$A_f$ & {:4.2e} &$\pm$ {:4.2e} $\times 1e-13$ [$V^2$]\\".format(params['Af'], params['eAf'])
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
    ax.plot(xs_for_plt, noisemodel(xs_for_plt, *noisemodel.best_fit_params),\
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
    title = f'det{detid}-{get_stripname(ch)}'
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
    pngfilename = os.path.join(plotdir, title + '-nmfit.png')
    noisemodel_file = os.path.join(plotdir, title + '-nmfit.dat') 
    noisemodel_fig.savefig(pngfilename)
    logger.warn(f'Saved file {pngfilename}')
    # attach some metainformation
    noisemodel.I_L = I_L
    noisemodel.A_f = A_f
    noisemodel.R_S = R_S
    noisemodel.detid     = detid
    noisemodel.stripname = get_stripname(ch)
    serialize_noisemodel(noisemodel,\
                         noisemodel_file)
    return noisemodel_fig, noisemodel

########################################################################

def enc2_trapezoid(xs, par0, par1, par2):
    """
    Noise model as to be fit to the resolution vs shaping times
    plot.

    Args:
        xs (ndarray)    : input data
        par0 (float)
        par1 (float)
        par2 (float)
    """
    raise NotImplementedError
    # weighting coefficients
    Aw_1 = 2  # series white
    Aw_2 = 1.67  # series parallel
    Aw_3 = 1.37  # series 1/f

    # mus -> s
    xs = 1e-6 * xs
    result = np.sqrt((par0 * xs) + (par1 / xs) + (np.ones(len(xs)) * par2))
    # fit C_in2, A_f and I_0 directly
    # coefficients
    a_0 = 0.5

    # u = 1/np.sqrt(xs)
    # transform into u space
    # result = np.sqrt(par0)*u**2 + np.sqrt(par1)* u + np.sqrt(par2)
    return result


########################################################################

def enc2_semigauss4(xs, par0, par1, par2):
    """
    Noise model as to be fit to the resolution vs shaping times
    plot.

    Args:
        xs (ndarray)    : input data
        par0 (float)
        par1 (float)
        par2 (float)
    """
    raise NotImplementedError
    # from root script
    # sqrt([0]*x*1e-6+[1]/(x*1e-6)+[2])

    # mus -> s
    xs = 1e-6 * xs
    result = np.sqrt((par0 * xs) + (par1 / xs) + (np.ones(len(xs)) * par2))
    # u = 1/np.sqrt(xs)
    # transform into u space
    # result = np.sqrt(par0)*u**2 + np.sqrt(par1)* u + np.sqrt(par2)
    return result

