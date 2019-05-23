'''
Coinciding class

- Friend of Friend
- DBSCAN

Idea is to do clustering on 4D space
    SN, PEAK_TIME, DM, WIDTH
and then checking which all antennas are there
'''

class Coincide(object):
    '''
    Coincide class which implements clustering
    '''
    def __init__(self, cluster='fof', epsilon=1e-1):
        '''
        Arguments
        ---------
        cluster: str
            Underlying clustering algorithm to be used
        epsilon: float
            Parameter for the underlying clustering algorithm
            It is like the tolerance
        '''
        self.algo = cluster
        self.epsilon = epsilon

    def __fof(self, dat):
        '''
        Performs Friend of friend clustering

        Arguments
        ---------
        dat : NumPy array
            SN, WIDTH, PEAK_TIME, DM, ANTENNA 
        '''

    def __dbscan(self, dat):
        '''
        Performs DBSCAN clustering

        Arguments
        ---------
        dat : NumPy array
            SN, WIDTH, PEAK_TIME, DM, ANTENNA 
        '''
        from sklearn.cluster import DBSCAN
        clus = DBSCAN(eps=self.epsilon, min_samples=4).fit(dat)
        return clus.labels_

    def Work(self, x):
        '''
        Work method.

        Arguments
        ---------
        x: instance of CandidateSet
            which is basically a dict
        '''
        # merge all the arrays into one big arrays
        total_num = sum([v.n for v in x.values()])
        dat  = np.zeros((5,total_num))
        ants = []
        lasti = 0
        for ant, cd in x:
            lastj = lasti + cd.n
            dat[0, lasti:lastj] = cd.sn
            dat[1, lasti:lastj] = cd.width
            dat[2, lasti:lastj] = cd.peak_time
            dat[3, lasti:lastj] = cd.dm
            ants = ants + [ant]*total_num
            lasti = lastj
        # clustering
        # count antennas in each cluster
