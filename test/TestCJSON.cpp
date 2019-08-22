#include "asgard.hpp"
#include "Filterbank.hpp"
#include "Candidate.hpp"
#include "FilterbankCandidate.hpp"
#include "CandidateJSON.hpp"


int main() {
std::string filpath("/mnt/ssd/fildata/20190711_221011_muos_ea06_kur.fil");
std::string candpath("/home/vlite-master/surya/asgard/20190711_221011_muos_ea06_kur.cand");
		CandidateJSON cj("/home/vlite-master/surya/dankspace/json_Test", true, 256);
		FilterbankCandidate fbc(filpath, candpath);
		cj.Write(fbc);
		return 0;
}
