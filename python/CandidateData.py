'''
Container class to hold data

All for the sake of efficiency
'''
import numpy as np

def OneLine(line):
    '''
    Returns
    -------
    SN, PEAK_TIME, DM, WIDTH
    '''
    lt = line.strip().split()
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

class CandidateData(object):
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
    antenna : str
        Antenna
        default: None
    '''
    def __init__(self, flines, ant=None):
        self.ant = ant
        self.n = len(flines)
        self.sn = np.zeros(self.n, dtype=np.float)
        self.width = np.zeros(self.n, dtype=np.float)
        self.peak_time = np.zeros(self.n, dtype=np.float)
        self.dm = np.zeros(self.n, dtype=np.float)
        # index 
        for i, l in enumerate(flines):
            self.sn[i],       \
            self.peak_time[i],\
            self.dm[i],       \
            self.width[i] = OneLine(l)


