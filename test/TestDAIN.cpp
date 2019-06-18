#include "asgard.hpp"
#include "PsrDADA.hpp"

using std::cout;
using std::endl;

int main() {
  PtrFloat f = nullptr;
  PtrByte  b = nullptr;
  key_t kk = 0x52;
  PsrDADA my(kk, 10240, 4096, 2);  
  bool going = false;
  //
  my.ReadLock(true);
  while(going || my.ReadHeader()) {
		if(!going) my.PrintHeader();
		auto xx = my.ReadData(f,b);
		if(xx == -1) {
				going = false;
				cout << "EOD" << endl;
		}
		else if(xx != my.GetByteChunkSize()) {
				// incomplete case
				cout << "Read " << xx << " bytes when requested " << my.GetByteChunkSize() << " bytes."<< endl;
				going = true;
		}
		else {
		// perfect case
				  going = true;
		}
  }
  my.ReadLock(false);
  //
  delete[] f;
  delete[] b;
  return 0;
}
