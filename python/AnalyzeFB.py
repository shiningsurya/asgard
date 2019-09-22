'''
Grouping 
Restructuring
'''
import os
import glob
import numpy as np
import matplotlib.pyplot as plt
from pysigproc import SigprocFile 
from candidate import Candidate, coincidence
from tqdm import tqdm

class AnalyzeFB(object):
    def __init__(self, wd):
        '''
        Constructor for AnalyzeFB object
        ----
        wd is working directory
        ----
        '''
        self.workdir = wd
        self.fildir = os.path.join(wd,'fil')
        self.candir = os.path.join(wd,'cands')
        self.plotdir = os.path.join(wd,'plots')
        fl = os.listdir(self.fildir)
        cl = os.listdir(self.candir)
        # These holds list
        # filist = filter(lambda f: f.endswith('.fil') and not f.endswith('_kur.fil'),fl)
        flist = filter(lambda f: f.endswith('.fil') and not f.endswith('_kur.fil'),fl)
        clist = filter(lambda f: f.endswith('.cand'),cl)
        klist = filter(lambda f: f.endswith('_kur.fil'),fl)
        rb = set([ x.split('_ea')[0] for x in flist ])
        fc = set([ x.split('_ea')[0] for x in clist ])
        fk = set([ x.split('_ea')[0] for x in klist ])
        iset = set()
        for x in [rb,fc,fk]:
            iset.update(x)
        # iset = fc.copy() 
        # print iset
        # print "haha", rb,fk
        # for x in [rb, fk]:
            # print "na",x
            # if not bool(x):
                # print x
                # continue
            # else:
                # iset.intersection_update(x)
        # rb now holds the common bases
        dl = []
        for x in iset:
            xr = dict()
            xr['fil'] = filter(lambda f: f.startswith(x),flist)
            xr['kur_fil'] = filter(lambda f: f.startswith(x),klist)
            xr['cand'] = filter(lambda f: f.startswith(x),clist)
            dl.append(xr)
        # super(AnalyzeFB, self).__init__()
        # dict is not a new style class hence super can't support
        self.dict = {k:v for k,v in zip(iset,dl)}    
        if not bool(self.dict): 
            raise ValueError('Did not find the base you are looking for')
        # Print summary statistics here
        print "[--] Found {0} base(s)".format(len(self.dict))
        for b,k in self.dict.items():
            print "[--] {0:s} has {1:02d} fils and {2:02d} kur fils and {3:02d} candidates.".format(b,len(k['fil']),len(k['kur_fil']),len(k['cand']))
       
    def _load_data(self,spf, tstart, tspan, pol=0):
        '''
        Takes SigprocFile and 
        returns a part of the fil
        '''
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
    @profile
    def Waterfall(self,base=None,use_kurto=False,sncut=1,tspan=2.,fignum=2):
        '''
        takes a base and plots waterfall plots
        if base is None, choose from constructor. 
        take all
        '''
        if base is None:
            base = self.dict
            ######
            # base = dict({'20180521_162250_muos':self.dict['20180521_162250_muos']})
            # use_kurto = True
            ######
        else:
            if base not in self.dict.keys():
                raise ValueError('Requested base wasnot read')
            base = dict({base:self.dict[base]})
        #
        # fignum = 2 # it might make the figure generation faster
        plt.figure(fignum, figsize=(20,8));plt.clf()
        plt.subplots_adjust(top=.99,bottom=0.06,left=.05,right=0.98)
        # You will have to modify this
        colors = ['white','violet','blue','green','yellow','orange','red']
        colors = ['blue']*len(colors)
        for bb in tqdm(base.keys(), desc='Base', unit='#/s',ascii=True):
            if use_kurto:
                fils = [ SigprocFile( os.path.join(self.fildir,x) )  for x in base[bb]['kur_fil'] ]
            else: 
                fils = [ SigprocFile( os.path.join(self.fildir,x) )  for x in base[bb]['fil'] ]
            cands = [ Candidate( os.path.join(self.candir,x),isfile=True )  for x in base[bb]['cand'] ]
            nspans = np.array([ spf.nspectra*spf.tsamp/tspan for spf in fils ],dtype=np.int)
            if not np.all(nspans == nspans[0]):
                print "[!!] Files differ in length!"
            nspan = nspans.min()
            for j in tqdm(range(nspan), desc='Plot', unit='Plots/s',ascii=True):
                tstart = j*tspan
                # fils have to be changed to incoporate multiple bases
                lfils = len(fils)
                for i,f in enumerate(fils):
                    freqs = f.chan_freqs
                    delays = 4.15e-3 * ( (freqs*1e-3)**-2 - (freqs[0]*1e-3)**-2)
                    sfreq = freqs[::freqs.size//3]
                    sdels = delays[::delays.size//3]
                    ax = plt.subplot(lfils,1,i+1) 
                    dat = self._load_data(f,tstart,tspan)
                    ax.imshow(dat.transpose(), extent = [tstart, tstart+tspan, f.fch1, f.fch1 + (f.foff*f.nchans)], aspect='auto', interpolation='nearest', origin='lower',cmap='Greys',vmin=0.,vmax=3.)
                    dat = None
                    ax.set_xticks(np.arange(tstart,tstart+tspan+1e-2,.1))
                    ax.axis([tstart,tstart+tspan,freqs[0],freqs[-1]])
                    ax.text(tstart+.965*tspan, 328, "ea{0:02d}".format(f.telescope_id),size='large',color='black')
                    if i == lfils//2:
                        ax.set_ylabel('Freq [MHz]')
                    if i == lfils - 1:
                        ax.set_xlabel('Time [s]')
                    #
                    # cand plotting
                    for c in cands:
                        ctstart = c.i0 * f.tsamp
                        ctstop = c.i1 * f.tsamp 
                        ctpeak = c.peak_time 
                        times = ctstart + c.dm * delays
                        if times[-1] < tstart or times[0] > (tstart + tspan):
                            # print "[!!] Candidate overflow"
                            continue
                        stimes = ctstart + c.dm * sdels
                        dt = ctstop - ctstart
                        color = colors[c.tfilt - 1]
                        #
                        ax.text(c.peak_time + c.dm*sdels[1], sfreq[1],"{0:d}".format(int(c.sn)),color=color,fontsize=12)
                        ax.fill_betweenx(freqs,times,times+dt,alpha=5e-2,color=color)
                suffix = '_kur' if use_kurto else ''
                if not os.path.exists(os.path.join(self.plotdir, bb)):
                    os.mkdir(os.path.join(self.plotdir,bb))
                plt.savefig(os.path.join(self.plotdir,bb,"{0:s}_{1:03d}{2:s}.png".format(bb,j,suffix)))
    # 
    def KS(base=None,tspan=1.,use_kurto=False,fignum=3,plot=False):
        '''
        Computes statistics and/or Plots a running window distribution.
        '''
        if base is None:
            base = self.basedict.keys()
        else:
            if base not in self.basedict.keys():
                raise ValueError('Requested base wasnot read')
            base = [self.basedict[base]]
        #
        # fignum = 2 # it might make the figure generation faster
        plt.figure(fignum, figsize=(20,8));plt.clf()
        plt.subplots_adjust(top=.99,bottom=0.06,left=.05,right=0.98)
        # You will have to modify this
        colors = ['green','yellow','orange','red']
        styles = ['-',':','-.','--']
        for b in tqdm(range(base), name='Base', units='#/s',ascii=True):
            if use_kurto:
                fils = [ SigprocFile( os.path.join(self.fildir,x['kur_fil']) )  for x in self.basedict[b] ]
            else: 
                fils = [ SigprocFile( os.path.join(self.fildir,x['fil']) )  for x in self.basedict[b] ]
            cands = [ Candidate( os.path.join(self.candir,x['cand']) ) for x in self.basedict[b] ] 
            nspans = np.array([ spf.nspectra*spf.tsamp/tspan for spf in fils ],dtype=np.int)
            if not np.all(nspans == nspans[0]):
                print "[!!] Files differ in length!"
            nspan = nspans.min()
            for j in tqdm(range(nspan), name='Distro', units='distro/s',ascii=True):
                tstart = j*tspan
                # fils have to be changed to incoporate multiple bases
                lfils = len(fils)
                for i,f in enumerate(fils):
                    ax = plt.subplot(lfils,1,i+1) 
                    dat = self._load_data(f,tstart,tspan)
                    dfreq = [ np.unique(x,return_counts=True)[1] for x in dat.T ]
                    dtime = [ np.unique(x,return_counts=True)[1] for x in dat ]
                    
                    ax.imshow(dat.transpose(), extent = [tstart, tstart+tspan, f.fch1, f.fch1 + (f.foff*f.nchans)], aspect='auto', interpolation='nearest', origin='lower',cmap='Greys',vmin=0.,vmax=3.)
                    ax.set_xticks(np.arange(tstart,tstart+tspan+1e-2,.1))
                    ax.axis([tstart,tstart+tspan,freqs[0],freqs[-1]])
                    ax.text(tstart+.965*tspan, 328, "ea{0:02d}".format(f.telescope_id),size='large',color='black')
                    if i == lfils//2:
                        ax.set_ylabel('Freq [MHz]')
                    if i == lfils - 1:
                        ax.set_xlabel('Time [s]')
                    #
                    # cand plotting
                    for c in cands:
                        ctstart = c.i0 * f.tsamp
                        ctstop = c.i1 * f.tsamp 
                        ctpeak = c.peak_time 
                        times = ctstart + c.dm * delays
                        if times[-1] < tstart or times[0] > (tstart + tspan):
                            print "[!!] Candidate overflow"
                            continue
                        stimes = ctstart + c.dm * sdels
                        dt = ctstop - ctstart
                        color = colors[c.tfilt - 1]
                        #
                        ax.text(c.peak_time + c.dm*sdels[1], sfreq[1],"{0:d}".format(c.sn),color=color,fontsize=12)
                        ax.fill_betweenx(freqs,times,times+dt,alpha=5e-2,color=color)
                suffix = '_kur' if use_kurto else ''
                if not os.path.exists(os.path.join(self.plotdir, b)):
                    os.path.mkdir(os.path.join(self.plotdir,b))
                plt.savefig(os.path.join(self.plotdir,b,"{0:s}_{1:03d}{2:s}.png".format(b,j,suffix)))
        

if __name__ == "__main__":
    g = AnalyzeFB('/home/shining/study/MS/vLITE/mkerr')
    g.Waterfall(base='20180521_162250_muos',use_kurto=True)
