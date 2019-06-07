#include "asgard.hpp"
#include "Coadd.hpp"
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
		bool kur;
		float loadsecs;
		StringVector groups;
		std::string dodir("/home/vlite-master/surya/dankspace/"), odir, dpre("/mnt/ssd/fildata/"), prefix;
		std::string ekfil("ea99_kur.fil"), efil("ea99.fil");
		// po
		po::variables_map vm;
		po::options_description opt("Options");
		po::positional_options_description opd;
		// adding options
		opt.add_options()
				("group,g", po::value<StringVector>(&groups)->composing(), "Groups")
				("loadsecs", po::value<float>(&loadsecs)->default_value(2.0f), "Timestep (s)")
				("xrfi,r", po::value<bool>(&kur)->default_value(true), "True for Kurtosis (def)\nOtherwise non kurtosis.")
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
		// MPI stuff
		struct CoaddMPI_Params param;
		param.same_for_all = true;
		param.rootpath.push_back(prefix);
		param.loadsecs = loadsecs;
		param.kur = kur;
		param.group_string = groups[0];
		if(param.kur) param.outfile = odir + eslash + param.group_string + ekfil;
		else param.outfile = odir + eslash + param.group_string + efil;
		// TODO multiple group work
		CoaddMPI cd;
		cd.Work(param);
		return 0;
}
