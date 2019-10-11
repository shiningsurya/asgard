#!/usr/bin/env python2.7
import fBSON as fbson
import sys

filename = sys.argv[1]

f_fbson = fbson.FBSON(filename)

print f_fbson
