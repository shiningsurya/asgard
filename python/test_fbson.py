#!/usr/bin/env python2.7
import fBSON as fbson
import sys

filename = sys.argv[1]

ff = fbson.FBSON(filename)

#sf = "{0:3.2f}\t{1:3.2f}"
#print sf.format(ff.sn, ff.dm)

print ff
