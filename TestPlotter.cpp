#include "asgard.hpp"
#include "Filterbank.hpp"
#include "Candidate.hpp"
#include "Plotter.hpp"

int main() {
		Filterbank fb;
		FilterbankReader fbr;
		fbr.Read(fb, std::string("/home/shining/study/MS/vLITE/mkerr/fil/20180521_162250_muos_ea02_kur.fil"));
		Candidate cd(std::string("6.07841	100366	78.4109	5	930	270.579	1	100366	100368"),TSAMP);	
		//std::cout << fb << std::endl;
		//Waterfall wf(std::string("wut.ps/cps"),16/1,10,std::string("base"));
		//wf.Plot(1);
		CandPlot cp(std::string("cacaca.ps/vps"));
		cp.CP(fb, cd);
		return 0;
}
