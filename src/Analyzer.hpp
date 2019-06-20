#pragma once
#include "asgard.hpp"
#include "Group.hpp"
#include <boost/algorithm/string.hpp>
#include <set>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/bind.hpp>

class AnalyzeFB {
 public:
	Map_Pathgroups fils, kfils, cands, kcands;
	GroupVector Gvector;
	StringVector base;
	void Crawl(const std::string& fdir, const std::string& cdir) {
	 fildir = fdir;
	 candir = cdir;
	 // iterating in directories
	 std::for_each(fs::directory_iterator(fildir), fs::directory_iterator(), boost::bind(AnalyzeFB::Builder, _1, boost::ref(Gvector)));
	 std::for_each(fs::directory_iterator(candir), fs::directory_iterator(), boost::bind(AnalyzeFB::Builder, _1, boost::ref(Gvector)));
	 // taking out file duplicates
	 std::set<Group, GroupCompare, std::allocator<Group> > bset( Gvector.begin(), Gvector.end() );
	 Gvector.assign( bset.begin(), bset.end() );
	 // building maps
	 for(const auto& b : Gvector) {
		base.push_back(b.group);
		if(b.kflag) {
		 if(b.ext == efil) {
			kfils[b.group].push_back(b.path());
		 }
		 else if(b.ext == ecand) {
			kcands[b.group].push_back(b.path());
		 }
		}
		else {
		 if(b.ext == efil) {
			fils[b.group].push_back(b.path());
		 }
		 else if(b.ext == ecand) {
			cands[b.group].push_back(b.path());
		 }
		}
	 }
	 // taking out group duplicates
	 std::set<std::string> sset(base.begin(), base.end());
	 base.assign( sset.begin(), sset.end() );
	}
	void Crawl(const std::string& fd) {
	 fildir = fd;
	 candir = fd;
	 Crawl(fd,fd);
	}
 private:
	fs::path fildir, candir;
	static void Builder(fs::directory_entry x, GroupVector& gv) {
	 Group gg = GroupFactory(x);
	 if(gg.kflag != -1) gv.push_back( gg );
	}
 public:
	void PrintPaths() {
	 std::cout << "Fildir: " << fildir.string() << std::endl;
	 std::cout << "Candir: " << candir.string() << std::endl;
	}
	void Groups() {
	 for(const auto& ix : base) std::cout << ix << std::endl;
	}
	void Summary() {
	 std::cout << "Summary\n";
	 for(const auto& it : fils) std::cout << "FILs base: " << it.first << " Length: " << it.second.size() <<   std::endl;
	 for(const auto& it : kfils) std::cout << "KILs base: " << it.first << " Length: " << it.second.size() <<   std::endl;
	 for(const auto& it : cands) std::cout << "CANs base: " << it.first << " Length: " << it.second.size() <<   std::endl;
	 for(const auto& it : kcands) std::cout << "CANs base: " << it.first << " Length: " << it.second.size() <<   std::endl;
	}
};

