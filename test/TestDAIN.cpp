#include "asgard.hpp"
#include "PsrDADA.hpp"

using std::cout;
using std::endl;

int main() {
 key_t kk = 0x56;
 uint64_t obs = 0;
 //
 while(true) // for every observation 
 {
	PsrDADA my(kk, 10240, 4096, 2);  
	my.ReadLock(true);
	bool going = false;
	PtrFloat f = nullptr;
	PtrByte  b = nullptr;
	while(going || my.ReadHeader()) {
	 if(!going) my.PrintHeader();
	 auto xx = my.ReadData(f,b);
	 if(xx == -1) {
		cout << "EOD" << endl;
		break;
	 }
	 else if(xx != my.GetByteChunkSize()) {
		// incomplete case
		cout << "Read " << xx << " bytes when requested " << my.GetByteChunkSize() << " bytes."<< endl;
		cout << "incomplete!" << endl;
		break;
	 }
	 else {
		// perfect case
		going = true;
	 }
	} // for span of each observation
	my.ReadLock(false);
	delete[] f;
	delete[] b;
	cout << "Observation #: " << ++obs << endl;
 } // for every observation
 return 0;
}
