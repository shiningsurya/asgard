#include "asgard.hpp"
#include "Analyzer.hpp"

int main() {
		std::string s(TROOT);
		AnalyzeFB f(s);
		//f.PrintPaths();
		f.Summary();
		return 0;
}
