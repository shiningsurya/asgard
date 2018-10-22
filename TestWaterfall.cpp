#include "asgard.hpp"
#include "Analyzer.hpp"
#include "Filterbank.hpp"
#include "Candidate.hpp"
#include "Plotter.hpp"

int main() {
		std::string s(TROOT);
		AnalyzeFB f(s);
		PairGroupDE mg = f.fils[0];
		CandPlot cp(std::string("test_wf.ps/vcps"));

		return 0;
}
