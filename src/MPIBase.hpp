#pragma once
#include <asgard.hpp>
// Crawling
#include "Analyzer.hpp"
// iota
#include <numeric>
// MPI
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/operations.hpp>
namespace mpi = boost::mpi;

class MPIBase {
 protected:
 	// Crawling
 	AnalyzeFB afb;
 	// MPI
	bool verbose;
	int root;
	mpi::environment env;
	mpi::communicator world;
	std::string rootpath;
	// my flatter
	template<typename T>
	 std::vector<T> flatter(const std::vector<std::vector<T>>& in) {
		std::vector<T> ret;
		for(auto& ii : in) {
		 std::copy(ii.cbegin(), ii.cend(), back_inserter(ret));
		}
		return ret;
	 }
	// filename, hostname resolution
	StringVector datastringlist, hostnamelist;
	void gatherDataHostname(const StringVector& data) {
	 std::vector<StringVector> All_datapathlist;
	 mpi::gather(world, data , All_datapathlist, root);
	 mpi::gather(world, env.processor_name(), hostnamelist, root);
	 if(verbose && world.rank() == root) {
	 	// first comes hostname
		for(int ij = 0; ij < hostnamelist.size(); ij++) {
		 // second comes filename
		 for(auto& _fn : All_datapathlist[ij]) {
			std::cout << " I " << _fn << " @ " << hostnamelist[ij];
			if(hostnamelist[ij] == env.processor_name())
			 std::cout << "*"; 
			std::cout << std::endl;
		 }
		}
	 }
	}
	template<typename T>
	 std::vector<T> gatherPayload(const std::vector<T>& pl) {
		// returns valid only in root
		std::vector<std::vector<T>> o_pl;
		mpi::gather(world, pl, o_pl, root);
		return flatter(o_pl);
	 }
	// striding inorder
	std::vector<unsigned int> stride;
	template<typename T>
	 void strider(const std::vector<T>& tag) {
		stride.resize(tag.size());
		std::iota(stride.begin(), stride.end(), static_cast<unsigned int>(0));
		std::sort(stride.begin(), stride.end(), 
			[&tag](unsigned int a, unsigned int b){ return tag[a] < tag[b];}
			);
	 }
public:
	MPIBase(int _root, bool _verbose, fs::path _rootpath) : root(_root), verbose(_verbose) {
	 Crawl(_rootpath);
	}
	inline bool isRoot() {return world.rank() == root;}
	void Crawl(fs::path rootpath) {
	 auto filpath = rootpath / "fildata";
	 auto candpath = rootpath / "cands";
	 afb.Crawl(filpath.string(), candpath.string());
	}
	inline PathList CrawlFils(const std::string& group, bool isKur) {
	 return isKur ? afb.kfils[group] : afb.fils[group];
	}
	inline PathList CrawlCands(const std::string& group, bool isKur) {
	 return isKur ? afb.kcands[group] : afb.cands[group];
	}
	
};
