import os
import CandidateSet as cs

ROOT = "/home/shining/study/MS/vLITE/mkerr/"
CANDS = os.path.join(ROOT,"cands")
FILS = os.path.join(ROOT,"fil")


ac = cs.AllCandidates(CANDS, FILS)
