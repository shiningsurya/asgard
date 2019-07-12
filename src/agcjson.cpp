#include "asgard.hpp"
#include "Analyzer.hpp"
// FilterbankCandidate 
#include "FilterbankCandidate.hpp"
// JSON 
#include "CandidateJSON.hpp"
// CLI
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
	("output-directory,o",po::value<fs::path>(&pdir), "Output directory,\nwhere Candidate JSONs will be stored")
	("group,g", po::value<StringVector>(&groups)->composing(),"Sets the group to use.\nIf option not given, all the groups found"
	 "\nwhile crawling will be used."
	)
 opd.add("group",-1);
 //
 try{
	po::store(po::command_line_parser(ac, av).options(opt).positional(opd).run(), vm);
	po::notify(vm);
	if(ac == 1 || vm.count("help")) {
	 std::cout << "------------------------------------------------------\n";
	 std::cout << "Asgard::CandidateJSON-- agcjson\n";
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
 // create CandidateJSON
 //
 if(! (fildirs.empty() || candirs.empty())  ) {
	fs::path xp = pdir;
	CandidateJSON cjson(pdir);
	if(! fs::exists(xp)) fs::create_directory(xp);
	// files given manually
	assert(fildirs.size() == candirs.size());
	for(int i = 0; i < fildirs.size(); i++) {
	 FilterbankCandidate thisfbc( fildirs[i].string(), candirs[i].string() );
	 cjson.Write(thisfbc);
	}
 }
 else {
	AnalyzeFB fb;
	int imin = fdirs.size() < cdirs.size() ? fdirs.size() : cdirs.size();
	for(int i = 0; i < imin; i++) fb.Crawl(fdirs[i], cdirs[i]);
	if(fdirs.size() < cdirs.size()) for(int i = imin; i < cdirs.size(); i++) fb.Crawl(cdirs[i]);
	else if(cdirs.size() < fdirs.size()) for(int i = imin; i < fdirs.size(); i++) fb.Crawl(fdirs[i]);
	///////////////////////////////////////////////////////////////////////////////////////////
	if(groups.size() == 0) groups = fb.base;
	///////////////////////////////////////////////////////////////////////////////////////////
	fs::path xp;
	for(int gin = 0; gin < groups.size(); gin++) {
	 // g = groups[gin]
	 xp = pdir / groups[gin];
	 if(! fs::exists(xp)) fs::create_directory(xp);
     CandidateJSON cjson(xp);
	 // Fbc
	 FilterbankCandidate thisfbc(fb.kfils[groups[gin]], fb.kcands[groups[gin]]);
	 cjson(thisfbc);
	}
 }
 return 0;
}
