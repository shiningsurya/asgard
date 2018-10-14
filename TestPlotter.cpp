#include "asgard.hpp"
#include "Plotter.hpp"
#include "Candidate.hpp"
#include "Filterbank.hpp"

int main() {
		Filterbank fb;
		FilterbankReader fbr;
		fbr.Read(fb, std::string("/home/shining/study/MS/vLITE/mkerr/fil/20180521_162250_muos_ea02_kur.fil"));
		Candidate cd("6.07841	100366	78.4109	5	930	270.579	1	100366	100368");	
		std::cout << cd << std::endl;
		//Waterfall wf(std::string("wut.ps/cps"),16/1,10,std::string("base"));
		//wf.Plot(1);
		CandPlot cp(std::string("cacaca.ps/vps"));
		
		cp.CP();
		cp.CP();
		cp.CP();
		return 0;
}
