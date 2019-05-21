#include "asgard.hpp"
#include "Analyzer.hpp"

int main() {
		std::string s(TROOT);
		fs::path ps = s;
		auto fps = ps / std::string("fil");
		auto cps = ps / std::string("cands");
		AnalyzeFB f;
		f.Crawl(fps.string(), cps.string());
		f.PrintPaths();
		f.Groups();
		//f.Summary();
		return 0;
}
