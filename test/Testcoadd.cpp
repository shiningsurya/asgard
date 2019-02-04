#include "asgard.hpp"
#include "Analyzer.hpp"
#include "Filterbank.hpp"
#include "Coadd.hpp"

int main() {
		std::string s(TROOT);
		std::string g = s + std::string("/fil/coaddtest");
		std::string group("20180521_183057_muos");
		AnalyzeFB f;
		f.Crawl(g);
		FilterbankList fl;
		FilterbankReader fbr;
		DEList del = f.kfils[group];
		for(fs::directory_entry de : del) {
				Filterbank xx; 
				fbr.Read(xx, de.path().string()); 
				fl.push_back( xx );
		}
		std::string oname(s + std::string("/fil/") + group + std::string("_ea00_kur.fil"));
		Coadd cd(oname);
		cd.Work(fl);
		return 0;
}
