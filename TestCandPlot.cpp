#include "asgard.hpp"
#include "Analyzer.hpp"
#include "Filterbank.hpp"
#include "Candidate.hpp"
#include "Plotter.hpp"

int main() {
		std::string s(TROOT);
		std::string g(TGROUP);
		AnalyzeFB f(s,g);
		FilterbankList fl;
		FilterbankReader fbr;
		for(PairGroupDE mg : f.kfils) {
				std::for_each(mg.second.begin(), mg.second.end(), [&fl, &fbr](fs::directory_entry x) { Filterbank xx; fbr.Read(xx, x.path().string()); fl.push_back( xx ); });
		}
		std::vector<CandidateList> cl;
		double ts = TSAMP;
		for(PairGroupDE mc : f.cands) {
				std::for_each(mc.second.begin(), mc.second.end(), [&cl, &ts](fs::directory_entry x) { cl.push_back( ReadCandidates(x.path().string(), ts) ); });
		}
		CandPlot cp(std::string("TestPlots/CandPlot/") + g + std::string(".png/png"));
		// sort here
		std::sort(fl.begin(), fl.end(), [](Filterbank& x, Filterbank& y) {return x.antenna < y.antenna;});
		std::sort(cl.begin(), cl.end(), [](CandidateList& x, CandidateList& y) { return x[0].antenna < y[0].antenna;});
		//
		for(int i = 0; i < fl.size(); i++) {
				cp.CP(fl[i], cl[i]);
		}
		return 0;
}
