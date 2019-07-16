'''
Candidate set

Class holding candidates in Numpy Tables for efficiency.

Part of Asgard
'''
import os
import copy
from CandidateData import CandidateData, CoincideCandidateData, FilterCandidateData

def GetGroup(cpath):
    '''
    Arguments
    ---------
    cpath: str
        List files in all of cpath
    '''
    cret = []
    cfilelist = [ix for ix in os.listdir(cpath) if os.path.isfile(os.path.join(cpath,ix))]
    # group match and extension match
    for fl in cfilelist:
        fname, ext = os.path.splitext(fl)
        # 20180521_162250_muos_ea02_kur.cand
        # 20180521_162250_muos_ea02.cand
        toks = fname.split('_')
        cret.append( '_'.join(toks[:3])  )
    return list(cret)

def GetGroups(cpath, fpath):
    '''
    Arguments
    ---------
    cpath: str
        List files in all of cpath
    '''
    fret = []
    cret = []
    cfilelist = [ix for ix in os.listdir(cpath) if os.path.isfile(os.path.join(cpath,ix))]
    ffilelist = [ix for ix in os.listdir(fpath) if os.path.isfile(os.path.join(fpath,ix))]
    # group match and extension match
    for fl in cfilelist:
        fname, ext = os.path.splitext(fl)
        # 20180521_162250_muos_ea02_kur.cand
        # 20180521_162250_muos_ea02.cand
        toks = fname.split('_')
        cret.append( '_'.join(toks[:3])  )
    for fl in ffilelist:
        fname, ext = os.path.splitext(fl)
        # 20180521_162250_muos_ea02_kur.cand
        # 20180521_162250_muos_ea02.cand
        toks = fname.split('_')
        fret.append( '_'.join(toks[:3])  )
    return list(set(fret).intersection(cret))

def GetCrawl(cpath, group):
    '''
    Arguments
    ---------
    cpath: str
        List files in all of cpath
    group: str
        Group to check against. 
        Naive test of startswith is employed here

    Returns
    -------
    Files : list of list 
        List of files with same group
    Ants :  list
        List of antennas preserving mapped to files
    Groups : list
        List of groups that it finds
    '''
    ret = dict()
    filelist = [ix for ix in os.listdir(cpath) if os.path.isfile(os.path.join(cpath,ix))]
    # group match and extension match
    for fl in filelist:
        fname, ext = os.path.splitext(fl)
        # 20180521_162250_muos_ea02_kur.cand
        # 20180521_162250_muos_ea02.cand
        toks = fname.split('_')
        if len(toks) >= 4 and ext == '.cand' and fname.startswith(group):
            ret[ toks[3] ] = os.path.join(cpath, fl)
    # return logic again
    return group, ret.values(), ret.keys()

def GetGLGB(cpath, group):
    '''
    Arguments
    ---------
    cpath: str
        List files in all of cpath
    group: str
        Group to check against. 
        Naive test of startswith is employed here

    Returns
    -------
    RAJ : float
        Right Ascension
    DECJ : float
        Declination
    '''
    from pysigproc import SigprocFile
    filelist = [ix for ix in os.listdir(cpath) if os.path.isfile(os.path.join(cpath,ix))]
    # group match and extension match
    for fl in filelist:
        fname, ext = os.path.splitext(fl)
        # 20180521_162250_muos_ea02_kur.fil
        # 20180521_162250_muos_ea02.fil
        # XXX Major assumption that all same group antennas have same pointing
        toks = fname.split('_')
        if len(toks) >= 4 and ext == '.fil' and fname.startswith(group):
            spf = SigprocFile(os.path.join(cpath, fl))
            return spf.src_raj, spf.src_dej
    return 0.0, 0.0


class CandidateSet(dict):
    '''
    Each candidate has:
        1. S/N
        2. Peak Index
        3. Peak Time
        4. Filterwidth
        5. DM Trial Index
        6. DM 
        7. Ngiant
        8. I0
        9. I1

    From which only the following are tabulated:
        1. S/N
        2. Peak Time
        3. Width 
            ( I1 - I0 ) * TSAMP
        4. DM
    '''
    def __init__(self, cpath, group=None, fpath=None, antenna=None, _tstart=None,gl=0.0, gb=0.0):
        '''
        Creates a CandidateSet object where 
        all the candidates belong to a single group.

        Arguments
        ---------
        cpath : str, list, dict
            Path to candidates str
            list of CandidateData
            dict 
        group: str
            Groups to consider
        gl: Galactic coordinates : Latitude
        gb: Galactic coordinates : Longitude 
        '''
        if isinstance(cpath, str):
            self.group = group
            self.cpath = cpath
            self.__group_action(cpath, group, fpath)
        elif isinstance(cpath, list):
            for x in cpath:
                self.__setitem__(x.ant, x)
            self.gl = gl 
            self.gb = gb
            self.group = group
        elif isinstance(cpath, dict):
            self.gl = gl 
            self.gb = gb
            self.group = group
            # after coinciding
            for ant, yy in cpath.items():
                if len(yy[0]) != 0:
                    self.__setitem__(ant, CoincideCandidateData(yy, antenna=ant, _tstart=_tstart))

    def __group_action(self, cpath, group, fpath):
        # walk logic
        # listing files
        self.allgroup, present_cands, self.ants = GetCrawl(cpath, group)
        # merge group and allgroup
        assert(len(present_cands) == len(self.ants))
        self.nants = len(self.ants)
        for cfile, cant in zip(present_cands, self.ants):
            with open(cfile,'r') as fcfile:
                dict.__setitem__(self, cant, CandidateData(fcfile.readlines()))
        # location
        self.fpath = fpath
        if fpath is not None:
            # read from filterbank header
            # TODO make a asgard variant of pysigproc
            # until then use pysigproc
            self.gl, self.gb = GetGLGB(fpath, group)

    def ChangeGroup(self, gr):
        '''
        Arguments
        ---------
        gr : str
            New group to which the object is changed
        '''
        if gr not in self.allgroup:
            raise ValueError("Requested group {0} not in paths given.".format(gr))
        else:
            self.__group_action(self.cpath, gr, self.fpath)

    
def AllCandidates(cpath, fpath):
    '''
    Returns a dict with keys as groups and
    values as instances of candidatesets

    Groups are only included if there are corresponding
    filterbank files

    Arguments
    ---------
    cpath: str
        Path to candidates
    fpath: str
        Path to filterbanks
    '''
    ret = dict()
    allgroup = GetGroups(cpath, fpath)
    print allgroup
    print len(allgroup)
    for ag in allgroup:
        ret[ ag ] = CandidateSet(cpath, ag, fpath)
    return ret

def FilterCandidateSet(cset, sncut=[0,10], dmcut=[0,100], inplace=False):
    '''
    Filter CandidateSet 
    '''
    if inplace:
        ret = cset
    else:
        ret = copy.deepcopy(cset)
    for ant, y in ret.items():
        ret[ant] = FilterCandidateData(y, sncut=sncut, dmcut=dmcut)
    return ret

