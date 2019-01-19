#include "Filterbank.hpp"

int main() {
		Filterbank fb;
		FilterbankReader fbr;
		fbr.Read(fb, std::string("/home/shining/study/MS/vLITE/mkerr/fil/20180521_162250_muos_ea02_kur.fil"));
		float *fd;
		fd = new float[100*4096];
		//fb.Unpack(fd, 100,100);
		/*
		 *for(int i = 0; i < 100; i++) {
		 *        for(int j = 0; j < 4096; j++) {
		 *                std::cout << " " << fd[i*4096 +  j];
		 *        }
		 *        //std::cout << std::endl;
		 *}
		 */
		std::cout << fb << std::endl;
		delete[] fd;
		return 0;
}
