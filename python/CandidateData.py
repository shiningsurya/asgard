'''
Container class to hold data

All for the sake of efficiency
'''
import numpy as np
import datetime as dt
import astropy.time as at
import os
import copy

DATEFMT = "%Y%m%d_%H%M%S"

class Candidates(object):
    '''
    Abstract class
    '''
    pass

def ExtractDTandAnt(fname):
    # 20180521_162250_muos_ea02_kur.cand
    # 20180521_162250_muos_ea02.cand
    toks = fname.split('_')
    date_str, ant = None, None
    ret = None
    try:
        date_str = "_".join(toks[:2])
        ant = toks[3]
        ret = dt.datetime.strptime(date_str, DATEFMT) 
    except:
        print toks
        print "Error reading Candidate ", fname
        print "     ant ", ant
        print "     dt  ", ret 
    return ret, ant

def OneLine(line):
    '''
    Returns
    -------
    SN, PEAK_TIME, DM, WIDTH
    '''
    lt = line.strip().split()
    if len(lt) == 2:
        return float(lt[0]), float(lt[1]), None, None
    else:
        pt = float(lt[2])
        tsamp = pt / int(lt[1])
        diff = int(lt[8]) - int(lt[7])
        return float(lt[0]), pt, float(lt[5]), diff * tsamp

class NullCandidateData(object):
    '''
    Null class

    Attributes
    ----------
    procname: str
        procname where it didn't find any cand files
    '''
    def __init__(self, procname):
        self.procname = procname
        self.ant      = "eaXX"

class CandidateData(Candidates):
    '''
    Container class

    Attributes
    ----------
    n : int
        Number of candidates
    sn : array, float
        Signal to Noise of candidates in an array
    width : array, float
        Width in times of candidates in an array
    peak_time : array, float
        Peak times in an array
    DM : array, float
        Dispersion measure in an array
    ant : str
        Antenna
        default: None
    tstart: datetime object
        When the observation was started from the
        filename
    ra: Galactic lat in ra
    dec: Galactic lon in dec
    '''
    def __init__(self, ff, ant=None, tstart_=None):
        if isinstance(ff, str):
            # filename
            self.tstart, self.ant = ExtractDTandAnt(os.path.basename(ff))
            with open(ff, 'r') as ffo:
                flines = ffo.readlines()
        else:
            self.tstart = tstart_
            self.ant = ant
            flines = ff
        self.n = len(flines)
        self.sn = np.zeros(self.n, dtype=np.float)
        self.width = np.zeros(self.n, dtype=np.float)
        self.peak_time = np.zeros(self.n, dtype=np.float)
        self.dm = np.zeros(self.n, dtype=np.float)
        self.ra = None
        self.dec = None
        # index 
        for i, l in enumerate(flines):
            alpha, beta, gamma, delta = OneLine(l)
            if gamma is None:
                self.ra = alpha
                self.dec = beta
            else:
                self.sn[i],self.peak_time[i],self.dm[i], self.width[i] = alpha, beta, gamma, delta 
        # time arry
        if self.tstart:
            tds = map(lambda t : dt.timedelta(seconds=t), self.peak_time)
            time = map(lambda t : self.tstart + t, tds)
            self.mjd = map(lambda t : at.Time(t).mjd, time)

class CandidateGroup(object):
    '''
    To deal with group of CandidateData 
    '''
    def __init__(self, ls):
        '''
        Arguments
        ---------
        ls : list
            List of CandidateData to be merged together
        '''
        ls.sort(key=lambda x : x.tstart)
        self.sn = np.concatenate(map(lambda x : x.sn, ls))
        self.dm = np.concatenate(map(lambda x : x.dm, ls))
        self.mjd = np.concatenate(map(lambda x : x.mjd, ls))
        # tsec
        self.tsec = 86400.0 *  (self.mjd - self.mjd[0] )
        assert np.all(self.tsec >= 0)

def FilterCandidateData(cdata, sncut=10, dmcut=100):
    '''
    To create a new copy of CandidateData instance and
    apply filtering
    '''
    # copying
    ret = copy.deepcopy(cdata)
    # filtering
    sn_selects = cdata.sn < sncut
    dm_selects = cdata.dm < dmcut
    al_selects = np.where( np.logical_or(sn_selects, dm_selects) )
    # selection
    ret.sn = np.delete(ret.sn,al_selects)
    ret.dm = np.delete(ret.dm,al_selects)
    ret.width = np.delete(ret.width,al_selects)
    ret.peak_time = np.delete(ret.peak_time,al_selects)
    ret.n = ret.sn.size
    return ret

def WriteCandidateData(cdata, filename=None, tsamp=97e-6):
    '''
    Writes out candidates in Heimdall format
    If filename is None, filename is constructed from cdata.
    DISCLAIMER: SOME DATA WAS LET GO WHILE CREATING CANDIDATE DATA
    HENCE, THIS FILE AND ORIGINAL FILE WON'T BE SAME
    '''
    # filename logic
    if filename is None:
        filename = "{0}_muos_{1}_fil.cand".format( cdata.tstart.strftime(DATEFMT) ,cdata.ant)
    # generating meta data
    peak_index = np.array( cdata.peak_time / tsamp , dtype=np.uint64)
    filterwidth = np.array(cdata.width / tsamp, dtype=np.uint64)
    i0 = np.array( peak_index - (0.5 * filterwidth), dtype=np.uint64 )
    i1 = np.array( peak_index + (0.5 * filterwidth), dtype=np.uint64 )
    # writing
    with open(filename,'w') as f:
        for i in xrange(cdata.n):
            line = ''
            line += "{0:3.2f}\t".format(cdata.sn[i])
            line += "{0:d}\t".format(peak_index[i])
            line += "{0:3.3f}\t".format(cdata.peak_time[i])
            line += "{0:1d}\t".format(filterwidth[i])
            line += "{0:d}\t".format(0)
            line += "{0:3.2f}\t".format(cdata.dm[i])
            line += "{0:d}\t".format(0)
            line += "{0:d}\t".format(i0[i])
            line += "{0:d}\n".format(i1[i])
            # write
            f.write(line)

class CoincideCandidateData(Candidates):
    '''
    Like CandidateData, 
    '''
    def __init__(self, r, antenna=None, _tstart=None):
        '''
        Arguments
        ---------
        r: list of 4 lists
            ordering:
            IPT, ISN, IDM, IWD
        '''
        # all the lens should be same
        assert len( set(map(len,r)) ) == 1
        self.peak_time = np.array(r[0])
        self.sn = np.array(r[1])
        self.dm = np.array(r[2])
        self.width = np.array(r[3])
        self.n = self.sn.size
        self.ant = antenna
        self.tstart = _tstart
