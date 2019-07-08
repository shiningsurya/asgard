#include "asgard.hpp"
#include "PsrDADA.hpp"

using std::cout;
using std::endl;

int main() {
		key_t kk = 0x60;
			PsrDADA dadaout(kk, 10240, 4096, 2, "/tmp/dadaout.log");  
			dadaout.WriteLock(true);
		while(true) {
			PsrDADA dadain(kk, 10240, 4096, 2, "/tmp/dadain.log");  
				
		}
		return 0;
}
