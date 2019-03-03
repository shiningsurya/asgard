#include "asgard.hpp"
#include "Analyzer.hpp"
#include "Filplot.hpp"
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

void printsv(StringVector x) {
		for(auto ix : x) std::cout << ix << "  ";
		std::cout << std::endl;
}

int main(int ac, char * av[]){
		StringVector fdirs, cdirs, groups;
		StringVector pasked;
		fs::path pdir;
		std::vector<int> rfiflag;
		//
		po::variables_map vm;
		po::options_description opt("Options");
		po::positional_options_description opd;
		opt.add_options()
				("help,h","Prints help")
				("fil-directory,f",po::value<StringVector>(&fdirs)->composing(),"Filterbank directory")
				("cand-directory,c",po::value<StringVector>(&cdirs)->composing(),"Candidate directory")
				("plot-directory,o",po::value<fs::path>(&pdir), "Plot directory,\nwhere plots will be stored")
				("group,g", po::value<StringVector>(&groups)->composing(),"Sets the group to use.\nIf option not given, all the groups found"
				 "\nwhile crawling will be used."
				)
				("xrfi,r", po::value<std::vector<int>>(&rfiflag)->composing()->default_value(std::vector<int>{1}, "1"), "0 to use standard\n1 to use kur(default)\n2 to use both");
		opd.add("group",-1);
		//
		double tsamp;
		float timestep, ctimestep;
		int fschanout;
		int xrfimet;
		po::options_description popt("Plotting options");
		popt.add_options()
				("candsum,s", "Candidate summary plot")
				("waterfall,w","Waterfall plot")
				("dplot,d", "Waterfall, bandshape, timeshape plot")
				("coarse waterfall,q", "Coarse waterfall plot")
				("tsamp",po::value<double>(&tsamp)->default_value(97e-6),"Time per sample")
				("chanout",po::value<int>(&fschanout)->default_value(32), "Fscrunching channels")
				("timestep",po::value<float>(&timestep)->default_value(1.0f), "Timestep in Waterfall in seconds")
				("rfi-xcision,x", po::value<int>(&xrfimet)->default_value(0), "1: MAD\n2: Histogram\n0: no excision")
				("coarse-timestep",po::value<float>(&ctimestep)->default_value(1e-2f), "Coarse time resolution\nPassing 0.0f would make a single plot for the entire duration.")
				("plot,p", po::value<StringVector>(&pasked)->composing(), "If you have more than one plotting requirement. For example,\n"
				 "sd: candidate summary and dplot;\n"
				 "sw: candidate summary and waterfall plot;\n"
				 "sdw: candidate summary, dplot and waterfall plot.\n");
		//
		po::options_description xopt("xRFI options");
		float tfac, ffac;
		xopt.add_options()
				("TFAC", po::value<float>(&tfac)->default_value(1.0f),"Multiplicative factor for Time flagging.")
				("FFAC", po::value<float>(&ffac)->default_value(1.0f),"Multiplicative factor for Freq flagging.");
		///
		po::options_description tp;
		tp.add(opt).add(popt).add(xopt);
		//
		try{
				po::store(po::command_line_parser(ac, av).options(tp).positional(opd).run(), vm);
				po::notify(vm);
				if(ac == 1 || vm.count("plot") == 0 || vm.count("help")) {
						std::cout << "------------------------------------------------------\n";
						std::cout << "Asgard::Plot -- agplot\n";
						if(vm.count("help")) {
								std::cout << tp ;
								std::cout << xopt ;
						}
						else if(vm.count("plot") == 0) {
								std::cout << popt << std::endl;
						}
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
		int imin = fdirs.size() < cdirs.size() ? fdirs.size() : cdirs.size();
		for(int i = 0; i < imin; i++) fb.Crawl(fdirs[i], cdirs[i]);
		if(fdirs.size() < cdirs.size()) for(int i = imin; i < cdirs.size(); i++) fb.Crawl(cdirs[i]);
		else if(cdirs.size() < fdirs.size()) for(int i = imin; i < fdirs.size(); i++) fb.Crawl(fdirs[i]);
		///////////////////////////////////////////////////////////////////////////////////////////
		if(groups.size() == 0) groups = fb.base;
		///////////////////////////////////////////////////////////////////////////////////////////
		// in an ideal setting
		// rfiflag.size() == groups.size() == pasked.size()
		// now starts the actual plotting
		while(pasked.size() <= groups.size()) pasked.push_back(pasked.back());
		while(rfiflag.size() <= groups.size()) rfiflag.push_back(rfiflag.back());
		// When pasked is less than group size
		// last options of pasked is copied. 
		// This is minor convenience and bad practise
		////
		std::string strcs("candsummary"), strd("dplot"), strwf("waterfall"), strcwf("coarse_waterfall");
		for(int gin = 0; gin < groups.size(); gin++) {
				// g = groups[gin]
				// plot dir is pdir
				// rfiflag tells if kur, nokur, or both
				fs::path xp = pdir / groups[gin];
				fs::path pxp;
				for(char& pin : pasked[gin]) {
						if(! fs::exists(xp)) fs::create_directory(xp);
						if(pin == 's') {
								// candidate summary plot
								if(! fs::exists(xp / strcs)) fs::create_directory(xp / strcs);
								pxp = xp / strcs;
								if(rfiflag[gin] == 0 || rfiflag[gin] == 2) {
										// nokur
										CandSummary cs((pxp / (groups[gin] + std::string("_candsummary.png/png"))).string(), tsamp);
										cs.Plot(fb.cands[groups[gin]]);	
								}
								if(rfiflag[gin] == 1 || rfiflag[gin] == 2) {
										// kur
										CandSummary cs( (pxp / (groups[gin] + std::string("_kur_candsummary.png/png"))).string(), tsamp);
										cs.Plot(fb.kcands[groups[gin]]);	
								}
						}
						else if(pin == 'd') {
								// dplot
								if(! fs::exists(xp / strd)) fs::create_directory(xp / strd);
								pxp = xp / strd;
								if(rfiflag[gin] == 0 || rfiflag[gin] == 2) {
										// nokur
										for(auto ff : fb.fils[groups[gin]]) {
												DPlot cp( (pxp / (groups[gin] + std::string("_") + GetAntenna(ff.path().string()) + std::string("_dplot.png/png"))).string(),timestep, fschanout , ffac, tfac);
												cp.Plot(ff, xrfimet);	

										}
								}
								if(rfiflag[gin] == 1 || rfiflag[gin] == 2) {
										// kur
										for(auto ff : fb.kfils[groups[gin]]) {
												DPlot cp( (pxp / (groups[gin] + std::string("_")+GetAntenna(ff.path().string()) + std::string("_kur_dplot.png/png"))).string(),timestep, fschanout, ffac, tfac );
												cp.Plot(ff, xrfimet);	

										}
								}
						}
						else if(pin == 'w') {
								// waterfall plot
								if(! fs::exists(xp / strwf)) fs::create_directory(xp / strwf);
								pxp = xp / strwf;
								if(rfiflag[gin] == 0 || rfiflag[gin] == 2) {
										// nokur
										Waterfall wf( (pxp / (groups[gin] + std::string("_waterfall.png/png"))).string(), timestep, fschanout);
										wf.Plot(fb.fils[groups[gin]]);	
								}
								if(rfiflag[gin] == 1 || rfiflag[gin] == 2) {
										// kur
										Waterfall wf( (pxp / (groups[gin] + std::string("_kur_waterfall.png/png"))).string(), timestep, fschanout);
										wf.Plot(fb.kfils[groups[gin]]);	
								}
						}
						else if(pin == 'q') {
								// coarse waterfall plot
								if(! fs::exists(xp / strcwf)) fs::create_directory(xp / strcwf);
								pxp = xp / strcwf;
								if(rfiflag[gin] == 0 || rfiflag[gin] == 2) {
										// nokur
										Waterfall wf( (pxp / (groups[gin] + std::string("_coarse_waterfall.png/png"))).string(), timestep, ctimestep, fschanout);
										wf.Plot(fb.fils[groups[gin]]);	
								}
								if(rfiflag[gin] == 1 || rfiflag[gin] == 2) {
										// kur
										Waterfall wf( (pxp / (groups[gin] + std::string("_kur_coarse_waterfall.png/png"))).string(), timestep, ctimestep, fschanout);
										wf.Plot(fb.kfils[groups[gin]]);	
								}
						}
				}
		}
		return 0;
}
