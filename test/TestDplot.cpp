#include "asgard.hpp"
#include "Filterbank.hpp"
#include "Plotter.hpp"

int main() {
		FilterbankReader fbr;
		DPlot xd(std::string("TestPlots/DPlot/dplot.png/png"), 4.0f, 512);
		std::string fbname("/home/shining/study/MS/vLITE/mkerr/fil/20180521_183057_muos_ea03_kur.fil");
		Filterbank f;
		fbr.Read(f, fbname);
		// we have a filterbank
		xd.Plot(f,1);		
		return 0;
}
