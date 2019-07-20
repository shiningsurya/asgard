#include "asgard.hpp"
#include "Filterbank.hpp"
#include "Candidate.hpp"
#include "FilterbankCandidate.hpp"
#include "CandidateJSON.hpp"


int main() {
std::string filpath("/mnt/ssd/fildata/20190711_215217_muos_ea11_kur.fil");
std::string candpath("/mnt/ssd/cands/20190711_215217_muos_ea11_kur.cand");
		CandidateJSON cj("/home/vlite-master/surya/dankspace/json_Test", true);
		FilterbankCandidate fbc(filpath, candpath);
		cj.Write(fbc);
		return 0;
}
