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
		for(MapGroupDE::iterator it = f.cands.begin(); it != f.cands.end(); it++) 
				for(fs::directory_entry de : it->second) 
						cant.push_back( ReadCandidates( de.path().string(), zero ) );  
		CandSummary cs(std::string("TestPlots/CandSummary/") + g + std::string(".png/png"));
		cs.Plot(cant);
		return 0;
}
