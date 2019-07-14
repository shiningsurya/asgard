#include "asgard.hpp"
#include "MPIPlot.hpp"
#include "MPIWaterfallCandidate.hpp"
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

int main(int ac, char * av[]) {
 bool kur, wplot;
 float loadsecs;
 StringVector groups;
 std::string dodir("/home/vlite-master/surya/dankspace/"), odir, dpre("/mnt/ssd/fildata/"), prefix;
 std::string ekpng("kur_waterfall.png/png"), epng("waterfall.png/png");
 std::string outfile;
 // po
 po::variables_map vm;
 po::options_description opt("Options");
 po::positional_options_description opd;
 // adding options
 opt.add_options()
	("group,g", po::value<StringVector>(&groups)->composing(), "Groups")
	("loadsecs", po::value<float>(&loadsecs)->default_value(4.0f), "Timestep (s)")
	("xrfi,r", po::value<bool>(&kur)->default_value(true), "True for Kurtosis (def)\nOtherwise non kurtosis.")
	("Wplot,W", po::value<bool>(&wplot)->default_value(true), "Overlay candidates w/ sn>=10")
	("odir,o", po::value<std::string>(&odir)->default_value(dodir), "Output directory")
	("prefix", po::value<std::string>(&prefix)->default_value(dpre), "Prefix for fildata.")
	("help,h", "Prints help");
 opd.add("group",-1);
 try {
	po::store(po::command_line_parser(ac,av).options(opt).positional(opd).run(), vm);
	po::notify(vm);
	if(ac == 1 || vm.count("group") == 0 || vm.count("help")) {
	 std::cout << opt << std::endl;
	 return 0;
	}
 }
 catch(std::exception& e) {
	std::cerr << "Error in asgard main: " << e.what() << std::endl;
	return 1;
 }
 for(auto& g : groups) {
 	outfile = odir + eslash + g;
 	if(kur) outfile +=  ekpng;
 	else outfile += epng; 
	if(wplot){
	 MPIWaterfallCandidate mp(outfile, prefix, 0, true);
	 mp.Plot(g, kur, loadsecs);
	}
	else{
	 MPIPlot mp(outfile, prefix, 0);
	 mp.Plot(g, kur, loadsecs);
	}
 }
 return 0;
}
