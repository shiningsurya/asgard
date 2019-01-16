#include "asgard.hpp"
#include "Analyzer.hpp"
#include<boost/program_options/cmdline.hpp>
#include<boost/program_options/config.hpp>
#include<boost/program_options/environment_iterator.hpp>
#include<boost/program_options/option.hpp>
#include<boost/program_options/options_description.hpp>
#include<boost/program_options/parsers.hpp>
#include<boost/program_options/positional_options.hpp>
#include<boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

void printsv(StringVector x) {
		for(auto ix : x) std::cout << ix << "  ";
		std::cout << std::endl;
}

int main(int ac, char * av[]){
		StringVector fdirs, cdirs, groups;
		po::variables_map vm;
		po::options_description opt("Options");
		po::positional_options_description opd;
		opt.add_options()
		("help,h","Prints help")
		("fil-directory,f",po::value<StringVector>(&fdirs)->composing(),"Filterbank directory")
		("cand-directory,c",po::value<StringVector>(&cdirs)->composing(),"Candidate directory")
		("group,g", po::value<StringVector>(&groups)->composing(),"Sets the group to use")
		("plot,p", "Plotter")
		("coincide,o", "Coincider");
		opd.add("group",-1);
		//
		try{
				po::store(po::command_line_parser(ac, av).options(opt).positional(opd).run(), vm);
				po::notify(vm);
				if(vm.count("help")) {
						std::cout << opt << std::endl;
						return 0;
				}
		}
		catch(std::exception &e) {
				std::cerr << "Error in asgard main: " << e.what() << std::endl;
				return 1;
		}
		catch(...) {
				std::cerr << "Exception of unknown type! " << std::endl;
				return 1;
		}
		//
		AnalyzeFB fb;
		int imin = fdirs.size() < cdirs.size() ? fdirs.size() : cdirs.size();
		for(int i = 0; i < imin; i++) fb.Crawl(fdirs[i], cdirs[i]);
		if(fdirs.size() < cdirs.size()) for(int i = imin; i < cdirs.size(); i++) fb.Crawl(cdirs[i]);
		if(cdirs.size() < fdirs.size()) for(int i = imin; i < fdirs.size(); i++) fb.Crawl(fdirs[i]);
		//
		fb.Groups();	
		return 0;
}
