#include "asgard.hpp"
#include "Operations.hpp"
#include "FilterbankJSON.hpp"
// boost program options
#include<boost/program_options/cmdline.hpp>
#include<boost/program_options/config.hpp>
#include<boost/program_options/environment_iterator.hpp>
#include<boost/program_options/option.hpp>
#include<boost/program_options/options_description.hpp>
#include<boost/program_options/parsers.hpp>
#include<boost/program_options/positional_options.hpp>
#include<boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

int main(int ac, char * av[]) {
  bool plotme = false;
  bool headme = false;
  std::string oplot, dstr("fbsons_candplot.png/png");
  StringVector args;
		// po
		po::variables_map vm;
		po::options_description opt("Options");
		po::positional_options_description opd;
		// adding options
		opt.add_options()
("help,h", "Prints help")
("args,f",po::value<StringVector>(&args)->composing(), "FBSONs files")
("plot,p", "Plots file")
("oplot,o", po::value<std::string>(&oplot)->default_value(dstr),"Plot signature.");
		opd.add("args",-1);
		// parsing
		try {
				po::store(po::command_line_parser(ac,av).options(opt).positional(opd).run(), vm);
				po::notify(vm);
				if(vm.count("help")) {
      std::cout << "Asgard::agfbson Filterbankdump plotting/printing utility." << std::endl;
      std::cout << std::endl;
      std::cout << opt << std::endl;
      std::cout << "Part of Asgard" << std::endl;
						return 0;
				}
				else if(vm.count("plot")) {
				  plotme = true;
				}
				else {
				  headme = true;
				}
		}
		catch(std::exception& e) {
				std::cerr << "Error in asgard main: " << e.what() << std::endl;
				return 1;
  }
  // work
  if(plotme) {
    FBDPlot myp(oplot);
    for(const auto& arg : args) {
      FBDump fbd(arg); 
      myp.Plot1(fbd);
    }
  }
  if(headme) {
    for(const auto& arg : args) {
      FBDump fbd(arg); 
      std::cout << fbd;
    }
  }
  return 0;
}
