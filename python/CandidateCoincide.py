import os
import numpy as np
from CandidateSet import CandidateSet
from CandidateData import CandidateData

'''
Coinciding class

- Friend of Friend
- DBSCAN

Idea is to do clustering on time-axis
    temporally close candidates should have same mask.
'''

class Coincide(object):
    '''
    Coincide class which implements clustering
    '''
    def __init__(self, cluster='fof', epsilon=1e-1):
        '''
        Arguments
        ---------
        cluster: str
            Underlying clustering algorithm to be used
        epsilon: float
            Parameter for the underlying clustering algorithm
            It is like the tolerance
        '''
        self.algo = cluster
        self.epsilon = epsilon

    def __fof(self, dat):
        '''
        Performs Friend of friend clustering

        Arguments
        ---------
        dat : NumPy array
            SN, WIDTH, PEAK_TIME, DM, ANTENNA 
        '''

    def __dbscan(self, dat):
        '''
        Performs DBSCAN clustering

        Arguments
        ---------
        dat : NumPy array
            SN, WIDTH, PEAK_TIME, DM, ANTENNA 
        '''
        from sklearn.cluster import DBSCAN
        clus = DBSCAN(eps=self.epsilon, min_samples=4).fit(dat)
        return clus.labels_

    def Work(self, x):
        '''
        Work method.

        Arguments
        ---------
        x: instance of CandidateData
        '''
        dat  = np.zeros((5,total_num))
        ants = []
        lasti = 0
        for ant, cd in x:
            lastj = lasti + cd.n
            dat[0, lasti:lastj] = cd.sn
            dat[1, lasti:lastj] = cd.width
            dat[2, lasti:lastj] = cd.peak_time
            dat[3, lasti:lastj] = cd.dm
            ants = ants + [ant]*total_num
            lasti = lastj
        # clustering
        # count antennas in each cluster

def kDCoincide(x, write_pickle=False, only_coincide=False, nn=5, cant=2, tslice=1):
    '''
    k-D Tree based coincidence algorithm.

    Arguments
    ---------
    x : instance of CandidateSet or pickle filename

    Returns
    -------
    instance of CandidateSet reduced

    Algorithm Take 1:
    Get a kDtree on peak_time, sn, dm, width across all the antennas.
    in a step of 1sec(?) query all the kdtrees for points
    sntolerance, dmtolerance matching
    --> For some reason it is missing things.
    '''
    ### KDTree building
    from sklearn.neighbors import KDTree 
    kdtree = dict()
    maxtime = 0.0
    mintstart = min(map(lambda r_: r_.tstart, x.values()))
    for ant,y in x.items():
        diff = ( y.tstart - mintstart ).seconds
        X = np.stack((diff+y.peak_time, y.sn, y.dm, y.width)).T
        kdtree[ant] = KDTree(X, leaf_size=40, metric='minkowski')
        #
        maxtime = max(maxtime, np.max(y.peak_time))
    #
    imaxtime = int(maxtime)
    ## Setting up variables
    IPT = 0
    ISN = 1
    IDM = 2
    IWD = 3
    kdkwargs = {'k':nn, 'breadth_first':True, 'sort_results':True, 'return_distance':False}
    ret = {k:[[],[],[],[]] for k in x.keys()}
    qq = np.zeros((1,4))
    mask = np.zeros(len(x.keys()))
    ## Main loop
    for tt in np.arange(imaxtime,step=tslice):
        qq[IPT] = tt # first is time
        # prepare arrays
        tdict = {k:np.zeros((5,nn)) for k in x.keys()}
        # forall antennas
        for iant, ant in enumerate(x.keys()):
            inn = kdtree[ant].query(qq, **kdkwargs)
            ptin = x[ant].peak_time[inn]
            sel = np.logical_and(ptin >= tt-tslice, ptin <= tt+tslice)
            selsum = sel.sum()
            xinn = inn[sel]
            mask[iant] = selsum
            tdict[ant][IPT,:selsum] = x[ant].peak_time[xinn]
            tdict[ant][ISN,:selsum] = x[ant].sn[xinn]
            tdict[ant][IDM,:selsum] = x[ant].dm[xinn]
            tdict[ant][IWD,:selsum] = x[ant].width[xinn]
            tdict[ant][4,:selsum] = selsum
        # overlap type1
        if np.count_nonzero(mask) >= cant:
            for ant, dat in tdict.items():
                for idx,dval in enumerate(dat[4]):
                    if dval == 0:
                        break
                    else:
                        ret[ant][ISN] += [dat[ISN,idx]]
                        ret[ant][IDM] += [dat[IDM,idx]]
                        ret[ant][IWD] += [dat[IWD,idx]]
                        ret[ant][IPT] += [tt]
        # overlap type2
        # do sn, dm overlap here
        # cleanup
        del tdict
    ## Make CandidateSet out of ret
    return CandidateSet(ret, _tstart = mintstart)

def kDDCoincide(x, write_pickle=False, only_coincide=False, nn=5, cant=2, tslice=1):
    '''
    k-D Tree based coincidence algorithm.

    Arguments
    ---------
    x : instance of CandidateSet or pickle filename

    Returns
    -------
    instance of CandidateSet reduced

    Algorithm Take 2:
    Create ONLY one KDTree. Indices tell which antennas
    '''
    ### KDTree building
    from sklearn.neighbors import KDTree 
    maxtime = 0.0
    mintstart = min(map(lambda r_: r_.tstart, x.values()))
    endtimes = dict()
    X = []
    for ant,y in x.items():
        diff = ( y.tstart - mintstart ).seconds
        aX = np.stack((diff+y.peak_time, y.sn, y.dm, y.width)).T
        X.append(aX)
        endtimes[ant] = aX.shape[0]
        #
        maxtime = max(maxtime, np.max(y.peak_time))
    # train
    X = np.concatenate(X, axis=0)
    kdtree = KDTree(X, leaf_size=40, metric='minkowski')
    #
    imaxtime = int(maxtime)
    ## Setting up variables
    IPT = 0
    ISN = 1
    IDM = 2
    IWD = 3
    kdkwargs = {'k':nn, 'breadth_first':True, 'sort_results':True, 'return_distance':False}
    ret = {k:[[],[],[],[]] for k in x.keys()}
    qq = np.zeros((1,4))
    mask = np.zeros(len(x.keys()))
    ## Main loop
    for tt in np.arange(imaxtime,step=tslice):
        # prepare arrays
        tdict = {k:np.zeros((5,nn)) for k in x.keys()}
        qq[IPT] = tt # first is time
        inn = kdtree.query(qq, **kdkwargs)
        ## query time
        ptin = X[inn, IPT]
        sel = np.logical_and(ptin >= tt-tslice, ptin <= tt+tslice)
        selsum = sel.sum()
        xinn = inn[sel]
        ## figure out antenna
        for ant in FigureAnt(xinn, endtimes):
            iant = x.keys().index(ant)
            mask[iant] = selsum
            tdict[ant][IPT,:selsum] = X[xinn, IPT]
            tdict[ant][ISN,:selsum] = X[xinn, ISN]
            tdict[ant][IDM,:selsum] = X[xinn, IDM]
            tdict[ant][IWD,:selsum] = X[xinn, IWD]
            tdict[ant][4,:selsum] = selsum
        # overlap type1
        if np.count_nonzero(mask) >= cant:
            for ant, dat in tdict.items():
                for idx,dval in enumerate(dat[4]):
                    if dval == 0:
                        break
                    else:
                        ret[ant][ISN] += [dat[ISN,idx]]
                        ret[ant][IDM] += [dat[IDM,idx]]
                        ret[ant][IWD] += [dat[IWD,idx]]
                        ret[ant][IPT] += [tt]
        # overlap type2
        # do sn, dm overlap here
        # cleanup
        del tdict
    ## Make CandidateSet out of ret
    return CandidateSet(ret, _tstart = mintstart)

def FigureAnt(li, et):
    '''
    To help me figure out antenna stuff
    from endtimes

    Arguments
    ---------
    li : list of indices
    et : dictionary

    Returns
    -------
    list of antennas
    '''
    ret = []
    for si in li:
        # one index
        for ant, endindex in et.items():
            if si > endindex:
                si -= endindex
                continue
            else:
                ret.append(ant)
                break
    return ret

def TCoincide(x, tol=0.1):
    '''Perform temporal coincidence.
    Only uses peak_time to do grouping.

    Arguments
    ---------
    x : instance of CandidateData
    tol : max. time difference

    Returns
    -------
    array of size x.n

    '''
    ret = np.zeros(x.n, dtype=np.uint32)
    last_group = 0
    ret[0] = last_group

    for i in range(1,x.n):
        if ( x.peak_time[i] - x.peak_time[i-1] ) >= tol:
            last_group = last_group + 1
        ret[i] = last_group

    assert ret.max() < x.n

    return ret

def Disparity(x, mask):
    '''Measures disparity of the masked candidates.
    Arguments
    ---------
    x : CandidateData array
    mask : as returned by TCoincide
    parameter: paramter to measure disparity of
    Returns
    -------
    disparity array
    '''
    unique_masks = np.unique(mask)
    ret = np.zeros(unique_masks.size)

    for i,u in enumerate(unique_masks):
        masked = x[mask == u]
        pmin = masked.min()
        pmax = masked.max()
        
        ret[i] = (pmax - pmin)/(pmax + pmin)

    return ret



