#include <iostream>

using namespace std;

int main() {
		int iret[] = {1,1,1,1,1};
		float fret[] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
		int size = 5;
		unsigned char A, B, C, D, ucval, cval;
		int ival;
		float fval;
		for(int i = 0; i < size; i++) {
				ival = 2;
				// int
				A = (unsigned char) (ival & 0x03) << 6;
				B = (unsigned char) (ival & 0x03) << 4;
				C = (unsigned char) (ival & 0x03) << 2;
				D = (unsigned char) (ival & 0x03) << 0;
				ucval = A|B|C|D;	
				//
		}
		return 0;
}
