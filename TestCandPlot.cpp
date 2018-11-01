#include "asgard.hpp"
//#include "Analyzer.hpp"
#include "Filterbank.hpp"
#include "Candidate.hpp"
#include "FilterbankCandidate.hpp"
#include "Plotter.hpp"

int main() {
		std::string s(TROOT);
		std::string g(TGROUP);
		//AnalyzeFB f(s,g);
		CandPlot cp(std::string("TestPlots/CandPlot/") + g + std::string(".png/png"));
		//
		std::string filfile = s + std::string("/fil/") + g + std::string("_ea02_kur.fil");
		std::string candfile = s + std::string("/cands/") + g + std::string("_ea02_kur.cand");
		FilterbankCandidate fbc(filfile, candfile); 
		//while(fbc.Next()); 
		cp.Plot(fbc);
		return 0;
}
