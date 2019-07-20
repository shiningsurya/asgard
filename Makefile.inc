# vim: ft=make
# This is makefile include
CPP := g++ -std=c++11 -Wno-unused-result
SRC := /home/shining/study/MS/asgard
DEBUG := -g
OPTIMIZE:= -O3 -fopenmp
LD_FLAGS := -lm 
CPPFLAGS := $(DEBUG) $(OPTIMIZE) -I$(SRC)/src -I$(SRC)/include
#### boost
BOOST := 
BOOST_INC :=
BOOST_LIB :=
BOOST_LD := -lboost_system -lboost_iostreams -lboost_filesystem -lboost_program_options
#### dedisp
DEDISP := /home/shining/study/MS/vLITE/dedisp
DEDISP_INC := -I$(DEDISP)/inc
DEDISP_LIB   := -L$(DEDISP)/lib
DEDISP_LD := -ldedisp
DEDISPFLAGS = $(DEDISP_INC) $(DEDISP_LIB)
#### pgplot
PGPLOT :=
PGPLOT_INC :=
PGPLOT_LIB :=
PGPLOT_LD := -lcpgplot -lpgplot -lpng
#### MPI
MPICPP := mpic++ -std=c++11 -Wno-unused-result -g
MPIFLAGS := -fopenmp -DOMPI_SKIP_MPICXX -DMPI -lboost_mpi -lboost_serialization
#### PSRDADA
DADAHOME := /home/shining/study/MS/vLITE
DADA_LD := -lpsrdada
#### JSON
JSONINC := /home/shining/study/MS
##### No user input beyond this line
##### No user input beyond this line
##### No user input beyond this line
CPPFLAGS +=  -Wno-deprecated-declarations
CPPFLAGS += -I. -Itest/ -Isrc/
CPPFLAGS += -Wl,-rpath=$(GCC)/lib64
BOOSTFLAGS := -I$(BOOST_INC) -L$(BOOST_LIB) -Wl,-rpath=$(BOOST_LIB) 
DEDISPFLAGS := -I$(DEDISP_INC) -L$(DEDISP_LIB) -Wl,-rpath=$(DEDISP_LIB)
PGPLOTFLAGS := -I$(PGPLOT_INC) -L$(PGPLOT_LIB) -Wl,-rpath=$(PGPLOT_LIB)
PGPLOTFLAGS += -L$(PGFORTRAN_LIB) -Wl,-rpath=$(PGFORTRAN_LIB)
PGPLOTFLAGS += -L$(PNG_LIB) -Wl,-rpath=$(PNG_LIB)
DADAFLAGS := -I$(DADAHOME)/include -L$(DADAHOME)/lib -lpsrdada
