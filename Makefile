# This is just the beginning of a long night. 
#
#
CPP := g++
BOOST_LD := -lboost_system -lboost_iostreams -lboost_filesystem
PGPLOT_LD := -lpgplot -lcpgplot
DEBUG := -g
OPTIMIZE:= -O3
LD_FLAGS := -lm 
CPPFLAGS := -std=c++11 $(DEBUG) 
DEDISP := /home/shining/study/MS/vLITE/dedisp
DEDISP_INC := -I$(DEDISP)/inc
DEDISP_LIB   := -L$(DEDISP)/lib
DEDISP_LD := -ldedisp
testanalyze: TestAnalyzer.cpp Analyzer.hpp 
	$(CPP) TestAnalyzer.cpp $(CPPFLAGS) -o test $(BOOST_LD) $(LD_FLAGS) 
testplot: *.hpp *.cpp
	$(CPP) TestPlotter.cpp $(CPPFLAGS) -o plot $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testfb: TestFilterbank.cpp Filterbank.hpp 
	$(CPP) TestFilterbank.cpp $(CPPFLAGS) -o testfb $(BOOST_LD) $(LD_FLAGS)
testcd: TestCandidate.cpp Candidate.hpp
	$(CPP) TestCandidate.cpp $(CPPFLAGS) -o testcd $(BOOST_LD) $(LD_FLAGS)
operations: Operations.hpp
	$(CPP) -c Operations.hpp $(DEDISP_INC) $(CPPFLAGS) $(DEDISP_INC) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testwf: *.hpp *.cpp
	$(CPP) TestWaterfall.cpp $(CPPFLAGS) -o testwf $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(DEDISP_LIB) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testcp: *.hpp *.cpp
	$(CPP) TestCandPlot.cpp $(CPPFLAGS) -o $@ $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(DEDISP_LIB) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testcs: *.hpp *.cpp
	$(CPP) TestCandSummary.cpp $(CPPFLAGS) -o $@ $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(DEDISP_LIB) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
