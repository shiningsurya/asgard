# This is just the beginning of a long night. 
#
#
CPP := g++
BOOST_LD := -lboost_system -lboost_iostreams -lboost_filesystem
PGPLOT_LD := -lpgplot -lcpgplot
LD_FLAGS := -lm 
CPPFLAGS := -std=c++11
DEDISP := /home/shining/study/MS/vLITE/dedisp
DEDISP_INC := -I$(DEDISP)/inc
DEDISP_LIB   := -L$(DEDISP)/lib
DEDISP_LD := -ldedisp
all: AnalyzeFB.cpp
	$(CPP) AnalyzeFB.cpp $(CPPFLAGS) -o test $(BOOST_LD) $(LD_FLAGS) 
plot: *.hpp *.cpp
	$(CPP) TestPlotter.cpp $(CPPFLAGS) -o plot $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testfb: TestFilterbank.cpp Filterbank.hpp 
	$(CPP) TestFilterbank.cpp $(CPPFLAGS) -o testfb $(BOOST_LD) $(LD_FLAGS)
testcd: TestCandidate.cpp Candidate.hpp
	$(CPP) TestCandidate.cpp $(CPPFLAGS) -o testcd $(BOOST_LD) $(LD_FLAGS)
operations: Operations.hpp
	$(CPP) -c Operations.hpp $(DEDISP_INC) $(CPPFLAGS) $(DEDISP_INC) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 

