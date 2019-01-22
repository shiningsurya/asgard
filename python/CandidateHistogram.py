import numpy as np
import sys
import matplotlib.pyplot as plt
import corner
import glob
import gc
from imblearn.over_sampling import SMOTE
sm = SMOTE(ratio = 1.0)
from candidate import Candidate, ReadCandidates
# starting
root = "/home/shining/study/MS/vLITE/mkerr/cands"
files  = sorted(glob.glob(root+"/*.cand"))
cands = [ ReadCandidates(x) for x in files]
fcands = [ddx for dx in cands for ddx in dx]
# cli stuff
if len(sys.argv) < 2:
    print sys.argv[0] 
    print " -g : Group wise"
    print " -a : All "
    sys.exit()
if sys.argv[1] == '-g':
    # groups, antennas and stuff
    # removes duplicates like pro
    groups = list(set([x[0].group for x in cands]))
    ants  = list(set([x[0].antenna for x in cands]))
    # this is our container
    candict = dict() 
    ### eating loop
    ### I never heard of optimization
    ### list comprehensions <3
    for g in groups:
        rd = dict()
        for a in ants:
            rd[a] = [cx for c in cands for cx in c if cx.antenna == a]
        candict[g] = rd    
    # so far so good
    # diagnostic
    for g, cal in candict.iteritems():
        for ant, cl in cal.iteritems():
            x = np.zeros((len(cl), 3))
            y = np.zeros((len(cl),1))
            for i, c in enumerate(cl):
                x[i,0] = c.sn
                x[i,1] = c.width
                x[i,2] = c.dm
            # whiting 
            # x -= np.mean(x, axis=0)
            # x /= 5*np.std(x, axis=0)
            # oversample or SMOTE
            xx = np.repeat(x, 5000, axis=0)
            # xx, yy = sm.fit_sample(x,y)
            # print xx.shape
            # plot
            # continue
            figure = corner.corner(xx, labels=["S/N", "Width", "DM"], show_titles=True, scale_hist=True, bins=60)
            plt.suptitle("              " + g + ant)
            plt.savefig("TestPlots/CandidateHistogram/" + g + ant + ".png")
            plt.clf()
            plt.close()
            gc.collect()
            # plt.show()
elif sys.argv[1] == "-a":
    xx = np.zeros((len(fcands),3))
    for i, c in enumerate(fcands):
        xx[i,0] = c.sn
        xx[i,2] = c.width
        xx[i,1] = c.dm
    figure = corner.corner(xx, labels=["S/N", "DM", "Width"], show_titles=True, scale_hist=True, bins=100)
    plt.savefig("TestPlots/CandidateHistogram/CandidateDistribution_" + str(len(cands)) + ".png")
    plt.clf()
    plt.close()
    figure = corner.corner(xx[:,1:], labels=["DM", "Width"], show_titles=True, scale_hist=True, bins=1000)
    plt.savefig("TestPlots/CandidateHistogram/CandidateDistributionDMW_" + str(len(cands)) + ".png")
    plt.clf()
    plt.close()
