
#include "asgard.hpp"
#include "Filterbank.hpp"
// Boost
#include<boost/program_options/cmdline.hpp>
#include<boost/program_options/config.hpp>
#include<boost/program_options/environment_iterator.hpp>
#include<boost/program_options/option.hpp>
#include<boost/program_options/options_description.hpp>
#include<boost/program_options/parsers.hpp>
#include<boost/program_options/positional_options.hpp>
#include<boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

int main(int ac, char * av[]){
		PathList files;
		FilterbankReader fbr;
		// po
		po::variables_map vm;
		po::options_description opt("Options");
		po::positional_options_description opd;
		// adding options
		opt.add_options()
				("files,f", po::value<PathList>(&files)->composing(), "Filterbank file(s).")
				("help,h", "Prints help");
		opd.add("files",-1);
		// parsing
		try {
				po::store(po::command_line_parser(ac,av).options(opt).run(), vm);
				po::notify(vm);
				if(ac == 1 || vm.count("files") == 0 || vm.count("help")) {
						std::cout << opt << std::endl;
				}
		}
		catch(std::exception& e) {
				std::cerr << "Error in asgard main: " << e.what() << std::endl;
				return 1;
		}
		// work
		for(auto& x : files) {
				Filterbank fb;
				fbr.Read(fb, x.string());
				std::cout << fb;
		}
		return 0;
}
