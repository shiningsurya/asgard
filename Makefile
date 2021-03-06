# This is just the beginning of a long night. 
include Makefile.inc
###########################################
testco: src/*.hpp src/*.cpp test/Testcoadd.cpp
	$(CPP) test/Testcoadd.cpp $(CPPFLAGS) $(PGPLOTFLAGS) -o test/$@ $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(DEDISP_LIB) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testdp: src/*.hpp src/*.cpp
	$(CPP) test/TestDplot.cpp $(CPPFLAGS) $(PGPLOTFLAGS) -o test/$@ $(BOOST_LD) $(PGPLOT_LD) $(DEDISP_INC) $(DEDISP_LIB) $(BOOST_LD) $(LD_FLAGS) $(DEDISP_LD) 
testmc : test/Testmc.cpp src/Multicast.hpp
	$(CPP) test/Testmc.cpp $(CPPFLAGS) -o test/$@ $(BOOSTFLAGS) $(BOOST_LD)
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
testwl: ./test/TestWritelogic.cpp
	$(CPP) ./test/TestWritelogic.cpp $(CPPFLAGS) -o test/$@ 
testcjson: ./test/TestCJSON.cpp
	$(CPP) ./test/TestCJSON.cpp $(CPPFLAGS) $(JSONINC) $(DEDISPFLAGS) $(BOOSTFLAGS) -o test/$@  $(DEDISP_LD) $(BOOST_LD)
testdain: ./test/TestDAIN.cpp
	$(CPP) ./test/TestDAIN.cpp $(CPPFLAGS) $(DADAFLAGS) $(BOOST_LD)  -o test/$@ 
	cp test/$@ /home/vlite-master/surya/bin/
testheim: ./test/TestHeimdall.cpp
	$(MPICPP) test/TestHeimdall.cpp $(CPPFLAGS) $(BOOSTFLAGS) $(MPIFLAGS) $(BOOST_LD) -o bin/$@
	cp test/$@ /home/vlite-master/surya/bin/
testdaout: ./test/TestDAOUT.cpp
	$(CPP) ./test/TestDAOUT.cpp $(CPPFLAGS) $(DADAFLAGS) $(BOOST_LD)  -o test/$@ 
	cp test/$@ /home/vlite-master/surya/bin/
testdasb: ./test/TestDADAScrub.cpp
	$(CPP) ./test/TestDADAScrub.cpp $(CPPFLAGS) $(DADAFLAGS) $(BOOST_LD)  -o test/$@ 
	cp test/$@ /home/vlite-master/surya/bin/
testdaco: ./test/TestDADACoadd.cpp
	$(MPICPP) ./test/TestDADACoadd.cpp $(CPPFLAGS) $(BOOSTFLAGS) $(MPIFLAGS) $(DADAFLAGS) $(BOOST_LD) -o test/$@ 
testmpi: ./test/TestMPI.cpp
	$(MPICPP) ./test/TestMPI.cpp $(CPPFLAGS) $(BOOSTFLAGS) $(MPIFLAGS) $(DADAFLAGS) $(BOOST_LD) -o test/$@ 
testmcoll: ./test/TestMPIColl.cpp
	$(MPICPP) ./test/TestMPIColl.cpp $(CPPFLAGS) $(BOOSTFLAGS) $(MPIFLAGS) $(BOOST_LD) -o test/$@ 
testmplot: 
	$(MPICPP) test/TestMPlot.cpp $(CPPFLAGS) $(BOOSTFLAGS) $(MPIFLAGS) $(PGPLOTFLAGS) $(BOOST_LD) -o bin/$@ $(PGPLOT_LD)
testddm : src/*.hpp
	$(CPP) test/TestDedisperser.cpp $(CPPFLAGS) $(DEDISPFLAGS) $(BOOSTFLAGS) -o test/$@ $(BOOST_LD) $(DEDISP_LD)  $(LD_FLAGS)
testfbc : src/*.hpp
	$(CPP) test/TestFBC.cpp $(CPPFLAGS) $(DEDISPFLAGS) $(BOOSTFLAGS) -o test/$@ $(BOOST_LD) $(DEDISP_LD)  $(LD_FLAGS)
testfbcp: src/*.hpp src/*.cpp
	$(CPP) test/TestFBCplot.cpp $(CPPFLAGS) -o test/$@ $(PGPLOTFLAGS) $(DEDISPFLAGS) $(BOOSTFLAGS) -o test/$@ $(PGPLOT_LD) $(BOOST_LD) $(DEDISP_LD)  $(LD_FLAGS)
###########################################
agdadacoadd: ./src/agdadacoadd.cpp
	$(MPICPP) src/agdadacoadd.cpp -DRT_PROFILE  $(CPPFLAGS) $(BOOSTFLAGS) $(MPIFLAGS) $(DADAFLAGS) $(BOOST_LD) -o bin/$@ 
	cp bin/$@ /home/vlite-master/surya/bin/
agmplot: ./src/agmplot.cpp
	$(MPICPP) src/agmplot.cpp $(CPPFLAGS) $(BOOSTFLAGS) $(MPIFLAGS) $(PGPLOTFLAGS) $(BOOST_LD) -o bin/$@ $(PGPLOT_LD)
agmcoadd: ./src/agmcoadd.cpp
	$(MPICPP) src/agmcoadd.cpp  $(CPPFLAGS) $(BOOSTFLAGS) $(MPIFLAGS) $(BOOST_LD) -o bin/$@ $(PGPLOT_LD)
agstat: src/*.hpp
	$(CPP) src/agstat.cpp $(CPPFLAGS) $(BOOSTFLAGS) -o bin/$@ $(BOOST_LD) $(LD_FLAGS)
agheader: src/*.hpp
	$(CPP) src/agheader.cpp $(CPPFLAGS) $(BOOSTFLAGS) -o bin/$@ $(BOOST_LD) $(LD_FLAGS)
agcandplot: src/*.hpp
	$(CPP) src/agcandplot.cpp $(CPPFLAGS) $(PGPLOTFLAGS) $(DEDISPFLAGS) $(BOOSTFLAGS) -o bin/$@ $(BOOST_LD) $(DEDISP_LD) $(PGPLOT_LD) $(LD_FLAGS)
agcjson: src/*.hpp
	$(CPP) src/agcjson.cpp $(CPPFLAGS) $(JSONINC) $(DEDISPFLAGS) $(BOOSTFLAGS) -o bin/$@ $(BOOST_LD) $(DEDISP_LD) $(LD_FLAGS)
agplot: src/*.hpp
	$(CPP) src/agplot.cpp $(CPPFLAGS) $(PGPLOTFLAGS) $(BOOSTFLAGS) -o bin/$@ $(BOOST_LD) $(PGPLOT_LD) $(LD_FLAGS)
agcoadd: src/*.hpp
	$(CPP) src/agcoadd.cpp $(CPPFLAGS) $(BOOSTFLAGS) -o bin/$@ $(BOOST_LD) $(LD_FLAGS)
