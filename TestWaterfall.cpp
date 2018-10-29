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
		Waterfall cp(std::string("TestPlots/Waterfall/") + g + std::string(".png/png"), 2.0f);
		cp.Plot(fl);
		return 0;
}
