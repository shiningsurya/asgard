# imports
import os
import sys
import cPickle as cpk
import matplotlib.pyplot as plt
import CandidateData as cd
import CandidateSet as cs

# VIBGYOR colormap the old-school way
colors = dict()
colors['ea01'] = "#FF0000"
colors['ea02'] = "#c3002b"
colors['ea03'] = "#870056"
colors['ea04'] = "#4b0082"
colors['ea05'] = "#3c009b"
colors['ea06'] = "#2d00b4"
colors['ea07'] = "#1e00cd"
colors['ea08'] = "#0f00e6"
colors['ea09'] = "#0000ff"
colors['ea10'] = "#0033cc"
colors['ea11'] = "#006699"
colors['ea12'] = "#009966"
colors['ea13'] = "#00cc33"
colors['ea14'] = "#00ff00"
colors['ea15'] = "#33ff00"
colors['ea16'] = "#66ff00"
colors['ea17'] = "#99ff00"
colors['ea18'] = "#ccff00"
colors['ea19'] = "#ffff00"
colors['ea20'] = "#ffe500"
colors['ea21'] = "#ffcb00"
colors['ea22'] = "#ffb200"
colors['ea23'] = "#ff9800"
colors['ea24'] = "#ff7f00"
colors['ea25'] = "#ff3400"
colors['ea26'] = "#ff4f00"
colors['ea99'] = "#ff0000"

# limits
snmin, snmax = 5, 100
wdmin, wdmax = 1e-3, 1
dmmin, dmmax = 10, 1000

def ReadPkl(f):
    '''
    Handy tool to read candset pickle files.
    '''
    if not f.endswith('.pkl'):
        raise ValueError('Expecting pickle file')
    with open(f, "rb") as ff:
        xx = cpk.load(ff)
    return xx

def AllScatter(xx, saveas=None):
    '''
    Perform a scatter plot.

    Arguments
    ---------
    xx : instance of CandidateSet or pickle filename
    '''
    if isinstance(xx, str):
        x = ReadPkl(xx)
        fi = saveas or "{0}.pdf".format(os.pathname.splitext(xx)[0])
    elif isinstance(xx, dict):
        x = xx
        fi = saveas 
    fig = plt.figure(dpi=300)
    #
    ## S/N vs DM
    ax1 = fig.add_axes([0.1,0.1, 0.4, 0.4])
    ax1.set_ylabel('S/N')
    ax1.set_ylim([snmin, snmax])
    ax1.set_yscale('log')
    ax1.set_xlabel('DM (pc/cc)')
    ax1.set_xscale('log')
    ax1.set_xlim([dmmin, dmmax])
    ## FW vs DM
    ax2 = fig.add_axes([0.1,0.5, 0.4, 0.4])
    ax2.set_ylabel('Width (s)')
    ax2.set_ylim([wdmin, wdmax])
    ax2.set_yscale('log')
    ax2.yaxis.tick_right()
    #ax2.set_xlabel('DM')
    ax2.set_xscale('log')
    ax2.set_xlim([dmmin, dmmax])
    ax2.set_xticks([])
    ## S/N vs FW
    ax3 = fig.add_axes([0.5,0.1, 0.4, 0.4])
    #ax3.set_ylabel('S/N')
    ax3.set_ylim([snmin, snmax])
    ax3.set_yscale('log')
    ax3.set_xlabel('Width (s)')
    ax3.set_xscale('log')
    ax3.set_xlim([wdmin, wdmax])
    ax3.xaxis.tick_top()
    ax3.set_yticks([])
    ## plotting
    for ant, y in x.items():
        ax1.scatter(y.dm, y.sn, label=ant, edgecolors='none', alpha=.6, c=colors[ant])
        ax2.scatter(y.dm, y.width, label=ant,edgecolors='none', alpha=.6, c=colors[ant])
        ax3.scatter(y.width, y.sn, label=ant, edgecolors='none', alpha=.6, c=colors[ant])
    ## legend
    ax3.legend(ncol=2,title=y.tstart, fancybox=True, loc='lower left', bbox_to_anchor=(1.15,1.2), fontsize='small')
    if isinstance(xx, str):
        plt.savefig(fi)
        plt.close()

def VsTime(xx, saveas=None):
    '''
    Performs a vs time plot of S/N, DM, and width.

    Arguments
    ---------
    xx : instance of CandidateSet or pickle filename
    '''
    if isinstance(xx, str):
        x = ReadPkl(xx)
        fi = saveas or "{0}.pdf".format(os.pathname.splitext(xx)[0])
    elif isinstance(xx, dict):
        x = xx
        fi = saveas 
    else:
        raise TypeError("Type of xx not understood.")
    fig = plt.figure(dpi=300)
    ## Width vs PeakTime
    ax1 = fig.add_axes([0.1, 0.05, 0.8, 0.3])
    ax1.set_yscale('log')
    ax1.set_ylabel('Width (s)')
    ax1.set_ylim([wdmin, wdmax])
    ax1.set_xlabel('Time (s)')
    ## S/N vs PeakTime
    ax2 = fig.add_axes([0.1, 0.35, 0.8, 0.3])
    ax2.set_yscale('log')
    ax2.set_ylabel('S/N')
    ax2.set_ylim([snmin, snmax])
    ax2.yaxis.tick_right()
    ax2.set_xticks([])
    ## DM vs PeakTime
    ax3 = fig.add_axes([0.1, 0.65, 0.8, 0.3])
    ax3.set_yscale('log')
    ax3.set_ylabel('DM (pc/cc)')
    ax3.set_ylim([dmmin, dmmax])
    ax3.set_xticks([])
    ## join axes
    ax1.get_shared_x_axes().join(ax1,ax2)
    ax1.get_shared_x_axes().join(ax1,ax3)
    ## plotting
    for ant, y in x.items():
        ax1.scatter(y.peak_time, y.width, label="{0}({1})".format(ant, y.width.size), edgecolors='none', alpha=.6, c=colors[ant])
        ax2.scatter(y.peak_time, y.sn, label="{0}({1})".format(ant, y.sn.size),edgecolors='none', alpha=.6, c=colors[ant])
        ax3.scatter(y.peak_time, y.dm, label="{0}({1})".format(ant, y.dm.size), edgecolors='none', alpha=.6, c=colors[ant])
    ax3.legend(ncol=4,title=y.tstart, fancybox=True, loc='lower left', bbox_to_anchor=(0.0,1.0), fontsize='small')
    if isinstance(xx, str):
        plt.savefig(fi)
        plt.close()
