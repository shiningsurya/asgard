// I define my types and classes here
// includes
#ifndef ASGARD_H
#define ASGARD_H
#include<iostream>
#include <algorithm>
#include <string>
#include <map>
#include <array>
#include <utility>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>
#include "math.h"
// Definitions
#define TSAMP  0.000097
#define TROOT "/home/shining/study/MS/vLITE/mkerr"
#define TGROUP "20180521_182857_muos"
// namespace
namespace fs = boost::filesystem;


// storage type classes
class Candidate;
class Filterbank;

// analyze type
class AnalyzeFB;

// plot type
class Waterfall;
class CandPlot;


// type def
typedef long unsigned int timeslice;
typedef std::vector<std::string> StringVector;

typedef std::vector<fs::directory_entry> DEList;
typedef std::map<std::string, DEList> MapGroupDE; 
typedef std::pair<std::string, DEList> PairGroupDE;

typedef std::vector<float> FloatVector;


typedef std::vector<Filterbank> FilterbankList;
typedef std::vector<Candidate>  CandidateList;
//typedef std::vector<std::string> GroupList;
typedef std::vector<CandidateList> CandidateAntenna;
typedef std::pair<std::string, CandidateAntenna> CandidateGroup;
typedef std::pair<std::string, FilterbankList> FilterbankGroup;
// functions helpful everywhere
std::string GetGroup(std::string fl){
		std::vector<std::string> b,r;
		boost::split(b, fl, boost::is_any_of("/"), boost::token_compress_on);
		boost::split(r, b[b.size() -1], boost::is_any_of("_"), boost::token_compress_on);
		std::string ret = r[0];
		std::string underscore("_");
		ret += underscore;
		ret += r[1];
		ret += underscore;
		ret += r[2];
		ret += underscore;
		return ret;
}
bool QueryKurtosis(std::string fl) {
		std::string kur = std::string("_kur.fil");
		return fl.compare(fl.length() - kur.length(), kur.length(), kur) == 0;
}	
std::string GetAntenna(std::string fl) {
		std::vector<std::string> b;
		boost::split(b, fl, boost::is_any_of("_"), boost::token_compress_on);
		if(b[3].compare(0, 2, std::string("ea")) == 0) return b[3];
}
#endif
