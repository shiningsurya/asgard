# This is just the beginning of a long night. 
include Makefile.inc
## boost
BOOSTFLAGS := $(BOOST_INC) $(BOOST_LIB)
BOOST_LD := -lboost_system -lboost_iostreams -lboost_filesystem -lboost_program_options
## dedisp
DEDISPFLAGS := $(DEDISP_INC) $(DEDISP_LIB)
DEDISP_LD := -ldedisp
## pgplot
PGPLOTFLAGS := $(PGPLOT_INC) $(PGPLOT_LIB)
PGPLOT_LD := -lpgplot -lcpgplot
###########################################
testanalyze: src/TestAnalyzer.cpp src/Analyzer.hpp 
	$(CPP) src/TestAnalyzer.cpp $(CPPFLAGS) -o test/test $(BOOST_LD) $(LD_FLAGS) 
testplot: src/*.hpp src/*.cpp
	$(CPP) src/TestPlotter.cpp $(CPPFLAGS) -o test/plot $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testfb: src/TestFilterbank.cpp src/Filterbank.hpp 
	$(CPP) src/TestFilterbank.cpp $(CPPFLAGS) -o test/testfb $(BOOST_LD) $(LD_FLAGS)
testcd: src/TestCandidate.cpp src/Candidate.hpp
	$(CPP) src/TestCandidate.cpp $(CPPFLAGS) -o test/testcd $(BOOST_LD) $(LD_FLAGS)
testwf: src/*.hpp src/*.cpp
	$(CPP) src/TestWaterfall.cpp $(CPPFLAGS) -o test/testwf $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(DEDISP_LIB) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testcp: src/*.hpp src/*.cpp
	$(CPP) src/TestCandPlot.cpp $(CPPFLAGS) -o test/$@ $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(DEDISP_LIB) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testcs: src/*.hpp src/*.cpp
	$(CPP) src/TestCandSummary.cpp $(CPPFLAGS) -o test/$@ $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(DEDISP_LIB) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
###########################################
agplot: src/*.hpp
	$(CPP) src/agplot.cpp $(CPPFLAGS) $(PGPLOTFLAGS) $(DEDISPFLAGS) $(BOOSTFLAGS) -o bin/$@ $(BOOST_LD) $(DEDISP_LD) $(PGPLOT_LD) $(LD_FLAGS)
