#include "asgard.hpp"
#include "Analyzer.hpp"
#include "Filterbank.hpp"
#include "Candidate.hpp"
#include "FilterbankCandidate.hpp"
#include "Plotter.hpp"

int main() {
		std::string s(TROOT);
		std::string g(TGROUP);
		AnalyzeFB f(s);
		double zero = 0.0;
		CandidateAntenna cant;
		CandSummary cs(std::string("TestPlots/CandSummary/") + std::string("CandidateSummary4.png/png"), 97e-6f);
		for(MapGroupDE::iterator it = f.cands.begin(); it != f.cands.end(); it++) {
				cant.clear();
				for(fs::directory_entry de : it->second) 
						cant.push_back( ReadCandidates( de.path().string(), zero ) );  
				cs.TPlot(cant);
		}
		return 0;
}
