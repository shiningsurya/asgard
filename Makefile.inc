# vim: ft=make
# This is makefile include
GCC := /home/vlite-master/surya/gcc-build
CPP := $(GCC)/bin/g++ -std=c++11
DEBUG := -g
OPTIMIZE:= -O3
LD_FLAGS := -lm 
CPPFLAGS := $(DEBUG) $(OPTIMIZE) 
#### boost
BOOST := /home/vlite-master/surya
BOOST_INC := $(BOOST)/include
BOOST_LIB := $(BOOST)/lib
BOOST_LD := -lboost_system -lboost_iostreams -lboost_filesystem -lboost_program_options
#### dedisp
DEDISP := /home/vlite-master/mtk/dedisp
DEDISP_INC := $(DEDISP)/src
DEDISP_LIB   := $(DEDISP)/lib
DEDISP_LD := -ldedisp
#### pgplot
PGPLOT := /home/vlite-master/surya/pgplot
PGPLOT_INC := $(PGPLOT)
PGPLOT_LIB := $(PGPLOT)
PGFORTRAN_LIB := $(GCC)/lib64
PGPLOT_LD := -lpgplot -lcpgplot -lgfortran -lX11 -lpng
#### png
PNG := /home/vlite-master/surya
PNG_INC := $(PNG)/include
PNG_LIB := $(PNG)/lib
#### MPI
MPICPP := /home/vlite-master/surya/bin/mpic++ -std=c++11
MPIFLAGS := -DMPI -DOMPI_SKIP_MPICXX -lboost_mpi -lboost_serialization
#### DADA
DADAHOME := /home/vlite-master/mtk
#### JSON
JSONINC := -I/home/vlite-master/surya/include
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
