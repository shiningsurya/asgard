'''
Fake simulator | Asgard

Python program to generate fake pulsar observations according to a distribution.

TODO

'''
import numpy as np
import subprocess as sp
import tqdm
##### USER IO #####
sig = '6543' # signature
Ndm = 1
Nw  = 10
Nsp = 10
cmd = ['fake']
ldm      = np.random.uniform(400, 1000, (Ndm,))
lwidth   = np.random.randint(1, 10, (Nw,))
lsnrpeak = np.random.uniform(0.001, 10.0, (Nsp,))
ltobs    = [100]
lidx     = np.arange(Ndm*Nw*Nsp*len(ltobs))
tab      = np.zeros((lidx.size, 5))
##### USER IO #####
##### some helper func begin #####
def AddArgument(cmd, argp, argv):
    '''
    Takes cmd puts argp and argv

    Parameters
    ----------

    cmd : str or list
        Command which will be executed in subprocess
    argp : str
        Argument to be passed
    argv : float, int 
        Argument to  

    Example
    -------

    >>> AddArgument(['fake'],'nbits', 2)
    >>> ['fake','-nbits', '2'] 
    '''
    if isinstance(cmd, str):
        cmd = cmd + '  '
        cmd = cmd + '-' + argp
        cmd = cmd + '  '
        cmd = cmd + argv
    elif isinstance(cmd, list):
        st = '-'+argp
        cmd.append(st)
        cmd.append(str(argv))
##
def Execute(cmd, sgs):
    for k, v in sgs.iteritems():
        AddArgument(cmd, k, v)
##### some helper func end  #####
## IMP Only going to pass what it is important
sargs = dict()
sargs['period']       = 1.0       # period (s)
# sargs['width']        = 4         # percent
# sargs['snrpeak']      = 1.0       # s/n
# sargs['dm']           = 1000.0    # pc/cc
sargs['nbits']        = 2         # nbits
sargs['nchans']       = 4096      # filterbank channels
sargs['tsamp']        = 781.25       # (us) sampling time
# sargs['tobs']         = 10        # (s) observation time
sargs['tstart']       = 57420.0   # MJD timestamp of first sample
sargs['nifs']         = 1         # # of intermediate channels
sargs['fch1']         = 361.941449 # frequncy channel of 1 in MHz
sargs['foff']         = -0.010238     # channel bandwidth in MHz
# sargs['seed']         = 10        # seed for random
sargs['rednoise']     = 0.0       # red noise magnitude
# sargs['nosmear']      = 10        # switch for dispersion/sampling smearing
# sargs['swapout']      = 0.0       # byte swapping
# sargs['evenodd']      = 1         # even/odd
# sargs['headerless']   = 768       # write header or not

# rest params don't need to change
pparams = ['width','dm','snrpeak']
# primary
# dm      --> Uniform distribution [400, 2000] pc/cc
# snrpeak --> Uniform distribution [0.001, 10.0]
# width   --> [1,2,3,4,5,6] 
sparams = ['tobs']
# second (don't need to change that much)
# tobs    --> [10, 30, 60, 90, 120, 150, 200] (s)

Execute(cmd, sargs)
idx = 0
header = ''
pb = tqdm.tqdm(desc='Number of fils', total=lidx.size)
for d in ldm:
    AddArgument(cmd, 'dm', d)
    tab[idx, 0] = d
    header = header + 'DM' + ', '
    for w in lwidth:
        AddArgument(cmd, 'width', w)
        tab[idx, 1] = w
        header = header + 'Width' + ', '
        for s in lsnrpeak:
            AddArgument(cmd, 'snrpeak', s)
            tab[idx, 2] = s
            header = header + 'SN' + ', '
            for t in ltobs:
                AddArgument(cmd, 'tobs', t)
                tab[idx, 3] = t
                header = header + 'NumPulses' + ', '
                tab[idx, 4] = lidx[idx]
                header = header + 'Index' 
                fn = sig + '_' + str(lidx[idx]) + '.fil'
                # cmd.append(fn)
                idx = idx + 1
                # execute
                with open(fn,'wb') as fout:
                    sp.Popen(cmd, stdout=fout)
                    pb.update()
## write db
np.save('fakes_dm_width_sn_idx'+sig+'.npy', tab) 
# np.savetxt('fakes'+sig+'.csv', tab, fmt = '%3.2f',header = header, delimiter=',')
