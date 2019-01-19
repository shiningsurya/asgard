#include "asgard.hpp"
#include "Candidate.hpp"

int main() {
		Candidate cd(std::string("6.07841	100366	78.4109	5	930	270.579	1	100366	100368"),TSAMP);	
		std::cout << cd << std::endl;
		return 0;
}
