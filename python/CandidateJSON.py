"""
All things to do with a JSON/UBSON
"""
import os
import numpy as np
import ubjson

def Read(fname):
    """
    Given a filename or list of filenames, returns the Python dict.
    
    Arguments
    ---------
    fname: str, list

    Returns
    -------
    dict or list of dict
    """
    if isinstance(fname, str):
        with open(fname, "rb") as f:
            ret = ubjson.loadb(f.read())
    elif isinstance(fname, list):
        ret = []
        for ff in fname:
            try:
                with open(ff, "rb") as f:
                    ret.append( ubjson.loadb(f.read()) )
            except:
                print "Error with file ", ff
    return ret

def ReadDir(dirname):
    '''
    Given a directory, read all the json/ubson

    Arguments
    ---------
    dirname: str

    Returns
    -------
    list of dicts
    '''
    fnames = [x for x in os.listdir(dirname) if os.path.isfile(x) and os.path.splitext(x)[1] in ["json", "bson"]]
    return Read(fnames)

def FilterSON(ilist, sncut=[10,10000], dmcut=[100,10000], inplace=True):
    '''
    Filters the list of JSONs/UBSONs it gets.
    Doesn't filter SONS

    Arguments
    ---------
    ilist: list of dicts


    Returns
    -------
    new list if inplace=False else same list
    '''
    if inplace:
        retlist = ilist
    else:
        retlist = copy.deepcopy(ilist)
    #
    delidx = []
    for i, x in enumerate(retlist):
        sn, dm = x["sn"], x["dm"]
        if sn > sncut[1] or sn < sncut[0]:
            delidx.append(i)
            continue
        if dm > dmcut[1] or dm < dmcut[0]:
            delidx.append(i)
            continue
    for d in delidx:
        del retlist[d]
    #
    return retlist

def PrintSON(x):
    '''
    Print stuff

    Arguments
    ---------
    x: dict 

    '''
    # XXX maybe I should just put it in a class
    print "---------------------------------------------"
    print "S/N: {0:3.2f}   DM: {1:3.2f} pc/cc".format(x["sn"], x['dm'])

def Unpack(ix, nsamps, nchans, nbits):
    '''
    Takes in JSON/UBSON read array and creates an Numpy float array

    Arguments
    ---------
    ix: list
        of length = nsamps * nchans * nbits / 8
    nsamps:
        int, number of samples
    nchans:
        int, number of channels
    nbits:
        int, number of bits

    Returns
    -------
    Numpy array of size nsamps*nchans
    the fastest changing axis is frequency
    '''
    ret = np.ones((nsamps*nchans), dtype=np.float32)
    idx = 0
    # two bit
    if nbits == 2:
        for ib in ix:
            a,b,c,d = unpack_2bit(ib)
            ret[idx] = a
            idx = idx + 1
            ret[idx] = b
            idx = idx + 1
            ret[idx] = c
            idx = idx + 1
            ret[idx] = d
            idx = idx + 1
    elif nbits == 4:
        for ib in ix:
            a,b = unpack_4bit(ib)
            ret[idx] = a
            idx = idx + 1
            ret[idx] = b
            idx = idx + 1
    elif nbits == 8:
        for ib in ix:
            a = unpack_8bit(ib)
            ret[idx] = a
            idx = idx + 1
    # return after reshaping
    return ret.reshape((nsamps, nchans)).T

###
HI2BITS    = 192
UPMED2BITS = 48
LOMED2BITS = 12
LO2BITS    = 3
###
LO4BITS    = 15
HI4BITS    = 240
###
HI8BITS    = 255

def unpack_2bit(xx):
    '''one byte to 4 2bit samples'''
    a = ( xx & HI2BITS    ) >> 6
    b = ( xx & UPMED2BITS ) >> 4
    c = ( xx & LOMED2BITS ) >> 2
    d = ( xx & LO2BITS    ) >> 0
    return a,b,c,d
def unpack_4bit(xx):
    '''one byte to 2 4bit samples'''
    a = ( xx & HI4BITS ) >> 4
    b = ( xx & LO4BITS ) >> 0
    return a,b
def unpack_8bit(xx):
    '''one byte to 2 8bit samples'''
    a = ( xx & HI8BITS ) >> 0
    return a

class FBC(object):
    '''
    FilterbankCandidate class

    Attributes
    ----------

    all the shebang
    '''
    def __init__(self, filename):
        '''
        Takes filename
        '''
        x = Read(filename)
        nsamps = x['indices']['nsamps']
        dd_nsamps = x['indices']['dd_nsamps']
        nchans = x['frequency']['nchans']
        nbits  = x['parameters']['nbits']
        for k,v in x.items():
            if k == 'dd_fb':
                self.__dict__[k] = Unpack(v, dd_nsamps, nchans, nbits)
            elif k == 'd_fb':
                self.__dict__[k] = Unpack(v, nsamps, nchans, nbits)
            elif k == 'dd_tim':
                self.__dict__[k] = np.array(v)
            elif isinstance(v, dict):
                for kk, vv in v.items():
                    self.__dict__[kk] = vv
            else:
                self.__dict__[k] = v

    def __str__(self):
        return  \
            "S/N: {0:3.2f}\nDM: {1:3.2f} pc/cc\nWidth: {2:3.2f} ms\nPeak Time: {3:3.2f} s\nAntenna: {4}\nSource: {5}\n"\
            "Total time: {6:3.2f} s\nTstart(MJD): {7:7.2f}\nNbits: {8:1d}\nNchans: {9:4d}".format(
            self.sn, self.dm, self.filterwidth * self.tsamp * 1e3, self.peak_time, 
            self.antenna, self.source_name, self.duration, self.tstart, self.nbits, self.nchans)
