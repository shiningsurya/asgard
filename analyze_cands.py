""" 
A new module to handle analysis of recorded candidates.

See previous code in analyze_fb.py.

MTK added 1/3/18.

BSR edits 9/27/18
"""

import glob
import os
mkroot = os.environ['MKERR']
import numpy as np
import pylab as pl
import matplotlib.lines as mlines

tableau20 = [(31, 119, 180), (174, 199, 232), (255, 127, 14), (255, 187, 120),    
            (44, 160, 44), (152, 223, 138), (214, 39, 40), (255, 152, 150),    
            (148, 103, 189), (197, 176, 213), (140, 86, 75), (196, 156, 148),    
            (127, 127, 127), (199, 199, 199),    
            (188, 189, 34), (219, 219, 141), (23, 190, 207), (158, 218, 229)]    
  
# Scale the RGB values to the [0, 1] range, which is the format matplotlib accepts.    
for i in range(len(tableau20)):    
        r, g, b = tableau20[i]    
        tableau20[i] = (r / 255., g / 255., b / 255.)    

def get_tableau20(only_dark=False):
    return tableau20

from candidate import Candidate,coincidence

def load_cands(cand_fname,sncut=10):
    if cand_fname is None:
        return []
    lines = filter(lambda x: len(x) > 0, map(str.strip,file(cand_fname).readlines()))
    cands = [Candidate(line) for line in lines]
    cands = filter(lambda c: c.sn >= sncut,cands)
    return sorted(cands,key=lambda c:c.i0)

def plot_cands(group,sncut=10,outpath=None,plot_single=True,clobber=False):

    if (not clobber): 
        c1 = os.path.isfile('%s/%s_candplot1.png'%(outpath,group))
        if c1 and os.path.isfile('%s/%s_candplot2.png'%(outpath,group)):
            return
    cand_fnames = sorted(glob.glob(mkroot+'cands/%s_ea*.cand'%group))
    all_cands = [load_cands(cand_fname,sncut=sncut) for cand_fname in cand_fnames]
    coincidence(all_cands)
    antennas = [os.path.basename(x).split('_')[3] for x in cand_fnames]
    pl.clf()
    all_sns = []
    all_dms = []
    all_sizes = []
    all_times = []
    all_cm1s = []
    all_cm2s = []
    all_cm3s = []
    axes = []
    pl.figure(1); pl.clf()
    pl.subplots_adjust(hspace=0.00,left=0.13,bottom=0.12,right=0.95,top=0.87)
    msncuts = [1,10,15,20,30,50]
    msizes = [2,3,7,11,14,18]
    mwidths = [3,10,30,100,300,1000]
    colors = get_tableau20(only_dark=False)
    colors = np.append(colors[::2],colors[1::2],axis=0)
    width_colors = [colors[5],colors[3],colors[4],colors[1],colors[0],colors[2]]
    # kluge to allow for more beams
    #colors = np.ravel(np.concatenate([colors,colors,colors]))
    #print colors.shape

    no_cands = True

    for i,cands in enumerate(all_cands):
        # do this regardless of candidates so axes won't be missing
        ax = pl.subplot(len(all_cands),1,i+1)
        axes.append(ax)
        ax.set_yscale('log')
        if len(cands)==0:
            # need this to keep the book keeping correct
            all_sns.append([])
            all_dms.append([])
            all_times.append([])
            all_sizes.append([])
            all_cm1s.append([])
            all_cm2s.append([])
            all_cm3s.append([])
            continue
        no_cands = False
        times = np.asarray([cand.peak_time for cand in cands])
        dms = np.asarray([cand.dm for cand in cands])
        sns = np.asarray([cand.sn for cand in cands])
        sizes = np.ones_like(sns)*1
        for msn,msz in zip(msncuts,msizes):
            sizes[sns > msn] = msz**2
        widths = np.asarray([cand.width for cand in cands])*1e3 # in ms
        c = np.empty((len(widths),3))
        for imw in xrange(len(mwidths)):
            c[widths<mwidths[-(imw+1)]] = width_colors[len(mwidths)-(imw+1)]
        c[widths > mwidths[-1]] = width_colors[-1]
        cm1 = np.asarray([cand.beam_mask.astype(bool).sum() == 1 for cand in cands])
        cm2 = np.asarray([cand.beam_mask.astype(bool).sum() == 2 for cand in cands])
        cm3 = np.asarray([cand.beam_mask.astype(bool).sum() > 2 for cand in cands])
        if plot_single:
            if np.any(cm1):
                ax.scatter(times[cm1],dms[cm1],c=c[cm1],marker='o',s=sizes[cm1],alpha=0.3)
                #ax.scatter(times[cm1],dms[cm1],c=c[cm1],marker='.',alpha=0.3)
        if np.any(cm2):
            ax.scatter(times[cm2],dms[cm2],c=c[cm2],marker='s',s=sizes[cm2],alpha=0.3)
            #ax.scatter(times[cm2],dms[cm2],c=c[cm2],marker='.',alpha=0.3)
        if np.any(cm3):
            ax.scatter(times[cm3],dms[cm3],c=c[cm3],marker='*',s=sizes[cm3],alpha=0.7)
            #ax.scatter(times[cm3],dms[cm3],c=c[cm3],marker='*',alpha=0.3)
        #for time in times[coincidence_mask]:
        #    pl.axvline(time)
        all_sns.append(sns)
        all_dms.append(dms)
        all_times.append(times)
        all_sizes.append(sizes)
        all_cm1s.append(cm1)
        all_cm2s.append(cm2)
        all_cm3s.append(cm3)
        if i < len(all_cands) - 1:
            ax.tick_params(labelbottom='off')

    if no_cands:
        return

    tmin = 0
    tmax = np.max(np.concatenate(all_times))
    if tmax < 10:
        return

    for iax,ax in enumerate(axes):
        ax.text(tmax*0.93,400,antennas[iax])
        ax.axis([tmin,tmax,3,1001])
    legend_lines = [mlines.Line2D([],[],color=width_colors[i],marker='o',markersize=msizes[i],alpha=0.5,label='S/N>%d, W<%dms'%(msncuts[i],mwidths[i]),ls=' ') for i in xrange(len(msizes))]
    axlegend = pl.axes([0.08,0.96,0.88,0.05])
    axlegend.axis('off')
    pl.legend(handles=legend_lines,mode='expand',ncol=3,frameon=False)
    pl.figtext(0.02,0.64,'Dispersion Measure',size='large',rotation='vertical')
    pl.figtext(0.40,0.02,'Elapsed Time (s)',size='large',rotation='horizontal')

    if outpath is not None:
        pl.savefig('%s/%s_candplot1.png'%(outpath,group))

    pl.figure(2); pl.clf()
    fig2_axes = pl.gca()
    fig2_axes.set_xscale('log')
    fig2_axes.set_yscale('log')
    for i,cands in enumerate(all_cands):
        if len(cands)==0:
            continue
        dms = all_dms[i]
        sns = all_sns[i]
        sizes = all_sizes[i]
        cm1 = all_cm1s[i]
        cm2 = all_cm2s[i]
        cm3 = all_cm3s[i]
        widths = np.asarray([cand.width for cand in cands])*1000
        #fig2_axes.scatter(widths[~cm],dms[~cm],marker='o',s=sizes[~cm],label=antennas[i],alpha=0.5,color=colors[i])
        #fig2_axes.scatter(widths[cm],dms[cm],marker='*',s=sizes[cm],label=antennas[i],alpha=0.5,color=colors[i])
        if plot_single and np.any(cm1):
            fig2_axes.scatter(dms[cm1],widths[cm1],marker='o',s=sizes[cm1],label=antennas[i],alpha=0.3,color=colors[i])
        if np.any(cm2):
            fig2_axes.scatter(dms[cm2],widths[cm2],marker='s',s=sizes[cm2],label=antennas[i],alpha=0.3,color=colors[i])
        if np.any(cm3):
            fig2_axes.scatter(dms[cm3],widths[cm3],marker='*',s=sizes[cm3],label=antennas[i],alpha=0.7,color=colors[i])
    dms = np.logspace(np.log10(2),3,100)
    dm_widths = 4.15e-3*(0.320**-2-(0.320+0.064/6250)**-2)*dms*1e3
    fig2_axes.plot(dms,dm_widths,alpha=0.5)
    fig2_axes.plot([1e2,1e2],[1,10],color='k',ls='-',lw=1,alpha=0.5)
    fig2_axes.plot([1e2,1e3],[10,10],color='k',ls='-',lw=1,alpha=0.5)
    #fig2_axes.axis([1,1000,3,1001])
    fig2_axes.axis([3,1001,1,1000])
    #pl.figtext(0.02,0.64,'Dispersion Measure',size='large',rotation='vertical')
    #pl.figtext(0.40,0.02,'Width (ms)',size='large',rotation='horizontal')
    pl.figtext(0.02,0.64,'Width (ms)',size='large',rotation='vertical')
    pl.figtext(0.40,0.02,'Dispersion Measure',size='large',rotation='horizontal')
    legend_lines = [mlines.Line2D([],[],color=colors[i],marker='o',markersize=msizes[2],alpha=0.5,label=antennas[i],ls=' ') for i in xrange(len(antennas))]
    axlegend = pl.axes([0.10,0.96,0.88,0.05])
    axlegend.axis('off')
    pl.legend(handles=legend_lines,mode='expand',ncol=len(antennas),frameon=False,loc='upper right')

    if outpath is not None:
        pl.savefig('%s/%s_candplot2.png'%(outpath,group))

    """
    pl.figure(3); pl.clf()
    fig3_axes = pl.gca()
    fig3_axes.set_xscale('log')
    fig3_axes.set_yscale('log')
    for i,cands in enumerate(all_cands):
        dms = all_dms[i]
        sns = all_sns[i]
        sizes = np.ones_like(sns)
        widths = np.asarray([cand.width for cand in cands])*1000
        for imw in xrange(len(mwidths)):
            sizes[widths<mwidths[-(imw+1)]] = msizes[len(msizes)-(imw+1)]**2
        fig3_axes.scatter(dms,sns,marker='o',s=sizes,label=antennas[i],alpha=0.5,color=colors[i])
    pl.figtext(0.02,0.64,'S/N',size='large',rotation='vertical')
    pl.figtext(0.40,0.02,'Dispersion Measure',size='large',rotation='horizontal')
    #legend_lines = [mlines.Line2D([],[],color=colors[i],marker='o',markersize=msizes[2],alpha=0.5,label=antennas[i],ls=' ') for i in xrange(len(antennas))]
    axlegend = pl.axes([0.10,0.96,0.88,0.05])
    axlegend.axis('off')
    pl.legend(handles=legend_lines,mode='expand',ncol=len(antennas),frameon=False,loc='upper right')

    if outpath is not None:
        pl.savefig('%s/%s_candplot3.png'%(outpath,group))
    """

def plot_all_cands(outpath='.',clobber=False):
    """ Find all groups of candidates and generate standard plots."""
    groups = [os.path.split(x)[1] for x in sorted(glob.glob('/data/kerrm/cands/*.cand'))]
    groups = sorted(set([x.split('_ea')[0] for x in groups]))
    # TMP
    #groups = [x for x in groups if x.startswith('20171218_201943')]
    #groups = [x for x in groups if x.startswith('20180116_16')]
    bad_groups = []
    for group in groups:
        print 'Working on %s.'%group
        #plot_cands(group,outpath=outpath,sncut=8,clobber=clobber)
        #continue
        try:
            plot_cands(group,outpath=outpath,sncut=8,clobber=clobber)
        #except ValueError:
            #continue
        except Exception as e:
            import traceback
            print 'Found other exception with %s.'%group
            print traceback.format_exc()
            bad_groups.append(group)
            continue
    print 'Bad groups:'
    for group in bad_groups:
        print group
            
def plot_coarse_group(group,clobber=False,plot_cands=True,use_kurto=False,fignum=2):
    """ Produce the waterfall plot for the beams at very coarse resolution.
    TODO -- make version without candidates.
    """

    fnames = sorted(glob.glob(mkroot+'fil/%s_ea*.fil'%group))
    if use_kurto:
        fnames = filter(lambda f: f.endswith('_kur.fil'),fnames)
    else:
        fnames = filter(lambda f: not f.endswith('_kur.fil'),fnames)
    if len(fnames)==0:
        if use_kurto:
            print 'No kurtosis for %s.'%(group)
        else:
            print 'No non-kurtosis for %s.'%(group)
        return
    base = os.path.split(fnames[0])[-1].split('_ea')[0]

    suffix = '_kur' if use_kurto else ''
    if plot_cands:
        suffix += '_cands'
    plot_name = '/data/kerrm/plots/%s_coarse%s.png'%(base,suffix)
    if (not clobber) and os.path.isfile(plot_name):
        return

    sncut = 10
    if plot_cands:
        l,r = fnames[0].replace("vlite_fb","cands").replace('fil','cand').split("ea")
        cand_fnames = sorted(glob.glob('*'.join((l,r[2:]))))
    else:
        cand_fnames = []

    # make full list -- for case we have more candidates than fils
    fil_antennas = [os.path.basename(x).split('_')[3] for x in fnames]
    cand_antennas = [os.path.basename(x).split('_')[3] for x in cand_fnames]
    all_antennas = sorted(set(fil_antennas).union(cand_antennas))
    all_fnames = []
    for antenna in all_antennas:
        fname = [x for x in fnames if antenna in x]
        fil_fname = None if len(fname)==0 else fname[0]
        fname = [x for x in cand_fnames if antenna in x]
        cand_fname = None if len(fname)==0 else fname[0]
        all_fnames.append((fil_fname,cand_fname))

    fil_fnames = [x[0] for x in all_fnames]
    cand_fnames = [x[1] for x in all_fnames]
    has_cands = sum((x is not None for x in cand_fnames)) > 0
    all_cands = [load_cands(fname,sncut=sncut) for fname in cand_fnames]

    if plot_cands and has_cands:
        coincidence(all_cands)
        msncuts = [1,10,15,20,30,50]
        msizes = [2,3,7,11,14,18]
        mwidths = [3,10,30,100,300,1000]
        colors = get_tableau20(only_dark=True)
        width_colors = [colors[5],colors[3],colors[4],colors[1],colors[0],colors[2]]

    figx = 14
    figy = 3.0*len(all_antennas)
    if pl.fignum_exists(fignum):
        x,y = pl.gcf().get_size_inches()
        if x != figx or y != figy:
            pl.close(fignum)
            pl.figure(fignum,(figx,figy))
        else:
            pl.clf()
    else:
        pl.figure(fignum,(figx,figy))
    pl.subplots_adjust(top=0.99,bottom=0.16 if len(fnames)==1 else 0.08,left=0.05,right=0.98,hspace=0.02)
    grid_shape = 2*len(all_antennas),5

    pl.clf()

    if plot_cands:
        fig2_axes = pl.subplot2grid(grid_shape, (0,4), rowspan=2*len(fnames))
        fig2_axes.set_xscale('log')
        fig2_axes.set_yscale('log')
        fig2_axes.yaxis.set_label_position('right')

    import pysigproc
    from analyze_fb import fb_avg

    tmin = np.inf
    tmax = 0
    fil_axes = []
    cand_axes = []

    for i,antenna in enumerate(all_antennas):
        fname,cand_fname = all_fnames[i]
        print i,antenna,fname,cand_fname
        if fname is not None:
            spf = pysigproc.SigprocFile(fname)
            data,freqs,times = fb_avg(fname)
            if tmax < times[-1]+(times[1]-times[0]):
                tmax = times[-1]+(times[1]-times[0])
            if times[0] < tmin:
                tmin = times[0]
        if plot_cands:
            ax = pl.subplot2grid(grid_shape, (2*i, 0), colspan=4)
        else:
            ax = pl.subplot(len(all_antennas),1,i+1)
        fil_axes.append(ax)
        if fname is not None:
            ax.imshow(data,extent=[times[0],times[-1],freqs[0],freqs[-1]],vmin=0.9,vmax=1.1,cmap='Greys',aspect='auto',origin='lower')
            ax.set_ylabel('Freq (MHz)')
            ax.text(times[int(len(times)*0.95)],328,'ea%02d'%spf.telescope_id,size='large',color='white')
            if not plot_cands and i==len(all_antennas)-1:
                ax.set_xlabel('Elapsed Time (s)')
            else:
                ax.set_xticks([])
            ax.set_yticks([330,340,350])

        if not plot_cands:
            continue

        axcand = pl.subplot2grid(grid_shape, (2*i+1, 0), colspan=4)
        cand_axes.append(axcand)
        cands = all_cands[i]
        cand_times = np.asarray([cand.peak_time for cand in cands])
        dms = np.asarray([cand.dm for cand in cands])
        sns = np.asarray([cand.sn for cand in cands])
        sizes = np.ones_like(sns)*1
        for msn,msz in zip(msncuts,msizes):
            sizes[sns > msn] = msz**2
        widths = np.asarray([cand.width for cand in cands])*1e3 # in ms
        c = np.empty((len(widths),3))
        for imw in xrange(len(mwidths)):
            c[widths<mwidths[-(imw+1)]] = width_colors[len(mwidths)-(imw+1)]
        fat_pulse_count = np.sum(widths >= mwidths[-1])
        if fat_pulse_count > 0:
            print 'Have %d fat pulses for %s.'%(fat_pulse_count,group)
        c[widths >= mwidths[-1]] = width_colors[-1]
        axcand.set_yscale('log')
        cm = coincidence_mask = np.asarray([cand.beam_mask.sum() > 1 for cand in cands])
        if len(cm) > 0:
            cm &= widths < mwidths[-1]
            axcand.scatter(cand_times[~cm],dms[~cm],c=c[~cm],marker='o',s=sizes[~cm],alpha=0.5)
            axcand.scatter(cand_times[cm],dms[cm],c=c[cm],marker='*',s=sizes[cm],alpha=0.8)
        axcand.set_ylabel('DM')
        if i == len(fnames)-1:
            axcand.set_xlabel('Elapsed Time (s)')
        else:
            axcand.set_xticks([])

        if len(cm) > 0:
            fig2_axes.scatter(dms[~cm],widths[~cm],marker='o',s=sizes[~cm],label=all_antennas[i],alpha=0.5,color=colors[i%len(colors)])
            fig2_axes.scatter(dms[cm],widths[cm],marker='*',s=sizes[cm],label=all_antennas[i],alpha=0.8,color=colors[i%len(colors)])


    #pl.figtext(0.40,0.02,'Elapsed Time (s)',size='large',rotation='horizontal')

    if plot_cands:
        dms = np.logspace(np.log10(2),3,100)
        dm_widths = 4.15e-3*(0.320**-2-(0.320+0.064/6250)**-2)*dms*1e3
        fig2_axes.plot(dms,dm_widths,alpha=0.5)
        fig2_axes.plot([1e2,1e2],[1,10],color='k',ls='-',lw=1,alpha=0.5)
        fig2_axes.plot([1e2,1e3],[10,10],color='k',ls='-',lw=1,alpha=0.5)
        fig2_axes.set_ylabel('Width (ms)')
        fig2_axes.set_xlabel('DM')
        fig2_axes.axis([3,1001,1,1000])
        for axcand in cand_axes:
            axcand.axis([tmin,tmax,3,1001])
    #pl.figtext(0.40,0.02,'Dispersion Measure',size='large',rotation='horizontal')
    """
    legend_lines = [mlines.Line2D([],[],color=colors[i],marker='o',markersize=msizes[2],alpha=0.5,label=antennas[i],ls=' ') for i in xrange(len(antennas))]
    axlegend = pl.axes([0.10,0.96,0.88,0.05])
    axlegend.axis('off')
    pl.legend(handles=legend_lines,mode='expand',ncol=len(antennas),frameon=False,loc='upper right')
    """

    pl.savefig(plot_name)

groups = [os.path.basename(x).split('_ea')[0] for x in sorted(glob.glob('/data/kerrm/vlite_fb/20180521_162250*.fil'))]
groups = sorted(list(set(groups)))
#groups = [x for x in groups if '20180309_002212' in x]
clobber = True
#for group in groups:
    ##try:
    #plot_coarse_group(group,clobber=clobber,plot_cands=True,use_kurto=True)
    #plot_coarse_group(group,clobber=clobber,plot_cands=False,use_kurto=True)
    #plot_coarse_group(group,clobber=clobber,plot_cands=False,use_kurto=False)
    #except Exception as e:
        #print 'failed on %s'%group
