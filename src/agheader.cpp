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
		timeslice unpack;
		FilterbankReader fbr;
		// po
		po::variables_map vm;
		po::options_description opt("Options");
		po::positional_options_description opd;
		// adding options
		opt.add_options()
				("files,f", po::value<PathList>(&files)->composing(), "Filterbank file(s).")
				("unpack,u", po::value<timeslice>(&unpack), "Data sample requested.")
				("src,s", po::bool_switch()->default_value(false), "print src_raj/src_dej.")
				("name,n", po::bool_switch()->default_value(false), "print source_name.")
				("group,Q", po::bool_switch()->default_value(false), "prints group in first column.")
				("help,h", "Prints help");
		opd.add("files",-1);
		// parsing
		try {
				po::store(po::command_line_parser(ac,av).options(opt).positional(opd).run(), vm);
				po::notify(vm);
				if(ac == 1 || vm.count("files") == 0 || vm.count("help")) {
						std::cout << opt << std::endl;
						return 0;
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
				if(vm.count("unpack")) {
						PtrFloat da = new float [ unpack * fb.nchans  ];
						fb.Unpack(da, 1L, unpack);
						for(timeslice ii = 0; ii < unpack*fb.nchans; ii++) std::cout << da[ii] << "  ";
						std::cout << std::endl;
						delete[] da;
				}
				else if(vm["src"].as<bool>()) {
						if(vm.count("group")) std::cout << fb.group << "\t";
						std::cout << fb.src_raj << "\t" << fb.src_dej << std::endl;
				}
				else if(vm["name"].as<bool>()) {
						if(vm.count("group")) std::cout << fb.group << "\t";
						std::cout << fb.source_name << "\t" << fb.duration << std::endl;
				}
				else {
						std::cout << fb;
				}
		}
		return 0;
}
