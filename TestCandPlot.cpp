#include "asgard.hpp"
#include "Analyzer.hpp"
#include "Filterbank.hpp"
#include "Candidate.hpp"
#include "FilterbankCandidate.hpp"
#include "Plotter.hpp"

int main() {
		std::string s(TROOT);
		std::string g(TGROUP);
		AnalyzeFB f(s,g);
		double zero = 0.0;
		CandidateAntenna cant;
		CandPlot cp(std::string("TestPlots/CandPlot/") + g + std::string(".png/png"));
		//
		std::string filfile = s + std::string("/fil/") + g + std::string("_ea02_kur.fil");
		std::string candfile = s + std::string("/cands/") + g + std::string("_ea02_kur.cand");
		FilterbankCandidate fbc(filfile, candfile); 
		for(MapGroupDE::iterator it = f.cands.begin(); it != f.cands.end(); it++) 
				for(fs::directory_entry de : it->second) 
						cant.push_back( ReadCandidates( de.path().string(), zero ) );  
		//while(fbc.Next()); 
		cp.Plot(fbc, cant);
		return 0;
}
