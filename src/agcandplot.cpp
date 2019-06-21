#include "asgard.hpp"
#include "Analyzer.hpp"
#include "Candplot.hpp"
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
 PathList fildirs, candirs;
 fs::path pdir;
 std::vector<int> rfiflag;
 //
 po::variables_map vm;
 po::options_description opt("Options");
 po::positional_options_description opd;
 opt.add_options()
	("help,h","Prints help")
	("fils,F", po::value<PathList>(&fildirs)->composing(), "Filterbank files")
	("cands,C", po::value<PathList>(&candirs)->composing(), "Candidate files")
	("fil-directory,f",po::value<StringVector>(&fdirs)->composing(),"Filterbank directory")
	("cand-directory,c",po::value<StringVector>(&cdirs)->composing(),"Candidate directory")
	("plot-directory,o",po::value<fs::path>(&pdir), "Plot directory,\nwhere plots will be stored")
	("group,g", po::value<StringVector>(&groups)->composing(),"Sets the group to use.\nIf option not given, all the groups found"
	 "\nwhile crawling will be used."
	)
	("xrfi,r", po::value<std::vector<int>>(&rfiflag)->composing()->default_value(std::vector<int>{1}, "1"), "0 to use standard\n1 to use kur(default)\n2 to use both");
 opd.add("group",-1);
 //
 try{
	po::store(po::command_line_parser(ac, av).options(opt).positional(opd).run(), vm);
	po::notify(vm);
	if(ac == 1 || vm.count("help")) {
	 std::cout << "------------------------------------------------------\n";
	 std::cout << "Asgard::CandPlot -- agcandplot\n";
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
 if(! (fildirs.empty() || candirs.empty())  ) {
	std::string FCGroup;
	fs::path xp = pdir;
	if(! fs::exists(xp)) fs::create_directory(xp);
	// files given manually
	assert(fildirs.size() == candirs.size());
	for(int i = 0; i < fildirs.size(); i++) {
	 FilterbankCandidate thisfbc( fildirs[i].string(), candirs[i].string() );
	 FCGroup = thisfbc.group;
	 CandPlot cp((xp / (FCGroup + std::string("_cand.png/png"))).string());
	 cp.Plot(thisfbc);
	}
 }
 else {
	AnalyzeFB fb;
	int imin = fdirs.size() < cdirs.size() ? fdirs.size() : cdirs.size();
	for(int i = 0; i < imin; i++) fb.Crawl(fdirs[i], cdirs[i]);
	if(fdirs.size() < cdirs.size()) for(int i = imin; i < cdirs.size(); i++) fb.Crawl(cdirs[i]);
	else if(cdirs.size() < fdirs.size()) for(int i = imin; i < fdirs.size(); i++) fb.Crawl(fdirs[i]);
	/*
	 *fb.Summary();
	 *for(auto& x : pasked) std::cout << x << std::endl;
	 */
	///////////////////////////////////////////////////////////////////////////////////////////
	if(groups.size() == 0) groups = fb.base;
	///////////////////////////////////////////////////////////////////////////////////////////
	// in an ideal setting
	// rfiflag.size() == groups.size() == pasked.size()
	// now starts the actual plotting
	while(rfiflag.size() <= groups.size()) rfiflag.push_back(rfiflag.back());
	// When pasked is less than group size
	// last options of pasked is copied. 
	// This is minor convenience and bad practise
	std::string strc("candplot");
	fs::path xp;
	fs::path pxp;
	for(int gin = 0; gin < groups.size(); gin++) {
	 // g = groups[gin]
	 // plot dir is pdir
	 // rfiflag tells if kur, nokur, or both
	 xp = pdir / groups[gin];
	 if(! fs::exists(xp)) fs::create_directory(xp);
	 pxp = xp / strc;
	 if(! fs::exists(pxp)) fs::create_directory(pxp);
	 // candidate plot
	 if(rfiflag[gin] == 0 || rfiflag[gin] == 2) {
		// nokur
		CandPlot cp( (pxp / (groups[gin] + std::string("_cand.png/png"))).string());
		cp.Plot(fb.fils[groups[gin]], fb.cands[groups[gin]]); 
	 }
	 if(rfiflag[gin] == 1 || rfiflag[gin] == 2) {
		// kur
		CandPlot cp( (pxp / (groups[gin] + std::string("_kur_cand.png/png"))).string());
		cp.Plot(fb.kfils[groups[gin]], fb.kcands[groups[gin]]); 
	 }
	}
 }
 return 0;
}
