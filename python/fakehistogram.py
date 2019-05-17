#!/usr/bin/python
import numpy as np
import matplotlib.pyplot as plt
import corner
import argparse
from candidate import Candidate, ReadCandidates
import gc
## IO
parser = argparse.ArgumentParser(prog='fakecandanalysis', epilog='Asgard::FakeCandCorner', description='Fake Candidates corner plot.')
parser.add_argument('sig', help='Signature of the test', required=True)
parser.add_argument('tnpy', help='Truth Numpy file', nargs='1', required=True)
parser.add_argument('cand', help='Candidate files', nargs='*', required=True)
parser.add_argument('--detection', help='Perform detection analysis.', default=True)
parser.add_argument('--estimation', help='Perform estimation analysis.', default=True)
parser.add_argument('-p','--plot', help='Name of the plot', default=None)
parser.add_argument('-s','--style', help='Style of plot: 0 : s/n DM', default=3, type=int)
parser.add_argument('--bins', help='Number of bins', type=int, default=100)
#
args = parser.parse_args()
if args.tnpy is None:
    parser.print_usage()
    parser.exit()
# starting # IO
cands  = [ ReadCandidates(x) for x in args.cand]
lcands = len(args.cand)
tnpy = np.load(args.tnpy)
idxx   = [ x.split('_')[0] for x in args.cand ]
idxx   = np.array(idxx, dtype=np.int)
if np.max(tnpy[:,-1]) != lcands:
    # you're missing out on some candidate files
    raise RunTimeWarning('Some candidate files are missing.')
#
gc.collect()
# detection analysis
if args.detection:
    idx_tp = np.zeros((lcands,))
    idx_fp = np.zeros((lcands,))
    idx_fn = np.zeros((lcands,))
    idx_tn = np.zeros((lcands,))
    #
    tsamp = args.tsamp
    sec2samp = 1/tsamp
    for ixc, ix in enumerate(cands):
        ci0 = np.array( [c.i0 for c in ix]  )
        ti0 = np.arange(0.5/sec2samp, 100.0/sec2samp, 1.0/sec2samp)
