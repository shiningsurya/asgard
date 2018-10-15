# This is just the beginning of a long night. 
#
#
CPP := g++
BOOST_LD := -lboost_system -lboost_iostreams -lboost_filesystem
PGPLOT_LD := -lpgplot -lcpgplot
LD_FLAGS := -lm -g
CPPFLAGS := -std=c++11
all: AnalyzeFB.cpp
	$(CPP) AnalyzeFB.cpp $(CPPFLAGS) -o test $(BOOST_LD) $(LD_FLAGS) 
plot: Plotter.hpp Test*.cpp
	$(CPP) TestPlotter.cpp $(CPPFLAGS) -o plot $(BOOST_LD) $(PGPLOT_LD) $(LD_FLAGS)
testfb: TestFilterbank.cpp Filterbank.hpp 
	$(CPP) TestFilterbank.cpp $(CPPFLAGS) -o testfb $(BOOST_LD) $(LD_FLAGS)
testcd: TestCandidate.cpp Candidate.hpp
	$(CPP) TestCandidate.cpp $(CPPFLAGS) -o testcd $(BOOST_LD) $(LD_FLAGS)

