#include "asgard.hpp"
#include "Analyzer.hpp"
#include "Filterbank.hpp"
#include "Candidate.hpp"
#include "FilterbankCandidate.hpp"
#include "Plotter.hpp"

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
		("group,g", po::value<StringVector>(&groups)->composing(),"Sets the group to use")
		("xrfi,r", po::value<std::vector<int>>(&rfiflag)->composing(), "0 to use standard\n1 to use kur\n2 to use both");
		opd.add("group",-1);
		//
		double tsamp;
		int fschanout;
		po::options_description popt("Plotting options");
		popt.add_options()
		("candsum,s", "Candidate summary plot")
		("tsamp",po::value<double>(&tsamp)->default_value(97e-6),"Time per sample")
		("candplot,l","Candidate plot")
		("waterfall,w","Waterfall plot")
		("chanout",po::value<int>(&fschanout)->default_value(32), "Fscrunching channels")
		("plot,p", po::value<StringVector>(&pasked)->composing(), "If you have more than one plotting requirement. For example,\n"
		 "sl: candidate summary and candidate plot;\n"
		 "lw: candidate and waterfall plot;\n"
		 "slw: candidate summary, candidate plot and waterfall plot.\n");
		//
		po::options_description tp;
		tp.add(opt).add(popt);
		//
		try{
				po::store(po::command_line_parser(ac, av).options(tp).positional(opd).run(), vm);
				po::notify(vm);
				if(vm.count("help") || ac == 1) {
						std::cout << "------------------------------------------------------\n";
						std::cout << "Asgard::Plot -- agplot\n";
						std::cout << tp << std::endl;
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
		if(cdirs.size() < fdirs.size()) for(int i = imin; i < fdirs.size(); i++) fb.Crawl(fdirs[i]);
		//fb.Summary();
		///////////////////////////////////////////////////////////////////////////////////////////
		// in an ideal setting
		// rfiflag.size() == groups.size() == pasked.size()
		// now starts the actual plotting
		if(pasked.size() < groups.size()) std::copy(pasked.end(), 
						pasked.end() - (groups.size() - pasked.size()),
						pasked.end());
		if(rfiflag.size() < groups.size()) std::copy(rfiflag.end(), 
						rfiflag.end() - (groups.size() - rfiflag.size()),
						rfiflag.end());
		// When pasked is less than group size
		// last options of pasked is copied. 
		// This is minor convenience and bad practise
		for(int gin = 0; gin < groups.size(); gin++) {
				// g = groups[gin]
				// plot dir is pdir
				// rfiflag tells if kur, nokur, or both
				for(char& pin : pasked[gin]) {
						if(pin == 's') {
								// candidate summary plot
								if(rfiflag[gin] == 0 || rfiflag[gin] == 2) {
										// nokur
										CandSummary cs( (pdir / (groups[gin] + std::string("_candsummary.png/png"))).string(), tsamp);
										cs.Plot(fb.cands[groups[gin]]);	
								}
								if(rfiflag[gin] == 1 || rfiflag[gin] == 2) {
										// kur
										CandSummary cs( (pdir / (groups[gin] + std::string("_kur_candsummary.png/png"))).string(), tsamp);
										cs.Plot(fb.kcands[groups[gin]]);	
								}
						}
						else if(pin == 'l') {
								// candidate plot
								if(rfiflag[gin] == 0 || rfiflag[gin] == 2) {
										// nokur
										CandPlot cp( (pdir / (groups[gin] + std::string("_cand.png/png"))).string());
										cp.Plot(fb.fils[groups[gin]], fb.cands[groups[gin]]);	
								}
								if(rfiflag[gin] == 1 || rfiflag[gin] == 2) {
										// kur
										CandPlot cp( (pdir / (groups[gin] + std::string("_kur_cand.png/png"))).string());
										cp.Plot(fb.kfils[groups[gin]], fb.kcands[groups[gin]]);	
								}
						}
						else if(pin == 'w') {
								// waterfall plot
								if(rfiflag[gin] == 0 || rfiflag[gin] == 2) {
										// nokur
										Waterfall wf( (pdir / (groups[gin] + std::string("_waterfall.png/png"))).string(), fschanout);
										wf.Plot(fb.fils[groups[gin]]);	
								}
								if(rfiflag[gin] == 1 || rfiflag[gin] == 2) {
										// kur
										Waterfall wf( (pdir / (groups[gin] + std::string("_kur_waterfall.png/png"))).string(), fschanout);
										wf.Plot(fb.kfils[groups[gin]]);	
								}
						}
				}
		}
		return 0;
}
