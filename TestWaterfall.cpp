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
		int ch = 32;
		FilterbankList fl;
		FilterbankReader fbr;
		for(PairGroupDE mg : f.kfils) {
				std::for_each(mg.second.begin(), mg.second.end(), [&fl, &fbr](fs::directory_entry x) { Filterbank xx; fbr.Read(xx, x.path().string()); fl.push_back( xx ); });
				//std::cerr << fl.size() << std::endl;
				//Waterfall cp(std::string("?"), 32);
		}
				Waterfall cp(s + std::string("/TestPlots/Waterfall/") + std::string("one.png/png"), ch);
				cp.Plot(fl);
		return 0;
}
