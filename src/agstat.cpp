#include "asgard.hpp"
#include "Analyzer.hpp"
// boost po
#include<boost/program_options/cmdline.hpp>
#include<boost/program_options/config.hpp>
#include<boost/program_options/environment_iterator.hpp>
#include<boost/program_options/option.hpp>
#include<boost/program_options/options_description.hpp>
#include<boost/program_options/parsers.hpp>
#include<boost/program_options/positional_options.hpp>
#include<boost/program_options/variables_map.hpp>
// console size
// TODO
#include "iomanip"

namespace po = boost::program_options;

int main(int ac, char * av[]) {
		StringVector fdirs, cdirs, groups;
		int ppr;
		po::variables_map vm;
		po::options_description opt("Options");
		po::positional_options_description opd;
		opt.add_options()
		("help,h","Prints help")
		("fil-directory,f",po::value<StringVector>(&fdirs)->composing(),"Filterbank directory")
		("cand-directory,c",po::value<StringVector>(&cdirs)->composing(),"Candidate directory")
		("group,g", po::value<StringVector>(&groups)->composing(),"Sets the group to use.\nIf option not given, all the groups found"
																"\nwhile crawling will be used."
		)
		("pretty-print,p", "Pretty prints.\n"
		 "Pretty prints aesthetic for eyes but makes it difficult to grep, cut, sed, awk later.\n"
		 "Default: no pretty print"
		);
		opd.add("group",-1);
		try{
				po::store(po::command_line_parser(ac, av).options(opt).positional(opd).run(), vm);
				po::notify(vm);
				if(ac == 1 || vm.count("help")) {
						std::cout << "------------------------------------------------------\n";
						std::cout << "Asgard::Stat -- agstat\n";
						std::cout << opt << std::endl;
						return 0;
				}
		}
		catch(std::exception& e) {
				std::cerr << "Error in asgard main: " << e.what() << std::endl;
				return 1;
		}
		catch(...) {
				std::cerr << "Exception of unknown type! " << std::endl;
				return 1;
		}
		//
		ppr = vm.count("pretty-print");
		std::string pl("+");
		std::string br("|");
		std::string kr("kur");
		std::string nkr("nokur");
		std::string fils("fils");
		std::string cands("cands");
		std::string grp("Group");
		std::string src("Source");
		std::string tab("\t");
		//
		AnalyzeFB fb;
		int imin = fdirs.size() < cdirs.size() ? fdirs.size() : cdirs.size();
		for(int i = 0; i < imin; i++) fb.Crawl(fdirs[i], cdirs[i]);
		if(fdirs.size() < cdirs.size()) for(int i = imin; i < cdirs.size(); i++) fb.Crawl(cdirs[i]);
		else if(cdirs.size() < fdirs.size()) for(int i = imin; i < fdirs.size(); i++) fb.Crawl(fdirs[i]);
		///////////////////////////////////////////////////////////////////////////////////////////
		if(groups.size() == 0) groups = fb.base;
		///////////////////////////////////////////////////////////////////////////////////////////
		// Group -- SRC -- # kur fils -- # nokur fils -- # kur cands -- # nokur cands
		// 20121221_162020_muos -- J2145-0750 -- 420 -- 420 -- 420 -- 420
		if(ppr) {
				std::cout << pl << std::setfill('-') << std::setw(22) << pl << std::setfill('-') << std::setw(12);
				std::cout << pl << std::setfill('-') << std::setw(6); // kur fils
				std::cout << pl << std::setfill('-') << std::setw(6); // nokur fils
				std::cout << pl << std::setfill('-') << std::setw(6); // kur cands
				std::cout << pl << std::setfill('-') << std::setw(6); // nokur cands 
				std::cout << pl << std::endl;
				//
				std::cout << br << std::right << std::setw(22) << grp << pl << std::right << std::setw(12) << src;
		}
		else {
				for(auto& g: groups) {
						std::cout << g << tab;
						std::cout << fb.kfils[g].size() << tab;
						std::cout << fb.fils[g].size() << tab;
						std::cout << fb.kcands[g].size() << tab;
						std::cout << fb.cands[g].size() << tab;
						std::cout << std::endl;
				}
		}
		return 0;
}
