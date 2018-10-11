#include <iostream>
#include "asgard.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/bind.hpp>

class AnalyzeFB {
		public:
		AnalyzeFB(std::string wd) {
				// This is the only constructor
				workdir = wd; // The work directory
				fildir = workdir / std::string("fil");    // filterbank 
				candir = workdir / std::string("cands");  // candidates
				plotdir = workdir / std::string("plots"); // plots
				//
				DEList flist, clist, klist; // directories not objects 
				GroupList base; // has the groups 
				std::for_each(fs::directory_iterator(fildir),fs::directory_iterator(),boost::bind(AnalyzeFB::dselbinder,base,flist,std::string(".fil"), std::string("_kur.fil"),_1));
				std::for_each(fs::directory_iterator(fildir),fs::directory_iterator(),boost::bind(AnalyzeFB::selbinder,base,klist,std::string(".fil"), std::string("_kur.fil"),_1));
				std::for_each(fs::directory_iterator(candir),fs::directory_iterator(),boost::bind(AnalyzeFB::selbinder,base,flist,std::string(".cand"), std::string(".cand"),_1));
				// taking out duplicates
				std::sort(base.begin(),base.end()); 
				//base.erase( std::unique( base.begin(), base.end() ), base.end() );
				std::cout << "Size of Base: " << base.size() << std::endl;//
				DEList tv;
				PairGroupDE ttp;
				for(std::string b : base) {
						std::cout << "e: " << b << std::endl;
						std::for_each(flist.begin(),flist.end(),boost::bind(AnalyzeFB::binder,b,tv,_1));
						ttp = std::make_pair(b,tv);
						for(fs::directory_entry fde : tv) std::cout << "d: " << fde.path().string() << std::endl;
						fils.insert(ttp);
						//fils[*b] = tv;
						tv.clear();
						// fils done.. now have to do kfils and cands
						std::for_each(clist.begin(),clist.end(),boost::bind(AnalyzeFB::binder,b,tv,_1));
						ttp = std::make_pair(b,tv);
						cands.insert(ttp);
						//cands[*b] = tv;
						tv.clear();
						// cands and fils done
						std::for_each(klist.begin(),klist.end(),boost::bind(AnalyzeFB::binder,b,tv,_1));
						//kfils[*b] = tv;
						ttp = std::make_pair(b,tv);
						kfils.insert(ttp);
						tv.clear();
				}
		}
		private:
				MapGroupDE fils, kfils, cands;
				fs::path workdir, fildir, candir, plotdir;
				static void binder(std::string b, DEList& tv, fs::directory_entry it) {
				// If b is found in *it push back to tv
						std::string r = it.path().filename().string();
						if(r.find(b) != std::string::npos)
								(tv).push_back(it);
				}
				static void selbinder(GroupList& base, DEList list, std::string alpha, std::string beta, fs::directory_entry one) {
						std::cout << "SELBINDER: " << one.path().string() << std::endl;
						if(one.path().extension().string() == alpha) {
								if(one.path().string().find(beta) != std::string::npos) {
										list.push_back(one);
										boost::iter_split(base, one.path().filename().string(), boost::algorithm::first_finder(std::string("_ea")));
										//boost::split(base,one.path().filename().string(),boost::is_any_of("_ea"));
										(base).pop_back();		// To remove the last part. after _ea ....
								}
						}
				}
				static void dselbinder(GroupList& base, DEList list, std::string alpha, std::string beta, fs::directory_entry one) {
						std::cout << "DSELBINDER: " << one.path().string() << std::endl;
						if(one.path().extension() == alpha) {
								if(one.path().string().find(beta) == std::string::npos) {
										list.push_back(one);
										boost::iter_split(base, one.path().filename().string(), boost::algorithm::first_finder(std::string("_ea")));
										//boost::split(base,one.path().filename().string(),boost::is_any_of("_ea"));
										(base).pop_back();		// To remove the last part. after _ea ....
								}
						}
				}
		public:
		void PrintPaths() {
				std::cout << "Workdir: " << workdir.string() << std::endl;
				std::cout << "Fildir: " << fildir.string() << std::endl;
				std::cout << "Candir: " << candir.string() << std::endl;
				std::cout << "Plotdir: " << plotdir.string() << std::endl;
		}
		void Summary() {
				std::cout << "Summary\n";
				for(PairGroupDE it : fils) std::cout << "base: " << it.first << std::endl;
				for(PairGroupDE it : kfils) std::cout << "base: " << it.first << std::endl;
				for(PairGroupDE it : cands) std::cout << "base: " << it.first << std::endl;
		}
};


int main() {
		std::string s("/home/shining/study/MS/vLITE/mkerr");
		AnalyzeFB f(s);
		//f.PrintPaths();
		f.Summary();
		return 0;
}
