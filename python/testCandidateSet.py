from CandidateSet import CandidateSet
from CandidateSet import AllCandidates

CPATH='/home/shining/study/MS/vLITE/mkerr/cands'
FPATH='/home/shining/study/MS/vLITE/mkerr/fil'
GROUP1 = '20180521_162516_muos'


x = CandidateSet(CPATH, GROUP1, fpath=FPATH)

r = AllCandidates(CPATH, FPATH)
