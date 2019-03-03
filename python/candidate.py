import numpy as np

class Candidate(object):

    def __init__(self,line,tsamp=1./1280,isfile=False, ant=None, gr=None):
        if isfile:
            f = open(line,'r')
            line  = f.read()
            f.close()
        self.line = line.strip()
        toks = self.line.split()
        self.sn = float(toks[0])
        if np.isinf(self.sn):
            self.sn = 1e5
        self.peak_idx = int(toks[1])
        self.peak_time = float(toks[2])
        # TODO -- is this OBO?
        self.tfilt = int(toks[3])
        self.dmi = int(toks[4])
        self.dm = float(toks[5])
        self.ngiant = int(toks[6])
        self.i0 = int(toks[7])
        self.i1 = int(toks[8])
        self.tsamp = tsamp
        self.width = float(self.i1-self.i0)*self.tsamp
        self.group =gr 
        self.antenna =ant 

    def tophat(self,block,override_tfilt=None):
        tfilt = override_tfilt or self.tfilt
        kernel = [1./2**tfilt]*2**tfilt
        if len(block.shape) == 2:
            kernel = [kernel]
        return fftconvolve(block,kernel,mode='same')

    def overlap(self,other,delta_dm=0.1,delta_width=None):
        """ Overlap if candidates match in DM, width, and time.

        delta_dm is percentage difference relative to arith. mean DM.
            [Default: 10%]
        delta_width is percentage difference relative to geo. mean width.
            [Default: None; NB that 0.71 will allow a 2x difference.]
        """
        if abs(2*(self.dm-other.dm)/(self.dm+other.dm)) > delta_dm:
            return False
        if (delta_width is not None):
            if abs(self.width-other.width)/(self.width*other.width)**0.5 > delta_width:
                return False
        if self.i0 < other.i0:
            return (other.i0 < self.i1)
        return self.i0 < other.i1

    def __str__(self):
        return 'i0=%d i1=%d w=%.2f sn=%.2f dm=%.2f'%(self.i0,self.i1,self.width*1000,self.sn,self.dm)

def ReadCandidates(filename, metadata=False):
    # given filename read all the candidates
    if metadata:
        toks = filename.strip().split("/")[-1]
        toks = toks.split("_")
        gro = toks[0] + "_" + toks[1] + "_" + toks[2] + "_"
        ant = toks[3]
    else:
        gro = 'No-metadata'
        ant = 'No-metadata'
    ret = []
    with open(filename, 'r') as f:
        for line in f:
            try:
                ret.append( Candidate(line, ant=ant, gr=gro) )
            except IndexError:
                print "Error in Reading Candidates from file:", filename.strip().split("/")[-1]
    return ret

class FilCandidate(Candidate):

    def __init__(self,fil,line):
        self.fil = fil
        tsamp = FilReader(self.fil).header['tsamp']
        super(FilCandidate,self).__init__(line,tsamp=tsamp)

    def get_block(self,wmult=1,include_DM=True):
        fr = FilReader(self.fil)
        frh = fr.header
        width = self.i1-self.i0
        if include_DM:
            f0 = frh.fch1
            f1 = frh.nchans*frh.foff + frh.fch1
            dm_delay = 4.148808e3*self.dm*abs(f0**-2-f1**-2)
            dm_width = int(dm_delay/frh.tsamp)
        else:
            dm_width = 0
        start_samp = max(0,self.i0-width*wmult-dm_width)
        nsamp = self.i1+width*wmult+dm_width - start_samp
        block = fr.readBlock(start_samp,nsamp)
        return start_samp,block

def coincidence (all_cands,delta_dm=0.1,delta_width=0.71):
    """ Coincidence all candidates against each other.  Only allow those
    candidates with matching DMs and widths (within one step of the heimdall
    factor of 2) to match.
    """

    # all_cands is a list of candidates from each beam 

    # assign each set of candidates a beam index (arbitrary)
    nbeam = len(all_cands)
    for ibeam in xrange(nbeam):
        for cand in all_cands[ibeam]:
            cand.beam = ibeam
            cand.beam_mask = np.zeros(nbeam,dtype=np.int16)
            cand.matches = []

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
                overlap = cand.overlap(ocand,
                        delta_dm=delta_dm,delta_width=delta_width)
                cand.beam_mask[ocand.beam] += overlap
                if overlap:
                    cand.matches.append(ocand)
            # correlation with previous time slice (in case of straddling)
            for icand,ocand in enumerate(previous_cands):
                overlap = cand.overlap(ocand)
                cand.beam_mask[ocand.beam] += overlap
                if overlap:
                    cand.matches.append(ocand)
                #ocand.beam_mask[cand.beam] += overlap
        
        previous_cands = these_cands
        idx0 = idx1
    return all_cands

 
