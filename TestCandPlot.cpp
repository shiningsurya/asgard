#include "asgard.hpp"
#include "Analyzer.hpp"
#include "Filterbank.hpp"
#include "Candidate.hpp"
#include "FilterbankCandidate.hpp"
#include "Plotter.hpp"
#include "boost/range/combine.hpp"
#include "boost/foreach.hpp"

StringVector DE2String(DEList x) {
		StringVector ret;
		for(fs::directory_entry& xx : x) ret.push_back( xx.path().string() );
		return ret;
}

int main() {
		std::string s(TROOT);
		AnalyzeFB f(s);
		double zero = 0.0;
		CandidateAntenna cant;
		StringVector ced, fed;
		std::string cfile, ffile;
		//f.Summary();
		for(std::string& g : f.base) {
				CandPlot cp(s + std::string("/TestPlots/CandPlot/") + g + std::string(".png/png"));
				std::cout << "base: " << g << std::endl;
				fed = DE2String(f.kfils[g]); ced = DE2String(f.cands[g]);
				for(std::string cdde : ced)
						cant.push_back( ReadCandidates( cdde, zero ) );  
				// sort fed ced on the basis of antenna
				std::sort(fed.begin(), fed.end(), [](std::string x, std::string y) { return GetAntenna(x) < GetAntenna(y); });
				std::sort(ced.begin(), ced.end(), [](std::string x, std::string y) { return GetAntenna(x) < GetAntenna(y); });
				int lmin = std::min(fed.size(), ced.size());
				for(int i = 0; i < lmin; i++) {
						FilterbankCandidate fbc(fed[i], ced[i]);
						cp.Plot(fbc, cant);
				}
				ced.clear(); fed.clear(); cant.clear();
		}
		return 0;
}
