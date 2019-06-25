#include <asgard.hpp>
#include <DADACoadd.hpp>

int main() {
		DADACoadd  dc(
				0x56, 0x60, false, // keys and filout
				10240, 4096, 2,          // big three
				0);                      // root
		// I don't know
		// what to do
		// if this doesn't work
		// lol jk
		dc.Work();
		return 0;
}
