#include "asgard.hpp"
#include "Analyzer.hpp"
#include "Filterbank.hpp"
#include "Coadd.hpp"

namespace mpi = boost::mpi;

int main() {
		std::string s(TROOT);
		std::string g = s + std::string("/fil/coaddtest");
		std::string group("20180521_162516_muos_");
		AnalyzeFB f;
		f.Crawl(g);
		FilterbankList fl;
		FilterbankReader fbr;
		PathList del = f.kfils[group];
		for(const auto& de : del) {
				Filterbank xx; 
				fbr.Read(xx, de.string()); 
				fl.push_back( xx );
		}
		std::string oname(s + std::string("/fil/coaddtest/") + group + std::string("ea99_kur.fil"));
		// MPI_Param stuff
		struct CoaddMPI_Params param;
		param.same_for_all = false;
		param.rootpath.push_back(g + std::string("/vd1/"));
		param.rootpath.push_back(g + std::string("/vd2/"));
		param.rootpath.push_back(g + std::string("/vd3/"));
		param.rootpath.push_back(g + std::string("/vd4/"));
		param.rootpath.push_back(g + std::string("/vd5/"));
		param.rootpath.push_back(g + std::string("/vd6/"));
		param.rootpath.push_back(g + std::string("/vd7/"));
		param.rootpath.push_back(g + std::string("/vd8/"));
		param.rootpath.push_back(g + std::string("/vd9/"));
		param.rootpath.push_back(g + std::string("/vd10/"));
		param.outfile = oname;
		param.group_string = group;
		param.loadsecs = 2.0f;
		param.kur = true;
		// Object initialization 
		CoaddMPI cd;
		cd.Work(param);
		return 0;
}
