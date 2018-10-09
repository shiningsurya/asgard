# This is just the beginning of a long night. 
#
#
all: AnalyzeFB.cpp
	g++ AnalyzeFB.cpp -std=c++11 -o test -lboost_system  -lboost_filesystem 
