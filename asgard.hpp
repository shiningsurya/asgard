// I define my types and classes here
// includes
#include<iostream>
#include <algorithm>
#include <string>
#include <map>
#include <utility>
#include <boost/filesystem.hpp>
#include "math.h"
// Definitions
#define TSAMP  0.000097

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

typedef std::vector<fs::directory_entry> DEList;
typedef std::map<std::string, DEList> MapGroupDE; 
typedef std::pair<std::string, DEList> PairGroupDE;



typedef std::vector<Filterbank> FilterbankList;
typedef std::vector<Candidate>  CandidateList;
typedef std::vector<std::string> GroupList;
typedef std::pair<std::string, CandidateList> CandidateGroup;
typedef std::pair<std::string, FilterbankList> FilterbankGroup;
