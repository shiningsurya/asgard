#include "Filterbank.hpp"

int main() {
		Filterbank fb;
		FilterbankReader fbr;
		fbr.Read(fb, std::string("/home/shining/study/MS/vLITE/mkerr/fil/20180521_162250_muos_ea02_kur.fil"));
		std::cout << fb << std::endl;
		return 0;
}
