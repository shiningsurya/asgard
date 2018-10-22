#include "asgard.hpp"
#include "Filterbank.hpp"
#include "Candidate.hpp"
#include "Plotter.hpp"

int main() {
		Filterbank fb;
		FilterbankReader fbr;
		fbr.Read(fb, std::string("/home/shining/study/MS/vLITE/mkerr/fil/20180521_162250_muos_ea02_kur.fil"));
		CandidateList cl = ReadCandidates(std::string("/home/shining/study/MS/vLITE/mkerr/cands/20180521_162250_muos_ea02_kur.cand"), TSAMP);
		std::cout << "CL Size: " << cl.size() << std::endl;
		//Candidate cd(std::string("6.07841	100366	78.4109	5	930	270.579	1	100366	100368"),TSAMP, std::string("20180521_162250_muos"), std::string("ea02"));
		//Waterfall wf(std::string("wut.ps/cps"),16/1,10,std::string("base"));
		//wf.Plot(1);
		CandPlot cp(std::string("cacaca.ps/vcps"));
		cp.CP(fb, cl);
		return 0;
}
