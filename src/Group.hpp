#pragma once
#include "asgard.hpp"
// 20180521_162511_muos_ea01_kur.fil
// 20180521_162511_muos_ea01.fil
const std::string efil(".fil"), ecand(".cand"), edelim("_"), emuos("muos"), ekur("kur");
class Group {
		public:
				Group(fs::path pat_, std::string date_, std::string ant_, int kflag_, std::string ext_)
						: pat(pat_), date(date_), ant(ant_), kflag(kflag_), ext(ext_)
				{
						group = date + edelim + emuos + edelim;
				}
				std::string date, ant, ext, group;
				fs::path pat;
				int iant, kflag;
				const fs::path path() const {
						auto p = pat;
						std::string sp;
						sp     += date + edelim;
						sp     += emuos+ edelim;
						sp     += ant;
						if(kflag) sp += edelim + ekur;
						sp     += ext;
						return p.append(sp);
				}
};

Group GroupFactory(const fs::directory_entry& x) {
		std::string date, ant, ext;
		fs::path pat = x.path().parent_path();
		int kflag;
		ext = x.path().extension().string();
		StringVector bb;
		boost::split(bb, x.path().stem().string(), boost::is_any_of("_"));
		if(bb.size() == 4 || bb.size() == 5) {
				date = bb[0] + edelim + bb[1];
				ant  = bb[3];
				if(bb.size() == 5 && bb[4] == ekur) kflag = 1;
				else kflag = 0;
		}
		else {
				kflag = -1;
		}
		return Group(pat, date, ant, kflag, ext);
}

Group GroupFactory(const std::string& g) {
		std::string date, ant, ext;
		int kflag;
		StringVector bb;
		boost::split(bb, g,  boost::is_any_of("_"));
		if(bb.size() == 4 || bb.size() == 5) {
				date = bb[0] + edelim + bb[1];
				ant  = bb[3];
				if(bb.size() == 5 && bb[4] == ekur) kflag = 1;
				else kflag = 0;
		}
		else {
				kflag = -1;
		}
		return Group("", date, ant, kflag, ext);
}

// functor for set
struct GroupCompare {
		bool operator()(const Group& one, const Group& two) { 
				return one.path().string() < two.path().string();
		}
};
