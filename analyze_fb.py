import numpy as np
from scipy.signal import fftconvolve
import matplotlib.pyplot as pl 
import matplotlib.lines as mlines
from tqdm import tqdm

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

import pysigproc
from sigpyproc.Readers import FilReader
import glob
import os
from candidate import Candidate,coincidence


def load_histo(fil_fname,noplot=True):
    """ Produce summary statistics of voltage histograms over course of obs.
    """
    #f = np.fromfile(fil_fname.replace('muos_','').replace('fil','histo'),
    f = np.fromfile(fil_fname.replace('fil','histo'),dtype=np.uint32)
    f = f.reshape((len(f)/256,256))
    p0 = f[::2,:]
    p1 = f[1::2,:]
    if noplot:
        return p0,p1
    v1 = np.arange(-128,128,dtype=np.float64)
    v2 = v1**2
    v3 = v1**3
    v4 = v1**4
    m1_p0 = np.asarray([np.average(v1,weights=p) for p in p0])
    m2_p0 = np.asarray([np.average(v2,weights=p) for p in p0])
    #m3_p0 = np.asarray([np.average(v3,weights=p) for p in p0])
    m4_p0 = np.asarray([np.average(v4,weights=p) for p in p0])
    m1_p1 = np.asarray([np.average(v1,weights=p) for p in p1])
    m2_p1 = np.asarray([np.average(v2,weights=p) for p in p1])
    #m3_p1 = np.asarray([np.average(v3,weights=p) for p in p1])
    m4_p1 = np.asarray([np.average(v4,weights=p) for p in p1])
    #std_p0 = (m2_p0 - m1_p0**2)**0.5
    #std_p1 = (m2_p1 - m1_p1**2)**0.5
    kur_p0 = m4_p0 / m2_p0**2
    kur_p1 = m4_p1 / m2_p1**2
    thist = float(p0[0].sum())/128e6 # hard code VLITE sampling rate
    time = np.arange(len(m1_p0))*0.1
    pl.figure(3); pl.clf()
    ax = pl.gca()
    ax.plot(time,m2_p0**0.5)
    ax.plot(time,m2_p1**0.5)
    #ax.plot(time,(m1_p0))
    #ax.plot(time,(m1_p1))
    ax.set_xlabel('Time (s)')
    ax.set_ylabel('Digi Dev.')
    #ax_rt = pl.twinx()
    #t0 = np.median(np.abs(kur_p0-np.median(kur_p0)))
    #t1 = np.median(np.abs(kur_p1-np.median(kur_p1)))
    ax.plot(time,kur_p0,ls='-')
    ax.plot(time,kur_p1,ls='-')
    #ax_rt.set_ylabel('Digi Kur.')
    return p0,p1

def load_kurto(fil_fname,tsamp=0.1,nkurto=500,nfft=12500,use_block=False,fignum=None):
    """ Load the .kurto files, which are records of power and kurtosis
    dumped with high time-resolution.  (Every NKURTO voltage samples, see
    source code.)  The value of tsamp should be set based to 1./SEG_PER_SEC.

    NB there is no spectral resolution here.
    """
    fname = fil_fname.replace('.fil','.block_kurto' if use_block else '.kurto')
    data = np.fromfile(file(fname),dtype=np.float32).astype(np.float64)
    n_per_pol = int(tsamp*128e6/nkurto)
    if use_block:
        n_per_pol /= nfft/nkurto
    # in principle, the axes are chunk,pow/kur,pol,time_in_chunk
    test = data.reshape((-1,2,2,n_per_pol))
    pows = test[:,0,...]
    kurs = test[:,1,...]
    t2_p0,t2_p1 = pows.mean(axis=2).transpose()
    t4_p0,t4_p1 = kurs.mean(axis=2).transpose()
    if fignum:
        pl.figure(fignum); pl.clf()
        c = get_tableau20()
        scale = 128 # convert back to digitizer units
        # this is the square root of power / sigma of digi distribution
        pl.plot(np.arange(len(t2_p0))*tsamp,t2_p0**0.5*scale,color=c[0])
        pl.plot(np.arange(len(t2_p1))*tsamp,t2_p1**0.5*scale,color=c[1])
        pl.twinx()
        # these are scaled kurtoses (should be 3)
        pl.plot(np.arange(len(t4_p0))*tsamp,t4_p0,color=c[2])
        pl.plot(np.arange(len(t4_p1))*tsamp,t4_p1,color=c[3])
    return pows,kurs

def load_weights(fil_fname,tsamp=0.1,nfft=12500,fignum=None):
    """ Load the .kurto files, which are records of power and kurtosis
    dumped with high time-resolution.  (Every NKURTO voltage samples, see
    source code.)  The value of tsamp should be set based to 1./SEG_PER_SEC.

    NB there is no spectral resolution here.
    """
    fname = fil_fname.replace('.fil','.weights')
    data = np.fromfile(file(fname),dtype=np.float32).astype(np.float64)

    n_per_pol = int(tsamp*128e6/nfft)
    weights = data.reshape((-1,2,n_per_pol))
    return weights

def load_cands(fil_fname,sncut=10):
    if fil_fname.endswith('.cand'):
        cand_fname = fil_fname
        fil_fname = None
    else:
        cand_fname = fil_fname.replace('.fil','.cand')
    if not os.path.isfile(cand_fname):
        # try to assemble from .00N files
        cand_fnames = sorted(glob.glob('%s.*'%cand_fname))
        f = file(cand_fname,'w')
        for fname in cand_fnames:
            f.write(file(fname,'r').read())
        f.close()
    lines = filter(lambda x: len(x) > 0, file(cand_fname).readlines())
    cands = [Candidate(fil_fname,line) for line in lines]
    cands = filter(lambda c: c.sn >= sncut,cands)
    return sorted(cands,key=lambda c:c.i0)

def load_data(spf,tstart,tspan,pol=0):
    if pol > spf.nifs-1:
        return
    start_samp = int(tstart/spf.tsamp)
    nsamp = int(tspan/spf.tsamp)
    if (start_samp+nsamp) > spf.nspectra:
        raise ValueError('Requested data beyond range!')
    d = spf.unpack(start_samp,nsamp)[:,pol,:]
    if spf.nchans == 4096:
        # scrunch channels by 4
        d = (d[:,::2] + d[:,1::2])*(1./2)
        d = (d[:,::2] + d[:,1::2])*(1./2)
    elif spf.nchans > 6000:
        # scrunch channels by 6
        d = (d[:,::3] + d[:,1::3] + d[:,2::3])*(1./3)
        d = (d[:,::2] + d[:,1::2])*(1./2)
    if abs(spf.tsamp - 3.2e-4) < 1e-10:
        # scrunch time by 5
        d = (d[::5] + d[1::5] + d[2::5] + d[3::5] + d[4::5])*(1./5)
    else:
        # scrunch time by 4
        d = (d[::2,:] + d[1::2])*(1./2)
    return d


def plot_group(group,clobber=False,use_kurto=False,first_panel=False,
        fignum=2,sncut=1,tspan=2.):
    """ Produce the waterfall plot for the beams."""

    if (not clobber):
        if use_kurto:
            pngs = glob.glob(mkroot+'plots/%s*_kur.png'%group)
        else:
            pngs = filter(lambda l: 'kur' not in l,glob.glob(mkroot+'plots/%s*png'%group))
        pngs = [x for x in pngs if 'cand' not in x]
        pngs = [x for x in pngs if 'coarse' not in x]
        if len(pngs) > 0:
            return

# /home/shining/study/MS/vLITE/mkerr
    fnames = sorted(glob.glob(mkroot+'fil/%s_ea*.fil'%group))
    if use_kurto:
        fnames = filter(lambda f: f.endswith('_kur.fil'),fnames)
    else:
        fnames = filter(lambda f: not f.endswith('_kur.fil'),fnames)
    #if len(fnames) != 5:
        #raise ValueError("Expected 5 antennas!")
    if len(fnames)==0:
        if use_kurto:
            print 'No kurtosis for %s.'%(group)
        else:
            print 'No non-kurtosis for %s.'%(group)
        return
    base = os.path.split(fnames[0])[-1].split('_ea')[0]
    spfs = [pysigproc.SigprocFile(fname) for fname in fnames]
    # print '%d channels, %f tsamp (ms)'%(spfs[0].nchans,spfs[0].tsamp*1e3)
    cands = []
    for fname in tqdm(fnames,ascii=True,unit='Cands/s',name='Reading Cands'):
        cand_fname = fname.replace('.fil','.cand').replace('fil','cands')
        # print cand_fname
        try:
            cands.append(sorted([Candidate(line) for line in filter(lambda x: len(x.strip())>0, file(cand_fname).readlines())],key=lambda c:c.i0))
            cands[-1] = filter(lambda c: c.sn >= sncut,cands[-1])
        except IOError:
            print "[!!] Error at ",cand_fname
            cands.append([])

    # get `tspan` of data
    nspans = np.asarray([int(spf.nspectra*spfs[0].tsamp/tspan) for spf in spfs])
    if not np.all(nspans==nspans[0]):
        print 'Warning, files differ in length!!!'
    # TEMPORARY
    # print 'nspans',nspans
    # end TEMPORARY
    nspan = nspans.min()

    # nspan = min((int(spf.nspectra*spfs[0].tsamp/tspan) for spf in spfs))
    
    pl.figure(fignum, (16,8)); pl.clf()
    pl.subplots_adjust(top=0.99,bottom=0.06,left=0.05,right=0.98)

    freqs = spfs[0].chan_freqs
    delays = 4.15e-3*((freqs*1e-3)**-2-(freqs[0]*1e-3)**-2)
    sel_freqs = freqs[np.asarray([spfs[0].nchans/20,spfs[0].nchans/2,19*(spfs[0].nchans/20)])]
    sel_dels = delays[np.asarray([spfs[0].nchans/200,spfs[0].nchans/2,19*(spfs[0].nchans/20)])]
    colors = ['white','violet','blue','green','yellow','orange','red']
    colors = ['blue']*len(colors)

    # do a kluge to plot 5 time segments in one go
    one_ant_counter = 0


    for j in tqdm(range(nspan),name='Plot',units='Plots/s',ascii=True):
        if first_panel and (j > 0) and (one_ant_counter==0):
            return
        if one_ant_counter == 0:
            pl.clf()
        tstart = j*tspan
        ds = [load_data(spf,tstart,tspan) for spf in spfs]
        for i,spf in enumerate(spfs):
            ax = pl.subplot(5 if len(spfs)==1 else len(spfs),1,i+1+one_ant_counter)
            ax.imshow(ds[i].transpose(),extent=[tstart,tstart+tspan,spf.fch1,spf.fch1+spf.foff*spf.nchans],aspect='auto',interpolation='nearest',origin='lower',cmap='Greys',vmin=0.0,vmax=3.0)
            ax.set_xticks(np.arange(tstart,tstart+tspan+0.01,0.1))
            if j == nspan//2:
                ax.set_ylabel('Freq (MHz)')
            if j == nspan -1:
                ax.set_xlabel('Time (s)')
            ax.text(tstart+0.965*tspan,328,'ea%02d'%spf.telescope_id,size='large',color='black')
            # process candidates
            #plot_cands = filter(lambda c:c.peak_time > tstart and c.peak_time <= tstart+tspan,cands[i])
            plot_cands = cands[i]
            for cand in plot_cands:
                # compute start time in seconds
                cand_tstart = cand.i0 * spf.tsamp
                cand_tstop = cand.i1 * spf.tsamp
                cand_tpeak = cand.peak_time
                times = cand_tstart + cand.dm*delays
                #times = cand_tpeak + cand.dm*delays
                if (times[-1] < tstart) or (times[0] > (tstart+tspan)):
                    continue
                sel_times = cand_tstart + cand.dm*sel_dels
                dt = cand_tstop - cand_tstart
                color = colors[cand.tfilt-1] 
                for i in xrange(3):
                    if i == 1:
                        #ax.plot([cand.peak_time + cand.dm*sel_dels[i]],[sel_freqs[i]],marker='*',color=color)
                        ax.text(cand.peak_time + cand.dm*sel_dels[i],sel_freqs[i],'%d'%(int(round(cand.sn))),color=color,fontsize=12)
                        #ax.plot([cand_tstart + cand.dm*sel_dels[i],cand_tstop + cand.dm*sel_dels[i]],[sel_freqs[i]]*2,color=color)
                    else:
                        #ax.plot([sel_times[i],sel_times[i]+dt],[sel_freqs[i]]*2,color=color)
                        pass

                ax.fill_betweenx(freqs,times,times+dt,alpha=0.05,color=color)
                #ax.plot(times,freqs,lw=1,color=color,alpha=0.5,ls='-')
                #ax.plot(times+dt,freqs,lw=1,color=color,alpha=0.5,ls='-')
            ax.axis([tstart,tstart+tspan,freqs[0],freqs[-1]])
        suffix = '_kur' if use_kurto else ''
        if len(spfs) == 1:
            if (one_ant_counter == 4) or j == nspan-1:
                pl.savefig('/home/shining/study/MS/vLITE/plots/%s_%03d%s.png'%(base,j/5,suffix))
                one_ant_counter = 0
            else:
                one_ant_counter += 1
        else:
            pl.savefig('/home/shining/study/MS/vLITE/plots/%s_%03d%s.png'%(base,j,suffix))

def plot_both_groups(group,clobber=True):
    try:
        plot_group(group,use_kurto=False,clobber=clobber,fignum=3)
        plot_group(group,use_kurto=True,clobber=clobber,fignum=4)
        print 'Succeeded with %s.'%group
    except:
        print 'Failed with %s.'%group

date = '2017071[4-6]'
#date = '*'
date = '2017092[7-9]'
stem = '011000'
#stem = '012210'
stem = '*'
groups = sorted(set([os.path.split(x)[1].split('_ea')[0] for x in glob.glob('/data/kerrm/vlite_fb/%s_%s*.fil'%(date,stem))]))
#groups = sorted(set([os.path.split(x)[1].split('_ea')[0] for x in glob.glob('/data/kerrm/vlite_fb/%s_%s*.cand'%(date,stem))]))

#plot_group(groups[0],clobber=True)
#plot_cands(groups[0])

fnames = sorted(glob.glob('/data/kerrm/vlite_fb/%s_%s*fil'%(date,stem)))
#assert(len(fnames)==1)

"""
p0,p1 = load_histo(fnames[0],noplot=True)
data = np.fromfile(file(fnames[0].replace('.fil','.kurto')),dtype=np.float32).astype(np.float64)
n_per_pol = int(0.1*128e6/500)
# in principle, the axes are chunk,pow/kur,pol,time_in_chunk
test = data.reshape((-1,2,2,n_per_pol))
pows = test[:,0,...]
kurs = test[:,1,...]
m2 = np.asarray([np.average((np.arange(-128,128,dtype=np.float32)/128)**2,weights=p) for p in p0])
m4 = np.asarray([np.average((np.arange(-128,128,dtype=np.float32)/128)**4,weights=p) for p in p0])
t2_p0,t2_p1 = pows.mean(axis=2).transpose()
t4_p0,t4_p1 = kurs.mean(axis=2).transpose()
tsamp = 0.1
pl.figure(1); pl.clf()
pl.plot(np.arange(len(t2_p0))*tsamp,t2_p0,color='blue')
pl.plot(np.arange(len(t2_p1))*tsamp,t2_p1,color='lightblue')
pl.twinx()
pl.plot(np.arange(len(t4_p0))*tsamp,t4_p0,color='red')
pl.plot(np.arange(len(t4_p1))*tsamp,t4_p1,color='coral')
"""

def get_weights(kurs,idx=None):

    # process kurtoses into thresholds
    kur_per_fb = 12500/500
    nweight = kurs.shape[2]/kur_per_fb
    weights = np.empty([kurs.shape[0],2,nweight])
    ok_data = ((kurs < 5) & (kurs > 2.3)).astype(np.float32)
    for i in xrange(kurs.shape[0]):
        for j in xrange(nweight):
            for k in xrange(2):
                weights[i,k,j] = np.average(ok_data[i,k,j*kur_per_fb:(j+1)*kur_per_fb])
    return weights

def kurtosis_percentile_plot(kurs,fignum=5):
    stats = np.percentile(kurs,[2.2,16,50,84,97.7],axis=2)
    pl.figure(fignum); pl.clf()
    times = 0.1*np.arange(kurs.shape[0])
    pl.gca().set_yscale('log')
    pl.plot(times,stats[0,:,0],color='lightblue',ls='-')
    pl.plot(times,stats[0,:,1],color='lightblue',ls='--')
    pl.plot(times,stats[1,:,0],color='blue',ls='-')
    pl.plot(times,stats[1,:,1],color='blue',ls='--')
    pl.plot(times,stats[2,:,0],color='green',ls='-')
    pl.plot(times,stats[2,:,1],color='green',ls='--')
    pl.plot(times,stats[3,:,0],color='tomato',ls='-')
    pl.plot(times,stats[3,:,1],color='tomato',ls='--')
    pl.plot(times,stats[4,:,0],color='red',ls='-')
    pl.plot(times,stats[4,:,1],color='red',ls='--')
    pl.axhline(3,color='k')
    pl.axhline(3-24**0.5/500**0.5,color='k')
    pl.axhline(3+24**0.5/500**0.5,color='k')
    pl.axhline(3-2*24**0.5/500**0.5,color='k')
    pl.axhline(3+2*24**0.5/500**0.5,color='k')

def compute_dagostino(kurs,nkurto=500,weights=None):
    N = float(nkurto)
    if weights is not None:
        N = weights*N
    mu1 = -6./(1+N)
    mu2 = 24.*N*(N-2)*(N-3)/((N+1)**2*(N+3)*(N+5))
    g1 = 6.*(N**2-5*N+2)/((N+7)*(N+9))*(6*(N+3)*(N+5)/(N*(N-2)*(N-3)))**0.5
    A = 6.+8/g1*(2./g1+(1+4/g1**2)**0.5)
    tmp = ( (1-2./A)/(1+(kurs-3-mu1)*(2./(mu2*(A-4)))**0.5) )
    Z = np.where(tmp > 0,(4.5*A)**0.5*(1-2/(9*A)-tmp**(1./3) ),-15)
    #Z = (4.5*A)**0.5*(1-2/(9*A)-( (1-2./A)/(1+(kurs-3-mu1)*(2./(mu2*(A-4)))**0.5) )**(1./3) )
    return Z

def compute_block_kurtosis(kurs,pows):
    dags = compute_dagostino(kurs)
    t = kurs*pows**2
    new_kurs = np.empty((kurs.shape[0],kurs.shape[1],1024))
    new_pows = np.empty((kurs.shape[0],kurs.shape[1],1024))
    new_weights = np.empty_like(new_pows)
    for i in xrange(kurs.shape[0]):
        for j in xrange(2):
            for k in xrange(1024):
                w = np.abs(dags[i,j,k*25:(k+1)*25]) < 3
                try:
                    new_pows[i,j,k] = np.average(pows[i,j,k*25:(k+1)*25],weights=w)
                    new_kurs[i,j,k] = np.average(t[i,j,k*25:(k+1)*25],weights=w)
                    new_weights[i,j,k] = w.sum()
                except ZeroDivisionError:
                    new_pows[i,j,k] = new_kurs[i,j,k] = new_weights[i,j,k] = 0
    new_kurs /= new_pows**2
    return new_kurs,new_pows,new_weights

def get_weights_dagostino(kurs,nfft=12500,nkurto=500,idx=None,dag_thresh=3):

    # process kurtoses into thresholds
    kur_per_fb = nfft/nkurto
    nweight = kurs.shape[2]/kur_per_fb
    if idx is None:
        weights = np.empty([kurs.shape[0],2,nweight])
    else:
        weights = np.empty([2,nweight])
        kurs = kurs[idx]

    dag = compute_dagostino(kurs)
    ok_data = np.abs(dag) < dag_thresh

    if idx is None: 
        for i in xrange(kurs.shape[0]):
            for j in xrange(nweight):
                for k in xrange(2):
                    weights[i,k,j] = np.average(ok_data[i,k,j*kur_per_fb:(j+1)*kur_per_fb])
    else:
        for j in xrange(nweight):
            for k in xrange(2):
                weights[k,j] = np.average(ok_data[k,j*kur_per_fb:(j+1)*kur_per_fb])
    return weights

"""
data = np.fromfile(file(fnames[0].replace('.fil','.block_kurto')),dtype=np.float32)
n_per_pol = int(0.1*128e6/12500)
# in principle, the axes are chunk,pow/kur,pol,time_in_chunk
test = data.reshape((-1,2,2,n_per_pol))
pows_block = test[:,0,...]
kurs_block = test[:,1,...]
t2_block = pows_block.mean(axis=1).mean(axis=1)
t4_block = kurs_block.mean(axis=1).mean(axis=1)

data = np.fromfile(file(fnames[0].replace('.fil','.weights')),dtype=np.float32)
n_per_pol = int(0.1*128e6/12500)
# in principle, the axes are chunk,pow/kur,pol,time_in_chunk
weights = data.reshape((-1,2,n_per_pol))
#t2_block = pows_block.mean(axis=1).mean(axis=1)
#t4_block = kurs_block.mean(axis=1).mean(axis=1)
"""

def group_to_cands(group,use_kurto=True,sncut=10):
    fnames = sorted(glob.glob('/data/kerrm/vlite_fb/%s_ea*.fil'%group))
    if use_kurto:
        fnames = [x for x in fnames if 'kur' in x]
    else:
        fnames = [x for x in fnames if 'kur' not in x]
    if len(fnames)==0:
        cand_fnames = sorted(glob.glob('/data/kerrm/vlite_fb/%s_ea*.cand'%group))
        fnames = cand_fnames
    else:
        cand_fnames = [fname.replace('.fil','.cand') for fname in fnames]
    all_cands = [load_cands(fname,sncut=sncut) for fname,cand_fname in zip(fnames,cand_fnames)]
    return all_cands

def plot_cands(group,sncut=10,use_kurto=True,outpath=None,plot_single=True,
        clobber=False):

    if (not clobber): 
        c1 = os.path.isfile('%s/%s_candplot1.png'%(outpath,group))
        if c1 and os.path.isfile('%s/%s_candplot2.png'%(outpath,group)):
            return
    fnames = sorted(glob.glob('/data/kerrm/vlite_fb/%s_ea*.fil'%group))
    if use_kurto:
        fnames = [x for x in fnames if 'kur' in x]
    else:
        fnames = [x for x in fnames if 'kur' not in x]
    if len(fnames)==0:
        cand_fnames = sorted(glob.glob('/data/kerrm/vlite_fb/%s_ea*.cand'%group))
        fnames = cand_fnames
    else:
        cand_fnames = [fname.replace('.fil','.cand') for fname in fnames]
    base = os.path.split(fnames[0])[-1].split('_ea')[0]
    all_cands = [load_cands(fname,sncut=sncut) for fname,cand_fname in zip(fnames,cand_fnames)]
    coincidence(all_cands)
    antennas = [os.path.basename(x).split('_')[3] for x in cand_fnames]
    pl.clf()
    all_sns = []
    all_dms = []
    all_sizes = []
    all_times = []
    all_cms = []
    axes = []
    pl.figure(1); pl.clf()
    pl.subplots_adjust(hspace=0.00,left=0.13,bottom=0.12,right=0.95,top=0.87)
    msncuts = [1,10,15,20,30,50]
    msizes = [2,3,7,11,14,18]
    mwidths = [3,10,30,100,300,1000]
    colors = get_tableau20(only_dark=True)
    width_colors = [colors[5],colors[3],colors[4],colors[1],colors[0],colors[2]]

    for i,cands in enumerate(all_cands):
        if len(cands)==0:
            continue
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
        ax = pl.subplot(len(all_cands),1,i+1)
        axes.append(ax)
        ax.set_yscale('log')
        cm = coincidence_mask = np.asarray([cand.beam_mask.sum() > 1 for cand in cands])
        if plot_single:
            ax.scatter(times[~cm],dms[~cm],c=c[~cm],marker='o',s=sizes[~cm],alpha=0.5)
        ax.scatter(times[cm],dms[cm],c=c[cm],marker='*',s=sizes[cm],alpha=0.8)
        #for time in times[coincidence_mask]:
        #    pl.axvline(time)
        all_sns.append(sns)
        all_dms.append(dms)
        all_times.append(times)
        all_sizes.append(sizes)
        all_cms.append(coincidence_mask)
        if i < len(all_cands) - 1:
            ax.tick_params(labelbottom='off')
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
        cm = all_cms[i]
        widths = np.asarray([cand.width for cand in cands])*1000
        #fig2_axes.scatter(widths[~cm],dms[~cm],marker='o',s=sizes[~cm],label=antennas[i],alpha=0.5,color=colors[i])
        #fig2_axes.scatter(widths[cm],dms[cm],marker='*',s=sizes[cm],label=antennas[i],alpha=0.5,color=colors[i])
        if plot_single:
            fig2_axes.scatter(dms[~cm],widths[~cm],marker='o',s=sizes[~cm],label=antennas[i],alpha=0.5,color=colors[i])
        fig2_axes.scatter(dms[cm],widths[cm],marker='*',s=sizes[cm],label=antennas[i],alpha=0.8,color=colors[i])
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

def plot_all_cands(groups,outpath='/data/kerrm/plots',clobber=False):
    for group in groups:
        print 'Working on %s.'%group
        try:
            plot_cands(group,outpath=outpath,sncut=8,clobber=clobber)
        except ValueError:
            continue
        except Exception as e:
            import traceback
            print 'Found other exception with %s.'%group
            print traceback.format_exc()
            continue

def plot_all_waterfalls(groups,clobber=False):
    for group in groups:
        plot_both_groups(group,clobber=clobber)

def coincidence (group,delta_dm=0.1):

    try:
        all_cands = group_to_cands(group)
        if len(all_cands==0):
            raise Exception
    except:
        all_cands = group

    # assign each set of candidates a beam index (arbitrary)
    nbeam = len(all_cands)
    for ibeam in xrange(nbeam):
        for cand in all_cands[ibeam]:
            cand.beam = ibeam
            cand.beam_mask = np.zeros(nbeam,dtype=np.int16)

    # make a master list of all candidates
    all_cands = np.concatenate(all_cands)
    if len(all_cands)==0:
        return
    end_times = np.asarray([cand.i1 for cand in all_cands])
    a = np.argsort(end_times)
    all_cands = all_cands[a]
    end_times = end_times[a]*all_cands[0].tsamp

    # now go through each time slice (twice the maximum width) and correlate
    # candidates within it
    tslice = 1
    nslice = int(end_times[-1]/tslice) + 1
    idx0 = 0
    previous_cands = []

    for i in xrange(nslice):
        idx1 = np.searchsorted(end_times,tslice*(i+1))
        these_cands = all_cands[idx0:idx1]

        for cand in these_cands:
            # do correlation between candidates in this time slice
            for icand,ocand in enumerate(these_cands):
                overlap = cand.overlap(ocand)
                cand.beam_mask[ocand.beam] += overlap
                #ocand.beam_mask[cand.beam] += overlap
            # correlation with previous time slice (in case of straddling)
            for icand,ocand in enumerate(previous_cands):
                overlap = cand.overlap(ocand)
                cand.beam_mask[ocand.beam] += overlap
                #ocand.beam_mask[cand.beam] += overlap
        
        previous_cands = these_cands
        idx0 = idx1
    return all_cands

# make a much more concise waterfall plot 

def fb_avg(fname,tgoal=1000,fgoal=128,fscr=None):
    """ Downsample a filterbank file to appropriate resolution for plotting.
    """
    fr = FilReader(fname)
    if fscr is None:
        fscr = int(fr.header['nchans']/fgoal)
    tscr = 2**int(np.log2(fr.header['nsamples']/tgoal))
    gulp = tscr

    nchan = fr.header['nchans']
    nblock = fr.header['nsamples']/tscr
    nsub = nchan/fscr
    output = np.empty( (nsub, nblock) )
    for nsamps,i,data in fr.readPlan(gulp,verbose=False):
        if nsamps != gulp:
            #print 'breaking at %d'%i
            break
        t = data.reshape((gulp,nchan)).transpose().astype(np.float32).mean(axis=1)

        output[:,i] = np.average(t.reshape(nchan/fscr,fscr),axis=1)
        """
        if fscr==1:
            output[:,i] = t
        elif fscr==2:
            output[:,i] = 0.5*(t[::2] + t[1::2])
        elif fscr==4:
            output[:,i] = 0.25*(t[::4] + t[1::4] + t[2::4] + t[3::4])
        else:
            for j in xrange(fscr):
                output[:,i] += t[j::fscr]
            output[:,i] *= 1./fscr
        """

    freqs = np.arange(fr.header['nchans'])*fr.header['foff']+fr.header['fch1']
    freqs = freqs.reshape(len(freqs)/fscr,fscr).mean(axis=1)
    times = np.arange(nblock)*fr.header['tsamp']*gulp

    return output,freqs,times


def plot_both_coarse_groups(group,clobber=False):
    try:
        plot_coarse_group(group,use_kurto=False,clobber=clobber,fignum=3)
        plot_coarse_group(group,use_kurto=True,clobber=clobber,fignum=4)
        print 'Succeeded with %s.'%group
    except:
        print 'Failed with %s.'%group

def plot_candidate(fr,cand,fignum=1):
    """ fr = FilReader, cand = Candidate
    NB this assumes sideband sense where channel 0 = max freq"""
    figx = 7
    figy = 7
    tfilt = 2**cand.tfilt
    width = max(tfilt,cand.i1-cand.i0)
    fhi = fr.header.fch1/1000
    flo = (fr.header.fch1 + fr.header.foff*fr.header.nchans)/1000
    dm_delay = cand.dm*4.15e-3*(flo**-2-fhi**-2)
    dm_delay = int(dm_delay/fr.header.tsamp)+1
    start = max(0,cand.i0-width-dm_delay)
    stop = min(fr.header.nsamples,cand.i1+width+dm_delay)
    data = fr.readBlock(start,stop-start+1)
    dd = data.dedisperse(cand.dm)
    t1 = dd.sum(axis=0)
    kernel = np.ones(tfilt)*(1./tfilt) 
    t2 = fftconvolve(kernel,t1,mode='valid')
    if pl.fignum_exists(fignum):
        x,y = pl.gcf().get_size_inches()
        if x != figx or y != figy:
            pl.close(fignum)
            pl.figure(fignum,(figx,figy))
        else:
            pl.clf()
    else:
        pl.figure(fignum,(figx,figy))
    pl.subplots_adjust(hspace=0,wspace=0,left=0.10,bottom=0.08,top=0.92,right=0.95)
    ax1 = pl.subplot(2,2,1)
    fscr = data.downsample(ffactor=4)
    ax1.imshow(fscr,interpolation='nearest',aspect='auto',cmap='Greys',
            vmin=0,vmax=3)
    ax1.set_xticks([])
    ax1.axvline(cand.i0-start,color='k',ls='--')
    ax1.axvline(cand.i1+dm_delay-start,color='k',ls='--')
    #ax1.axvline(cand.i1+dm_delay,color='k')
    ax2 = pl.subplot(2,2,2)
    fscr = dd.downsample(ffactor=4)
    fscr = fftconvolve(np.matrix([np.ones(tfilt)])*(1./tfilt),fscr,
            mode='valid')
    ax2.imshow(fscr,interpolation='nearest',aspect='auto',cmap='Greys',
            vmin=0,vmax=3)
    ax2.set_xticks([])
    ax2.set_yticks([])

    times = np.arange(len(t1))*cand.tsamp

    ax4 = pl.subplot(2,2,4)
    ax4.plot(times,t1)
    ax4.plot(times[tfilt/2:-(tfilt/2-1)],t2,label='tfilt=%d'%2**cand.tfilt)
    ax4.set_xlabel('Time (s)')
    ax4.set_yticks([])
    from mpl_toolkits.axes_grid1.inset_locator import inset_axes
    ax4i = inset_axes(ax4,"40%","50%",loc=1)
    i0 = (cand.i0-start)-2*width
    i1 = (cand.i0-start)+2*width
    ax4i.plot(times[i0:i1],t1[i0:i1])
    ax4i.plot(times[i0+tfilt/2:i1+tfilt/2],t2[i0:i1])
    ax4i.set_yticks([])
    ymin = ax4.axis()[2]
    ymax = ax4.axis()[3]
    ax4.axis([times[0],times[-1],ymin,ymax])
    ax4.legend(loc='upper left')

    ax3 = pl.subplot(2,2,3)
    t1nodd = data.sum(axis=0)
    ax3.plot(times,data.sum(axis=0))
    t2nodd = fftconvolve(kernel,t1nodd,mode='valid')
    #ax2.axvline(times[cand.peak_idx-start],ls='--',color='k')
    ax3.plot(times[tfilt/2:-(tfilt/2-1)],t2nodd,label='tfilt=%d'%2**cand.tfilt)
    ax3.axis([times[0],times[-1],ymin,ymax])
    ax3.set_xlabel('Time (s)')

    pl.figtext(0.25,0.95,'DM=%.1f  S/N=%.1f, width=%.3f'%(cand.dm,cand.sn,cand.width),size='x-large')

    return data,dd,t1,t2

def process_observation(group):
    """ Make overview plots for data quality and heimdall candidates."""
    pass
    # make a plot fo

def find_0329():
    fnames = glob.glob('/data/kerrm/vlite_fb/*.fil')
    good_fnames = []
    for fname in fnames:
        try:
            fr = FilReader(fname)
            #if '0329' in fr.header.source_name:
            if fr.header.source_name == 'J0303+4716':
                good_fnames.append(fname)
        except:
            print 'failed on %s.'%fname
            continue
    return sorted(good_fnames)

def test_coadd(group='20170627_011000_muos',use_kurto=False):
    """ Test a co-addition algorithm based on a greedy matching of means
        and variances between the beams.
    """

    fnames = sorted(glob.glob('/data/kerrm/vlite_fb/%s_ea*.fil'%group))
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
    frs = map(FilReader,fnames)

    nbeam = len(fnames)
    nsamp = int(2./frs[0].header.tsamp)
    blocks = np.empty((len(fnames),4096,nsamp))
    for ifr,fr in enumerate(frs):
        b = fr.readBlock(int(292./fr.header.tsamp),nsamp)
        blocks[ifr] = b
    means = blocks[:,180:,:].mean(axis=1)
    thresh = 0.1
    consistent = np.empty((len(fnames),len(fnames),nsamp),dtype=np.float32)
    for iblock in xrange(len(fnames)):
        for jblock in xrange(iblock+1,len(fnames)):
            print iblock,jblock
            consistent[iblock,jblock] = np.abs(means[iblock]-means[jblock])<thresh
            consistent[jblock,iblock] = consistent[iblock,jblock]
            print '%d,%d <= %d %d'%(iblock,jblock,jblock,iblock)
        for jblock in xrange(iblock):
            print '%d,%d <= %d %d'%(iblock,jblock,jblock,iblock)
            consistent[iblock,jblock] = consistent[jblock,iblock]
        # adding this bit in gives preference in the odd case where we
        # have two groups of beams well separated; we want to select those
        # that are close to the baseline mean
        consistent[iblock,iblock] = 1. + 0.1*(np.abs(means[iblock]-1.0367)<thresh)

    beams = np.argmax(consistent.sum(axis=0),axis=0)
    # TODO there must be a better way to form this array
    # TODO -- one easy idea for flagging narrowband RFI is just to check
    #   for persistent values (3,0) over multiple sub-ints and to blank
    #   those, particularly if we know the channels; these should also
    #   show up as anomalously low (high) as std over subints
    # TODO -- perhaps add std over channels to consistency check 
    beam_mask = np.asarray([consistent[b,:,i] for i,b in enumerate(beams)]).transpose().astype(int)
    output = (blocks*beam_mask[:,np.newaxis,:]).sum(axis=0)/beam_mask.sum(axis=0)

    return means,consistent,blocks,beam_mask,output

    # sketch of the algorithm -- expect that the mean should be close to 1,
    # and that means should be within ~0.1.  So for each time sample, find
    # the beam closest to 1, then match up all other beams

    # alternatively, go through pairwise beams, and find the largest group
    # of consistant beams

groups = sorted(set([os.path.split(x)[1].split('_ea')[0] for x in glob.glob('/data/kerrm/vlite_fb/20180521*.fil')]))
#for group in groups:
    #plot_both_groups(group,clobber=True)
    #plot_group(group,use_kurto=True,clobber=False)
