#include "asgard.hpp"
#include "PsrDADA.hpp"

constexpr unsigned long int SIZE = 16;

int main() {
		key_t kk = 0x1a4;
		int nchan = 4;
		PsrDADA mybuf(kk);	
		float arr[SIZE], out[SIZE];
		for(int i = 0; i < SIZE; i++) arr[i] = i % 4;
		mybuf.Connect();
		// some stuff here
		std::cout << "WriteHeader" << std::endl;
		mybuf.WriteHeader();
		std::cout << "WriteData" << std::endl;
		mybuf.WriteData(SIZE/nchan, arr);
		std::cout << "ReadHeader" << std::endl;
		mybuf.ReadHeader();
		std::cout << "ReadData" << std::endl;
		mybuf.ReadData(SIZE/nchan, &out[0]);
		std::cout << "Disconnect" << std::endl;
		mybuf.Disconnect();
		// print
		for(int i = 0; i < SIZE; i++) std::cout << 
				arr[i] << "  " << out[i] << std::endl;
		return 0;
}
