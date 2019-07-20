# This is just the beginning of a long night. 
include Makefile.inc
###########################################
testco: src/*.hpp src/*.cpp test/Testcoadd.cpp
	$(CPP) test/Testcoadd.cpp $(CPPFLAGS) $(PGPLOTFLAGS) -o test/$@ $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(DEDISP_LIB) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testdp: src/*.hpp src/*.cpp
	$(CPP) test/TestDplot.cpp $(CPPFLAGS) $(PGPLOTFLAGS) -fopenmp -o test/$@ $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(DEDISP_LIB) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testmc : test/Testmc.cpp src/Multicast.hpp
	$(CPP) test/Testmc.cpp $(CPPFLAGS) -o test/$@ $(BOOSTFLAGS) $(BOOST_LD)
testrand : test/TestRandom.cpp
	$(CPP) test/TestRandom.cpp $(CPPFLAGS) -o test/$@ $(BOOSTFLAGS) $(BOOST_LD)
testfrfi : test/TestFilterRFI.cpp
	$(CPP) test/TestFilterRFI.cpp $(CPPFLAGS) -o test/$@ $(BOOSTFLAGS) $(BOOST_LD)
testanalyze: 
	$(CPP) test/TestAnalyzer.cpp $(CPPFLAGS) -o test/test $(BOOST_LD) $(LD_FLAGS) 
testplot: src/*.hpp src/*.cpp
	$(CPP) src/TestPlotter.cpp $(CPPFLAGS) -o test/plot $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testfb: src/TestFilterbank.cpp src/Filterbank.hpp 
	$(CPP) src/TestFilterbank.cpp $(CPPFLAGS) -o test/testfb $(BOOST_LD) $(LD_FLAGS)
testcd: src/TestCandidate.cpp src/Candidate.hpp
	$(CPP) src/TestCandidate.cpp $(CPPFLAGS) -o test/testcd $(BOOST_LD) $(LD_FLAGS)
testwf: src/*.hpp src/*.cpp
	$(CPP) src/TestWaterfall.cpp $(CPPFLAGS) -o test/testwf $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(DEDISP_LIB) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testcp: src/*.hpp src/*.cpp
	$(CPP) test/TestCandPlot.cpp $(CPPFLAGS) -o test/$@ $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(DEDISP_LIB) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testcs: src/*.hpp src/*.cpp
	$(CPP) src/TestCandSummary.cpp $(CPPFLAGS) -o test/$@ $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(DEDISP_LIB) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testwl: ./test/TestWritelogic.cpp
	$(CPP) ./test/TestWritelogic.cpp $(CPPFLAGS) -o test/$@ 
testjson: ./test/TestCJSON.cpp
	$(CPP) ./test/TestCJSON.cpp $(CPPFLAGS) -I$(JSONINC) $(DEDISPFLAGS) $(BOOSTFLAGS) -o bin/$@ $(BOOST_LD) $(DEDISP_LD) $(PGPLOT_LD) $(LD_FLAGS)
testfbsink: ./test/TestFBSink.cpp
	$(CPP) ./test/TestFBSink.cpp $(CPPFLAGS) -o test/$@ $(BOOST_LD)
testdada: ./test/TestDADA.cpp ./src/PsrDADA.hpp
	$(CPP) ./test/TestDADA.cpp $(CPPFLAGS) -I$(DADAHOME)/include -L$(DADAHOME)/lib $(BOOST_LD) $(DADA_LD) -o test/$@ 
testcompi:
	$(MPICPP) ./test/TestCoaddMPI.cpp $(CPPFLAGS) -Wno-unused-result $(MPIFLAGS) $(BOOST_LD) -o test/$@
testmplot:
	$(MPICPP) ./test/TestMPlot.cpp $(CPPFLAGS) $(PGPLOTFLAGS) -Wno-unused-result $(MPIFLAGS) $(BOOST_LD) $(PGPLOT_LD) $(LD_FLAGS) -o test/$@
testmlaunch:
	$(MPICPP) ./test/TestMPILaunch.cpp $(CPPFLAGS) -Wno-unused-result $(MPIFLAGS) $(BOOST_LD) -o test/$@
testsync:
	$(MPICPP) ./test/TestSync.cpp $(CPPFLAGS) -Wno-unused-result $(MPIFLAGS) $(BOOST_LD) -o test/$@
testcc: src/*.hpp src/*.cpp
	$(CPP) test/TestColorC.C $(CPPFLAGS) -o test/$@ $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(DEDISP_LIB) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testsh: src/*.hpp src/*.cpp
	$(CPP) test/TestShell.cpp $(CPPFLAGS) -o test/$@ $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(DEDISP_LIB) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
###########################################
agstat: src/*.hpp
	$(CPP) src/agstat.cpp $(CPPFLAGS) $(BOOSTFLAGS) -o bin/$@ $(BOOST_LD) $(LD_FLAGS)
agcandplot: src/*.hpp
	$(CPP) src/agcandplot.cpp $(CPPFLAGS) $(PGPLOTFLAGS) $(DEDISPFLAGS) $(BOOSTFLAGS) -o bin/$@ $(BOOST_LD) $(DEDISP_LD) $(PGPLOT_LD) $(LD_FLAGS)
agplot: src/*.hpp
	$(CPP) src/agplot.cpp $(CPPFLAGS) $(PGPLOTFLAGS) $(BOOSTFLAGS) -o bin/$@ $(BOOST_LD) $(PGPLOT_LD) $(LD_FLAGS)
agcoadd: src/*.hpp
	$(CPP) src/agcoadd.cpp $(CPPFLAGS) $(BOOSTFLAGS) -o bin/$@ $(BOOST_LD) $(LD_FLAGS)
agheader : src/*.hpp
	$(CPP) src/agheader.cpp $(CPPFLAGS) $(BOOSTFLAGS) -o bin/$@ $(BOOST_LD) $(LD_FLAGS)
agmcoadd: src/*.hpp
	$(MPICPP) src/agmcoadd.cpp -D_DEBUG $(CPPFLAGS) $(BOOSTFLAGS) $(MPIFLAGS) -o bin/$@  $(BOOST_LD) 
agmplot: src/*.hpp
	$(MPICPP) src/agmplot.cpp $(CPPFLAGS) $(PGPLOTFLAGS) $(BOOSTFLAGS) $(MPIFLAGS) -o bin/$@  $(BOOST_LD) $(PGPLOT_LD)
agdadacoadd: src/*.hpp
	$(MPICPP) src/agdadacoadd.cpp  $(CPPFLAGS) -I$(DADAHOME)/include -L$(DADAHOME)/lib  $(BOOSTFLAGS) $(MPIFLAGS) -o bin/$@  $(BOOST_LD) $(DADA_LD)
