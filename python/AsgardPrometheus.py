'''
Prometheus setup for Asgard
'''
import prometheus_client as pc

class CandidatePrometheus(object):
    '''
    Prometheus for candidates
    '''
    def __init__(self, push_gateaway, nodename, jobname = "Duty"):
        '''
        Arguments
        ---------
        push_gateaway: str
            Push gateway
        nodename : str
            Name of node
        jobname : str
            default of "Duty"
            Not important
        '''
        self.push_gateaway = push_gateaway
        self.registry = pc.CollectorRegistry()
        self.nodename = nodename
        self.jobname = jobname
        ### Defining metrics
        # Are there heimdall|fredda candidates?
        self.heimdall_up = pc.Enum('cands_heimdall_up', 'Heimdall candidates present', states=['yes', 'no'], labelnames = ['node', 'antenna'] )
        self.fredda_up = pc.Enum('cands_fredda_up', 'Fredda candidates present', states=['yes', 'no'], labelnames = ['node', 'antenna'] )
        # How many candidates 
        self.heimdall_n = pc.Gauge('cands_heimdall_num', 'Heimdall candidates number', labelnames = ['node', 'antenna'] )
        self.fredda_n = pc.Gauge('cands_fredda_num', 'Fredda candidates number', labelnames = ['node', 'antenna'] )
        # summary of candidates
        # coincider

    def __push(self):
        pc.push_to_gateway(self.push_gateaway, job=self.jobname, registry = self.registry)
    
    def Prom(self, x):
        '''
        Arguments
        ---------
        x : CandidateData or NullCandidateData
        '''
        if isinstance(x, NullCandidateData):
            self.heimdall_n.labels(node=self.nodename, antenna='ea??').set(0)
        elif isinstance(x, CandidateData):
            self.heimdall_up.labels(node=self.nodename, antenna=x.ant).state('yes')
            self.heimdall_n.labels(node=self.nodename, antenna=x.ant).set(x.n)
        # push
        self.__push()

class FilterbankPrometheus(object):
    pass
