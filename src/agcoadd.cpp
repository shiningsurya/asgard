#include "asgard.hpp"
#include "Analyzer.hpp"
#include "Filterbank.hpp"
#include "Coadd.hpp"
// CLI stuff
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
		StringVector fdirs, groups;
		fs::path odir;
		int xrfichoice;
		std::vector<int> rfiflag;
		//
		po::variables_map vm;
		po::options_description opt("Options");
		po::positional_options_description opd;
		opt.add_options()
				("help,h","Prints help")
				("rfi-help,H","Prints RFI Excision help")
				("fil-directories,f",po::value<StringVector>(&fdirs)->composing(),"Filterbank directory")
				("out-directory,o",po::value<fs::path>(&odir), "Directory where coadded files will be written.")
				("group,g", po::value<StringVector>(&groups)->composing(),
				 "Sets the group to use.\n"
				 "If option not given, all the groups found\n"
				 "while crawling will be used."
				)
				("rfi-excision,x", po::value<int>(&xrfichoice)->default_value(0), 
				 "0 : no RFI-excision\n"
				 "1 : MAD based RFI-excision\n"
				 "2 : Histogram based RFI-excision"
				)
				("xrfi,r", po::value<std::vector<int>>(&rfiflag)->composing()->default_value(std::vector<int>{1}, "1"), "0 to use standard\n1 to use kur(default)\n2 to use both");
		opd.add("group",-1);
		po::options_description xopt("xRFI options");
		float tfac, ffac;
		float loadsecs;
		xopt.add_options()
				("load-secs", po::value<float>(&loadsecs)->default_value(2.0f), "Step size for RFI excision.")
				("TFAC", po::value<float>(&tfac)->default_value(1.0f),"Multiplicative factor for Time flagging.")
				("FFAC", po::value<float>(&ffac)->default_value(1.0f),"Multiplicative factor for Freq flagging.");
		///
		po::options_description mainopt;
		mainopt.add(opt).add(xopt);
		//
		try{
				po::store(po::command_line_parser(ac, av).options(mainopt).positional(opd).run(), vm);
				po::notify(vm);
				if(ac == 1 ||  vm.count("group") == 0 || vm.count("help") || vm.count("rfi-help")) {
						std::cout << "------------------------------------------------------\n";
						std::cout << "Asgard::Coadd -- agcodd\n";
						if(vm.count("help")) std::cout << opt << std::endl;
						if(vm.count("rfi-help")) std::cout << xopt << std::endl;
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
		AnalyzeFB fb;
		for(auto& fd : fdirs) fb.Crawl(fd);
		std::string fn, ea0k("_ea00_kur.fil"), ea0("_ea00.fil");
		for(int gin = 0; gin < groups.size(); gin++) {
				if(rfiflag[gin] == 0 || rfiflag[gin] == 2) {
						// nokur
						fn = groups[gin] + ea0;
						if(xrfichoice) {
								Coadd wf(fn, loadsecs, tfac, ffac, xrfichoice);
								wf.coadd(fb.fils[groups[gin]]);	
						}
						else {
								Coadd wf(fn, loadsecs);
								wf.coadd(fb.kfils[groups[gin]]);	
						}
				}
				if(rfiflag[gin] == 1 || rfiflag[gin] == 2) {
						// kur
						fn = groups[gin] + ea0k;
						if(xrfichoice) {
								Coadd wf(fn, loadsecs, tfac, ffac, xrfichoice);
								wf.coadd(fb.kfils[groups[gin]]);	
						}
						else {
								Coadd wf(fn, loadsecs);
								wf.coadd(fb.kfils[groups[gin]]);	
						}
				}
		}
		return 0;
}
