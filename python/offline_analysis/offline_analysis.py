#! /usr/bin/env python3 

import sys
import gc
import pylab as p
import tqdm
import numpy as np
import uproot as up
import argparse
import concurrent.futures
import scipy.integrate as integrate
import os

from scipy.signal import sosfilt

import matplotlib
#matplotlib.use('Agg')

import hepbasestack as hep                                   
import dashi as d
import HErmes as h                                                     
import HErmes.selection as v, HErmes.fitting as fit, HErmes.visual as vis
from HErmes.selection.cut import Cut
import matplotlib.colors as colors

import cmocean as cmo
CMAP = cmo.cm.solar

from shapers import shaper
SHAPINGTIMES = [0.5, 1, 2, 4, 5, 8, 12, 15, 20, 30]
SHAPINGTIMES = [0.1, 0.2,0.5, 1, 2, 4]

CUT = 4 #throw away everything which peaks at smaller than

d.visual()

hep.visual.set_style_present()  

palette = vis.colors.get_color_palette("dark")

def fwhm(sigma):
    return 2.355*sigma

def sample(k):
    return -0.5 + (k*(1/2**14))

def integrator(ys, xs):
    return integrate.simps(ys, xs)

def bins_to_volts(input, dynamic_range="05VPP"):
    if dynamic_range == "05VPP":
        output = np.array([sample(k) for k in input])
        return output
    else:
        raise ValueError

def baseline_correction(input, nsamples=2000):
    zero = (2**14)/2
    baseline = input[:nsamples].mean() - zero
    #print (f"Baseline calculation gives us {baseline}")
    return baseline,(input - baseline)

def do_shaping(tail, t, peaktime, order, decay_time, dt,\
               plot_waveforms=False,fig=None, batch_id=0, event_id=0):
        try:
            sos = shaper("gaussian",order,peaktime,dt=dt,pz=1./decay_time)
            y = sosfilt(sos,tail) 

        except Exception as e:
            print (e)
            return 0,fig
        save_plot = False
        if plot_waveforms:
            if fig is None:
               save_plot = True
               fig = p.figure()
               ax = fig.gca()
               ax.plot(1e6*t, 1e3*tail, lw=0.9, color=palette[0],alpha=0.7, label='waveform')
               ax.set_ylabel("voltage [mV]")
               ax.set_xlabel("time [$\mu$s]")
               #ax.set_ylabel("digitizer bin [14bit]")
               #ax.set_xlabel(r"record length [?] (might be $\propto$ ns)")
            else:
               ax = fig.gca()

            ax.plot(1e6*t, 1e3*y, lw=1.5, label=f'shaper output pt {peaktime} mic sec')
            #ax.legend()              
            if save_plot:
               fig.savefig(f"wf_{batch_id}_{event_id}_{peaktime}.png")
               p.close()
               fig= None

        return 1e3*max(y),  fig


def do_work(argpacket):
    batch_id, plot_waveforms, waveformdata = argpacket
    energies = dict([(k, []) for k,__ in enumerate(SHAPINGTIMES)])
    baselines = dict([(k, []) for k,__ in enumerate(SHAPINGTIMES)])
    charges = dict([(k, []) for k,__ in enumerate(SHAPINGTIMES)])
    #print (f"Found argpacket {batch_id}, {plot_waveforms}") 

    f = open(f"specialwaveforms_{batch_id}.dat", "w")
    for event_id,data in enumerate(waveformdata):
        baseline, tail = baseline_correction(data, nsamples=200)
        tail = bins_to_volts(tail)
        charge = integrator(tail, t)
        if plot_waveforms:
            fig = p.figure()
            ax = fig.gca()
            ax.plot(1e6*t, 1e3*tail, lw=0.9, color=palette[0],alpha=0.7, label='waveform')
            ax.set_ylabel("voltage [mV]")
            ax.set_xlabel("time [$\mu$s]")
        else:
            fig = None
        
        for k,ptime in enumerate(SHAPINGTIMES):
            ptime *= 1e-6
            
            # shaping    
            energy,  fig = do_shaping(tail, t, ptime, order, decay_time, dt,\
                                      plot_waveforms=plot_waveforms,
                                      batch_id=batch_id,
                                      event_id=event_id,
                                      fig=fig)
            #print (energy) 
            #if energy < 4:
            #    continue
            energies[k].append(energy)
            baselines[k].append(baseline)
            charges[k].append(charge)
            if energy > 4:
                f.write(f"{batch_id} {event_id} {energy}\n")
        #print (fig)
        if fig is not None:
            fig.savefig(f"wf_{batch_id}_{event_id}.png")
            p.close(fig)
            del fig
        gc.collect()
    f.close()
    return energies, baselines, charges


def distribution2d(sample,
                   bins,
                   cmap=p.get_cmap('Blues'),
                   despine=False,
                   interpolation='gaussian',
                   cblabel='events',
                   weights=None,
                   norm=None,
                   figure_factory=None,
                   return_h=False):
    h2 = d.factory.hist2d(sample, bins, weights=weights)
    minval, maxval = min(h2.bincontent[0]), max(h2.bincontent[0])
    if callable(figure_factory):
        fig = figure_factory()
    if figure_factory is None:
        fig = p.figure()
    cmap.set_bad('w', 1)
    if not norm:
        #h2.imshow(log=True, cmap=cmap, interpolation=interpolation, alpha=0.95, label='events')
        h2.imshow(log=False, cmap=cmap, interpolation=interpolation, alpha=0.95, label='events')
    else:
        h2.imshow(log=True, cmap=cmap, interpolation=interpolation, alpha=0.95, label='events', norm=colors.LogNorm(minval, maxval))
    cb = p.colorbar(drawedges=False, shrink=0.5, orientation='vertical', fraction=0.05)
    cb.set_label(cblabel)
    #cb.ticklocation = 'left'
    ax = fig.gca()
    ax.grid(1)
    if despine:
        sb.despine()
    #else:
    #    ax.xaxis.
    if return_h:
        return (fig, h2)
    return fig


if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='Analyze data taken by N6725 digitizer.')
    #parser.add_argument('--config-file',
    #                    dest='configfile', type=str,
    #                    default='',
    #                    help='Specify a config file with all the parameters.')
    parser.add_argument('infiles', metavar='infiles', type=str, nargs='+',
                    help='input root files, taken with CraneLab')
    parser.add_argument('--debug', dest='debug',
                        default=False, action='store_true',
                        help='Set the loglevel to 10, that is debug')
    parser.add_argument('-n','--nevents', dest='nevents',
                        default=-1,type=int, 
                        help='only digest n events')
    parser.add_argument('-o','--outdir', dest='outdir',
                        default=".",type=str, 
                        help='save plots in a directory named "outdir"')
    parser.add_argument('-j','--jobs', dest='njobs',
                        default=4,type=int, 
                        help='number of jobs to use')
    parser.add_argument('--plot-waveforms', dest='plot_waveforms',
                        default=False, action='store_true',
                        help='plot the individual waveforms as png')
    parser.add_argument('--digitizer-energy-only', dest='digitizer_energy_only',
                        default=False, action='store_true',
                        help='Only produce the plot of the energy as calculated by the digitizer')

    args = parser.parse_args()
    loglevel = 20
    if args.debug:
        loglevel = 10
    logger = hep.logger.get_logger(loglevel)
    logger.info(f"Using {args.njobs} cores!")
    logger.info(f"Will run over the following files {args.infiles}")
    sampling = 4e-9

    firstfile = up.open(args.infiles[0]) 
    if not args.digitizer_energy_only:
        waveformdata = firstfile.get("ch0").get("waveform").array()
        waveformdata = np.array([np.array(k) for k in waveformdata])
        t = np.array([sampling*i for i,__ in enumerate(waveformdata[0])])
    energydata = firstfile.get("ch0").get("energy").array() 
    #test = np.array([len(k) for k in waveformdata])
    #print (set(test))
    #print (waveformdata)
    #print (type(waveformdata))
    #print (waveformdata.shape)
    #raise
    thisdata = None
    for filename in tqdm.tqdm(args.infiles[1:],total=len(args.infiles[1:])):   
        f = up.open(filename)
        if not args.digitizer_energy_only:
            thisdata = f.get("ch0").get("waveform").array()
            thisdata = np.array([np.array(k) for k in thisdata])
            print (thisdata.shape)
            waveformdata = np.vstack((waveformdata, thisdata))   
        energydata = np.hstack((energydata,  f.get("ch0").get("energy").array()))

    del thisdata
    #t = np.linspace(0, len(testwf), sampling) 
    gc.collect()
    peaktime = 4E-6 #four microseconds
    order = 4 #fourth order filter
    decay_time = 80E-6 # 100 microsecond preamp tail pulse decay time
    #dt = 32E-9 # 32 nanosecond sample period
    dt = 4e-9

    print (energydata)
    energydata = energydata[energydata > 0]
    ebins = np.linspace(0,100,100)
    print (f"We see {len(energydata)} energies")
    #h = d.factory.hist1d(energydata, bins)
    #h.line()

    if args.outdir != ".":
        logger.info("Will create director {args.outdir}")
        try:
            os.mkdir(args.outdir)
        except Exception as e:
            logger.warn(f"Will overwrite plots in {args.outdir}")
    mod, fig = vis.gaussian_model_fit(energydata,\
                                      startparams=(3, 0.2),\
                                      fig=None,\
                                      norm=True,\
                                      bins=ebins,\
                                      xlabel='digitizer energy [a.u.]')
    ax = fig.gca()
    ax.set_yscale("symlog")
    fig.savefig(os.path.join(args.outdir,"energy_digitizer.png"))
    p.close(fig)
    ebins_digi_zoomin = np.linspace(0,10,100)
    mod, fig = vis.gaussian_model_fit(energydata,\
                                      startparams=(3, 0.2),\
                                      fig=None,\
                                      norm=True,\
                                      bins=ebins_digi_zoomin,\
                                      xlabel='digitizer energy [a.u]')
    ax = fig.gca()
    ax.set_yscale("symlog")
    fig.savefig(os.path.join(args.outdir,"energy_digitizer_zoomin.png"))
    p.close(fig)
    #sys.exit(0)
    if args.digitizer_energy_only:
        sys.exit(0)

    event = 0

    if args.nevents > 0:
        waveformdata = waveformdata[:args.nevents]

    energies = dict([(k, []) for k,__ in enumerate(SHAPINGTIMES)])
    baselines = dict([(k, []) for k,__ in enumerate(SHAPINGTIMES)])
    charges = dict([(k, []) for k,__ in enumerate(SHAPINGTIMES)])
    split_data = np.array_split(waveformdata,args.njobs*4)
    #do_work = worker_factor(args.plot_waveforms)
    worker_args = [(batch_id, args.plot_waveforms, split_data[batch_id]) for batch_id in range(len(split_data))]
    with concurrent.futures.ProcessPoolExecutor(max_workers=args.njobs) as executor:
        for data, result in tqdm.tqdm(zip(worker_args, executor.map(do_work, worker_args)), total=len(split_data)):
            for k, __ in enumerate(SHAPINGTIMES):
                energies[k].extend(result[0][k])
                baselines[k].extend(result[1][k])        
                charges[k].extend(result[2][k])

    # binning
    ebins_zoomin = np.linspace(0,10, 80)
    cbins = np.linspace(0,1e-8, 80)
    ebins = np.linspace(0,100,80)

    resolutions = [] 

    xrayevents, allevents = [],[]
    for k,ptime in enumerate(SHAPINGTIMES):
        
        
        mod, fig = vis.gaussian_fwhm_fit(energies[k],\
                                         startparams=(4, 2, max(energies[k])/2),\
                                         fig=None,\
                                         fitrange=((2,6), (1,5), (0, max(energies[k]))),\
                                         bins=ebins,\
                                         xlabel='shaper peak value [mv]')
        #print (dir(mod))
        ax = fig.gca()
        vis.plotting.adjust_minor_ticks(ax)
        fig.savefig(os.path.join(args.outdir,f"energies_ptime{ptime}gauss.png"))
        
        energies_to_fit = np.asarray(energies[k])
        
        #construct limits
        #the maximum is between 3 and 6
        temph = d.factory.hist1d(energies_to_fit, ebins_zoomin)
        peak  = max(temph.bincontent[np.logical_and(temph.bincenters > 3, temph.bincenters < 6)])
        peakloc = np.where(temph.bincontent == peak)
        peakloc = temph.bincenters[peakloc]
        print (peakloc)
        peakloc = peakloc[0]

        mod, fig = vis.gaussian_fwhm_fit(energies_to_fit,\
                                         #startparams=(energies_to_fit.mean(), energies_to_fit.std(), max(energies_to_fit)),\
                                         startparams=(peakloc,1.5, peak),\
                                         #startparams=(5, 0.8),\
                                         fitrange=((peakloc - (0.2*peakloc),peakloc + 0.2*peakloc), (0,2.5), (peak - 0.1*peak, peak + 0.1*peak)),\
                                         fig=None,\
                                         bins=ebins_zoomin,\
                                         xlabel='shaper peak value [mv]')

        thisenergies = np.asarray(energies[k])
        xrayevents.append(len(thisenergies[thisenergies > 4]))
        allevents.append(len(energies[k]))

        #print (dir(mod))
        ax = fig.gca()
        vis.plotting.adjust_minor_ticks(ax)
        ax.set_yscale("symlog")
        #ax.set_yscale("log")
        fig.savefig(os.path.join(args.outdir,f"energies_ptime{ptime}gausszoomin.png"))
        print ("==============================best fit============")
        print (mod.best_fit_params)
        #resolutions.append(fwhm(mod.best_fit_params[1]))
        resolutions.append(mod.best_fit_params[1])

        fig = distribution2d((charges[k], energies[k]),(cbins,ebins),\
                              cmap=CMAP,
                              interpolation=None,
                              norm=False)
        ax = fig.gca()
        vis.plotting.adjust_minor_ticks(ax)
        fig.savefig(os.path.join(args.outdir,f"charge_energy_{ptime}.png"))             
        p.close(fig)

    # charges and baselines are always the same for different shapers
    fig = p.figure()
    baselines_for_plot = np.asarray(baselines[0])
    baselines_for_plot = bins_to_volts(baselines_for_plot)
    mod, fig = vis.gaussian_model_fit(baselines_for_plot,\
                                      startparams=(baselines_for_plot.mean(), baselines_for_plot.std()),\
                                      fig=None,\
                                      bins=80,\
                                      xlabel='baseline [mV]')
    fig.savefig(os.path.join(args.outdir,f"baselines.png"))

    fig = p.figure()
    charge_for_plot = np.asarray(charges[0])*1e6
    mod, fig = vis.gaussian_model_fit(charge_for_plot,\
                                      startparams=(0.1, 0.02),\
                                      fig=None,\
                                      bins=1e7*cbins,\
                                      xlabel='charge x 1e6 [a.u.]')
    ax = fig.gca()
    vis.plotting.adjust_minor_ticks(ax)
    fig.savefig(os.path.join(args.outdir,f"charges.png"))

    fig = p.figure()
    ax = fig.gca()
    ax.scatter(SHAPINGTIMES, resolutions)
    ax.set_xscale("log")
    fig.savefig(os.path.join(args.outdir,"resolutions.png"))

    for i,ptime in enumerate(SHAPINGTIMES):
        xray = xrayevents[i]
        allev = allevents[i]
        resolution = resolutions[i]
        print (f"We have {xray} of {allev} for peaking time {ptime} with resolution {resolution}")
        
