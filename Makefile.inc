# vim: ft=make
# This is makefile include
CPP := g++
DEDISP := /home/shining/study/MS/vLITE/dedisp
PGPLOT := 
BOOST := 

BOOST_LD := -lboost_system -lboost_iostreams -lboost_filesystem -lboost_program_options
PGPLOT_LD := -lpgplot -lcpgplot
DEDISP_LD := -ldedisp
DEBUG := -g
OPTIMIZE:= -O3
LD_FLAGS := -lm 
CPPFLAGS := -std=c++11 $(DEBUG) 
DEDISP_INC := -I$(DEDISP)/inc
DEDISP_LIB   := -L$(DEDISP)/lib

Filterbank.o 
Candidate.o
FilterbankCandidate.o
Operations.o
Plotter.o
Analyzer.o

