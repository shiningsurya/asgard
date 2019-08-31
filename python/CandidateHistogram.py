#!/usr/bin/python
import numpy as np
import matplotlib.pyplot as plt
import corner
import argparse
from candidate import Candidate, ReadCandidates
import gc
## IO
parser = argparse.ArgumentParser(prog='candcorner', epilog='Asgard::CandCorner', description='Candidate corner plot.')
parser.add_argument('cand', help='Candidate file', nargs='*', default=[])
parser.add_argument('-t','--tnpy', help='Truth Numpy file',  default=None)
parser.add_argument('-p','--plot', help='Name of the plot', default=None)
parser.add_argument('-s','--style', help='Style of plot: 0 : s/n DM', default=3, type=int)
parser.add_argument('--whiten', help='Whiten the data', nargs='?', const=True, default=False, type=bool)
parser.add_argument('--bins', help='Number of bins', type=int, default=100)
#
args = parser.parse_args()
if len(args.cand) == 0 and args.tnpy is None:
    parser.print_usage()
    parser.exit()
# starting
if len(args.cand) != 0: 
    cands  = [ ReadCandidates(x) for x in args.cand]
    fcands = [ddx for dx in cands for ddx in dx]
    # extracting data
    xx = np.zeros((len(fcands),3))
    for i, c in enumerate(fcands):
        xx[i,0] = c.sn
        xx[i,1] = c.dm
        xx[i,2] = c.width
    gc.collect()
    if args.whiten:
        means = xx.mean(0)
        xx = xx - means
        # xx = xx / xx.std(0)
elif args.tnpy is not None:
    xxx = np.load(args.tnpy)
    xx = xxx[:,[2,0,1]]
# plot
fn = args.plot
if args.style == 0:
    labels = ['Width', 'S/N']
    xp = xx[:, [2,0]]
elif args.style == 1:
    labels = ['DM', 'S/N']
    xp = xx[:,[1,0]]
elif args.style == 2:
    labels = ['DM', 'Width']
    xp = xx[:,[1,2]]
elif args.style == 3:
    labels = ['S/N', 'DM', 'Width']
    xp = xx
## actually plot
plt.ion()
figure = corner.corner(xp, labels=labels, show_titles=True, scale_hist=True, bins=args.bins)
if args.plot:
    plt.savefig(args.plot)
else:
    plt.show()
