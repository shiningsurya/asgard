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
                print ( "Error with file ", ff )
    return ret

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

class FBSON(object):
    '''
    Filterbank BSON
    Attributes
    ----------
    all the shebang
    '''
    def __init__(self, filename, _lazy = True):
        '''
        Takes filename
        '''
        x = Read(filename)
        for k,v in x.items():
            if k in ["sn", "dm", "width"]:
                self.__dict__[k] = v
        # 
        for sd in ["frequency", "time", "parameters"]:
            for k,v in x[sd].items():
                self.__dict__[k] = v
        #
        self.nsamps = len(x['fb']) / self.nchans /self.nbits * 8
        self.lazy = _lazy
        self.fb = None
        if not self.lazy:
            self.fb = Unpack(x['fb'], self.nsamps, self.nchans, self.nbits)

    def __str__(self):
        if self.fb is None:
            size = self.nsamps * self.nchans * self.nbits / 8e6
        else:
            size = self.fb.nbytes / 1e6
        return  \
            "S/N: {0:3.2f}\nDM: {1:3.2f} pc/cc\nWidth: {2:3.2f} ms\nPeak Time: {3:3.2f} s\nAntenna: {4}\nSource: {5}\n"\
            "Tstart(MJD): {7:7.2f}\nNbits: {8:1d}\nNchans: {9:4d}\nSizeFB: {10:3.2f}MB".format(
            self.sn, self.dm, self.width, self.peak_time, 
            self.antenna, self.source_name, self.duration, self.tstart, self.nbits, self.nchans, size)
