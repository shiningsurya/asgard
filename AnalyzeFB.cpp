#include <iostream>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/bind.hpp>
#include <string>
#include <map>
#include <utility>

namespace fs =  boost::filesystem;

typedef std::map<std::string, std::vector<fs::directory_entry>> mymap; 
typedef std::pair<std::string, std::vector<fs::directory_entry>> mypair;

class AnalyzeFB {
		public:
		AnalyzeFB(std::string wd) {
				// This is the only constructor
				workdir = wd;
				fildir = workdir / std::string("fil");
				candir = workdir / std::string("cands");
				plotdir = workdir / std::string("plots");
				//
				//std::vector<fs::directory_entry> fl, cl;
				//copy(fs::directory_iterator(fildir),fs::directory_iterator(), std::back_inserter(fl));
				//copy(fs::directory_iterator(candir),fs::directory_iterator(), std::back_inserter(cl));
				std::vector<fs::directory_entry> flist, clist, klist; 
				std::vector<std::string> base; 
				// flist, clist, klist contain full directories
				// base contain the group
				std::for_each(fs::directory_iterator(fildir),fs::directory_iterator(),boost::bind(AnalyzeFB::dselbinder,&base,flist,std::string(".fil"), std::string("_kur.fil"),_1));
				std::for_each(fs::directory_iterator(fildir),fs::directory_iterator(),boost::bind(AnalyzeFB::selbinder,&base,klist,std::string(".fil"), std::string("_kur.fil"),_1));
				std::for_each(fs::directory_iterator(candir),fs::directory_iterator(),boost::bind(AnalyzeFB::selbinder,&base,flist,std::string(".cand"), std::string(".cand"),_1));
				// taking out duplicates
				std::sort(base.begin(),base.end()); 
				base.erase( std::unique( base.begin(), base.end() ), base.end() );
				//
				std::vector<fs::directory_entry> tv;
				mypair ttp;
				for(std::vector<std::string>::iterator b = base.begin(); b != base.end(); b++) {
						std::for_each(flist.begin(),flist.end(),boost::bind(AnalyzeFB::binder,b,tv,_1));
						ttp = std::make_pair(*b,tv);
						fils.insert(ttp);
						for(std::string i : base ) std::cout << i << " " ; 
						std::cout << std::endl;
						//fils[*b] = tv;
						tv.clear();
						// fils done.. now have to do kfils and cands
						std::for_each(clist.begin(),clist.end(),boost::bind(AnalyzeFB::binder,b,tv,_1));
						ttp = std::make_pair(*b,tv);
						cands.insert(ttp);
						//cands[*b] = tv;
						tv.clear();
						// cands and fils done
						std::for_each(klist.begin(),klist.end(),boost::bind(AnalyzeFB::binder,b,tv,_1));
						//kfils[*b] = tv;
						ttp = std::make_pair(*b,tv);
						kfils.insert(ttp);
						tv.clear();
				}

		}
		private:
				std::map<std::string, std::vector<fs::directory_entry>> fils, kfils, cands;
				fs::path workdir, fildir, candir, plotdir;
				static void binder(std::vector<std::string>::iterator b, std::vector<fs::directory_entry> tv, fs::directory_entry it) {
				// If b is found in *it push back to tv
						std::string r = it.path().filename().string();
						if(r.find(*b) != std::string::npos)
								(tv).push_back(it);
				}
				static void selbinder(std::vector<std::string> *base, std::vector<fs::directory_entry> list, std::string alpha, std::string beta, fs::directory_entry one) {
						if(one.path().extension().string() == alpha) {
								if(one.path().string().find(beta) != std::string::npos) {
										list.push_back(one);
										boost::iter_split(*base, one.path().filename().string(), boost::algorithm::first_finder(std::string("_ea")));
										//boost::split(base,one.path().filename().string(),boost::is_any_of("_ea"));
										(*base).pop_back();		// To remove the last part. after _ea ....
								}
						}
				}
				static void dselbinder(std::vector<std::string> *base, std::vector<fs::directory_entry> list, std::string alpha, std::string beta, fs::directory_entry one) {
						if(one.path().extension() == alpha) {
								if(one.path().string().find(beta) == std::string::npos) {
										list.push_back(one);
										boost::iter_split(*base, one.path().filename().string(), boost::algorithm::first_finder(std::string("_ea")));
										//boost::split(base,one.path().filename().string(),boost::is_any_of("_ea"));
										(*base).pop_back();		// To remove the last part. after _ea ....
								}
						}
				}
		public:
		void PrintPaths() {
				std::cout << "Workdir: " << workdir.string() << std::endl;
				std::cout << "Fildir: " << fildir.string() << std::endl;
				std::cout << "Candir: " << candir.string() << std::endl;
				std::cout << "Plotdir: " << plotdir.string() << std::endl;
				// simply iterating over fildir for fun
				
				//copy(fs::directory_iterator(fildir),fs::directory_iterator(),std::ostream_iterator<fs::directory_entry>(std::cout,"\n"));
		}
		void Summary() {
				std::cout << "Summary?\n";
				for(mymap::iterator it = fils.begin(); it != fils.end(); it++) std::cout << "base: " << it->first << std::endl;
				for(mymap::iterator it = kfils.begin(); it != kfils.end(); it++) std::cout << "base: " << it->first << std::endl;
				for(mymap::iterator it = cands.begin(); it != cands.end(); it++) std::cout << "base: " << it->first << std::endl;
		}
};


int main() {
		std::string s("/home/shining/study/MS/vLITE/mkerr");
		AnalyzeFB f(s);
		f.PrintPaths();
		f.Summary();
		return 0;
}
