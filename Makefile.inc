# vim: ft=make
# This is makefile include
CPP := g++ -std=c++11
DEBUG := -g
OPTIMIZE:= -O3
LD_FLAGS := -lm 
CPPFLAGS := $(DEBUG) 
#### boost
BOOST := 
BOOST_INC :=
BOOST_LIB :=
#### dedisp
DEDISP := /home/shining/study/MS/vLITE/dedisp
DEDISP_INC := -I$(DEDISP)/inc
DEDISP_LIB   := -L$(DEDISP)/lib
#### pgplot
PGPLOT :=
PGPLOT_INC :=
PGPLOT_LIB :=
