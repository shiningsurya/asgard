#include "asgard.hpp"
#include <boost/algorithm/string.hpp>
#include <set>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/bind.hpp>

class AnalyzeFB {
		public:
		MapGroupDE fils, kfils, cands;
		AnalyzeFB(std::string wd) {
				// This is the only constructor
				workdir = wd; // The work directory
				fildir = workdir / std::string("fil");    // filterbank 
				candir = workdir / std::string("cands");  // candidates
				plotdir = workdir / std::string("plots"); // plots
				//
				DEList flist, clist, klist; // directories not objects 
				GroupList base; // has the groups
				/*
				 * boost::bind should get boost::ref or else boost::bind creates internal references
				 * time taken to learn that 25minutes
				 * learnt it after reading a SO answer
				 */
				std::for_each(fs::directory_iterator(fildir),fs::directory_iterator(),boost::bind(AnalyzeFB::dselbinder,boost::ref(base),boost::ref(flist),std::string(".fil"), std::string("_kur.fil"),_1));
				std::for_each(fs::directory_iterator(fildir),fs::directory_iterator(),boost::bind(AnalyzeFB::selbinder,boost::ref(base),boost::ref(klist),std::string(".fil"), std::string("_kur.fil"),_1));
				std::for_each(fs::directory_iterator(candir),fs::directory_iterator(),boost::bind(AnalyzeFB::selbinder,boost::ref(base),boost::ref(clist),std::string(".cand"), std::string(".cand"),_1));
				// taking out duplicates
				std::set<std::string> bset( base.begin(), base.end() );
				base.assign( bset.begin(), bset.end() );
				//
				DEList tv;
				PairGroupDE ttp;
				for(std::string b : base) {
						std::for_each(flist.begin(),flist.end(),boost::bind(AnalyzeFB::binder,b,boost::ref(tv),_1));
						ttp = std::make_pair(b,tv);
						fils.insert(ttp);
						//fils[*b] = tv;
						tv.clear();
						// fils done.. now have to do kfils and cands
						std::for_each(clist.begin(),clist.end(),boost::bind(AnalyzeFB::binder,b,boost::ref(tv),_1));
						ttp = std::make_pair(b,tv);
						cands.insert(ttp);
						//cands[*b] = tv;
						tv.clear();
						// cands and fils done
						std::for_each(klist.begin(),klist.end(),boost::bind(AnalyzeFB::binder,b,boost::ref(tv),_1));
						//kfils[*b] = tv;
						ttp = std::make_pair(b,tv);
						kfils.insert(ttp);
						tv.clear();
				}
		}
		AnalyzeFB(std::string wd, std::string g) : AnalyzeFB(wd) {
				for(auto it = fils.begin(); it != fils.end(); it++)   if(it->first != g) fils.erase(it);
				for(auto it = kfils.begin(); it != kfils.end(); it++) if(it->first != g) kfils.erase(it);
				for(auto it = cands.begin(); it != cands.end(); it++) if(it->first != g) cands.erase(it);
		}
		private:
				fs::path workdir, fildir, candir, plotdir;
				static void binder(std::string b, DEList& tv, fs::directory_entry it) {
				// If b is found in *it push back to tv
						std::string r = it.path().filename().string();
						if(r.find(b) != std::string::npos)
								(tv).push_back(it);
				}
				static void selbinder(GroupList& b3, DEList& list, std::string alpha, std::string beta, fs::directory_entry one) {
						if(one.path().extension().string() == alpha) {
								if(one.path().string().find(beta) != std::string::npos) {
										list.push_back(one);
										GroupList bb;
										boost::iter_split(bb, one.path().filename().string(), boost::algorithm::first_finder(std::string("_ea")));
										bb.pop_back();		// To remove the last part. after _ea ....
										b3.insert(b3.end(),bb.begin(),bb.end());
								}
						}
				}
				static void dselbinder(GroupList& b3, DEList& list, std::string alpha, std::string beta, fs::directory_entry one) {
						if(one.path().extension() == alpha) {
								if(one.path().string().find(beta) == std::string::npos) {
										list.push_back(one);
										GroupList bb;
										boost::iter_split(bb, one.path().filename().string(), boost::algorithm::first_finder(std::string("_ea")));
										bb.pop_back();		// To remove the last part. after _ea ....
										b3.insert(b3.end(),bb.begin(),bb.end());
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
				for(PairGroupDE it : fils) std::cout << "FILs base: " << it.first << " Length: " << it.second.size() <<   std::endl;
				for(PairGroupDE it : kfils) std::cout << "KILs base: " << it.first << " Length: " << it.second.size() <<   std::endl;
				for(PairGroupDE it : cands) std::cout << "CANs base: " << it.first << " Length: " << it.second.size() <<   std::endl;
		}
};
