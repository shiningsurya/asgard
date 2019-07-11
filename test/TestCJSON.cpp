#include "asgard.hpp"
#include "Filterbank.hpp"
#include "Candidate.hpp"
#include "FilterbankCandidate.hpp"
#include "CandidateJSON.hpp"


int main() {
std::string filpath("/home/shining/study/MS/vLITE/mkerr/fil/fil2/20180521_183057_muos_ea12_kur.fil");
std::string candpath("/home/shining/study/MS/vLITE/mkerr/cands/20180521_183057_muos_ea12_kur.cand");
		CandidateJSON cj("/tmp/test");
		FilterbankCandidate fbc(filpath, candpath);
		cj.Write(fbc);
		return 0;
}
