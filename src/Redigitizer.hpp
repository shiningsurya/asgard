#pragma once
#ifndef _REDIGIT_H_
#define _REDIGIT_H_
#define HI2BITS 192
#define UPMED2BITS 48
#define LOMED2BITS 12
#define LO2BITS 3
#define LO4BITS 15 
#define UP4BITS 240

#if 0
static inline int quant2bit_1(float a) {
		if(a <= 0.09) return 0;
		else if(a <= 0.38) return 1;
		else if(a <= 1.04) return 2;
		else return 3;
}
static inline int quant2bit_2(float a) {
		if(a <= 0.43) return 0;
		else if(a <= 0.98) return 1;
		else if(a <= 1.75) return 2;
		else return 3;
}
static inline int quant2bit_3(float a) {
		if(a <= 0.81) return 0; else if(a <= 1.44) return 1;
		else if(a <= 2.14) return 2;
		else return 3;
}
static inline int quant2bit_4(float a) {
		if(a <= 1.31) return 0;
		else if(a <= 1.77) return 1;
		else if(a <= 2.37) return 2;
		else return 3;
}
static inline int quant2bit_5(float a) {
		if(a <= 1.40) return 0;
		else if(a <= 2.00) return 1;
		else if(a <= 2.55) return 2;
		else return 3;
}
static inline int quant2bit_6(float a) {
		if(a <= 1.61) return 0;
		else if(a <= 2.16) return 1;
		else if(a <= 2.61) return 2;
		else return 3;
}
static inline int quant2bit_7(float a) {
		if(a <= 1.77) return 0;
		else if(a <= 2.29) return 1;
		else if(a <= 2.67) return 2;
		else return 3;
}
static inline int quant2bit_8(float a) {
		if(a <= 1.91) return 0;
		else if(a <= 2.38) return 1;
		else if(a <= 2.72) return 2;
		else return 3;
}
static inline int quant2bit_9(float a) {
		if(a <= 2.03) return 0;
		else if(a <= 2.46) return 1;
		else if(a <= 2.76) return 2;
		else return 3;
}
static inline int quant2bit_10(float a) {
		if(a <= 2.12) return 0;
		else if(a <= 2.51) return 1;
		else if(a <= 2.79) return 2;
		else return 3;
}
static inline int quant2bit_11(float a) {
		if(a <= 2.20) return 0;
		else if(a <= 2.56) return 1;
		else if(a <= 2.81) return 2;
		else return 3;
}
static inline int quant2bit_12(float a) {
		if(a <= 2.26) return 0;
		else if(a <= 2.60) return 1;
		else if(a <= 2.83) return 2;
		else return 3;
}
static inline int quant2bit_13(float a) {
		if(a <= 2.32) return 0;
		else if(a <= 2.64) return 1;
		else if(a <= 2.84) return 2;
		else return 3;
}
static inline int quant2bit_14(float a) {
		if(a <= 2.37) return 0;
		else if(a <= 2.66) return 1;
		else if(a <= 2.86) return 2;
		else return 3;
}
static inline int quant2bit_15(float a) {
		if(a <= 2.41) return 0;
		else if(a <= 2.69) return 1;
		else if(a <= 2.87) return 2;
		else return 3;
}
static inline int quant2bit_16(float a) {
		if(a <= 2.45) return 0;
		else if(a <= 2.71) return 1;
		else if(a <= 2.88) return 2;
		else return 3;
}

static inline int quant2bit(int nant, float a) {
	//	a *= sqrt(nant);
		if(nant == 1)       return quant2bit_1(a);
		else if(nant == 2)  return quant2bit_2(a);
		else if(nant == 3)  return quant2bit_3(a);
		else if(nant == 4)  return quant2bit_4(a);
		else if(nant == 5)  return quant2bit_5(a);
		else if(nant == 6)  return quant2bit_6(a);
		else if(nant == 7)  return quant2bit_7(a);
		else if(nant == 8)  return quant2bit_8(a);
		else if(nant == 9)  return quant2bit_9(a);
		else if(nant == 10) return quant2bit_10(a);
		else if(nant == 11) return quant2bit_11(a);
		else if(nant == 12) return quant2bit_12(a);
		else if(nant == 13) return quant2bit_13(a);
		else if(nant == 14) return quant2bit_14(a);
		else if(nant == 15) return quant2bit_15(a);
		else if(nant == 16) return quant2bit_16(a);
		else return static_cast<int>(a);
}
unsigned char dig2bit___(float a, float b, float c, float d, int numant) {
		unsigned char A, B, C, D;	
		float sqrt_nant = sqrt(numant);
		A = (unsigned char) (static_cast<unsigned int>(a*sqrt_nant) & 0x03) << 6;
		//std::cout << " a =  " << a << " A =  " << std::bitset<8>(A) << std::endl;
		B = (unsigned char) (static_cast<unsigned int>(b*sqrt_nant) & 0x03) << 4;
		//std::cout << " b =  " << b << " B =  " << std::bitset<8>(B) << std::endl;
		C = (unsigned char) (static_cast<unsigned int>(c*sqrt_nant) & 0x03) << 2;
		//std::cout << " c =  " << c << " C =  " << std::bitset<8>(C) << std::endl;
		D = (unsigned char) (static_cast<unsigned int>(d*sqrt_nant) & 0x03) << 0;
		//std::cout << " d =  " << d << " D =  " << std::bitset<8>(D) << std::endl;
		return A|B|C|D;
}
unsigned char dig2bit(float a, float b, float c, float d, int numant) {
		unsigned char A, B, C, D;	
		A = (unsigned char) (quant2bit(numant, a) & 0x03) << 6;
		//std::cout << " a =  " << a << " A =  " << std::bitset<8>(A) << std::endl;
		B = (unsigned char) (quant2bit(numant, b) & 0x03) << 4;
		//std::cout << " b =  " << b << " B =  " << std::bitset<8>(B) << std::endl;
		C = (unsigned char) (quant2bit(numant, c) & 0x03) << 2;
		//std::cout << " c =  " << c << " C =  " << std::bitset<8>(C) << std::endl;
		D = (unsigned char) (quant2bit(numant, d) & 0x03) << 0;
		//std::cout << " d =  " << d << " D =  " << std::bitset<8>(D) << std::endl;
		return A|B|C|D;
}
#endif // deadcode
// Numbers from Jenet&Anderson
static inline int quant4bit(float x) {
 if(x < -2.397) return 0;
 else if(x < -1.839) return 1;
 else if(x < -1.432) return 2;
 else if(x < -1.093) return 3;
 else if(x < -0.7928) return 4;
 else if(x < -0.5152) return 5;
 else if(x < -0.2514) return 6;
 else if(x <  0.000) return 7;
 else if(x < 0.2514) return 8;
 else if(x < 0.5152) return 9;
 else if(x < 0.7928) return 10;
 else if(x < 1.093) return 11;
 else if(x < 1.432) return 12;
 else if(x < 1.839) return 13;
 else if(x < 2.397) return 14;
 else return 15;
}
static inline int quant8bit(float x) {
 if(x < -4.395) return 0;
 else if(x < -4.056) return 1;
 else if(x < -3.829) return 2;
 else if(x < -3.654) return 3;
 else if(x < -3.510) return 4;
 else if(x < -3.387) return 5;
 else if(x < -3.279) return 6;
 else if(x < -3.183) return 7;
 else if(x < -3.095) return 8;
 else if(x < -3.015) return 9;
 // TODO -- they are so many of them!!
 else return 255;
}
static inline int quant2bit(float x) {
		if(x < -0.9674) return 0;
		else if(x < 0) return 1;
		else if(x < 0.9674) return 2;
		else return 3;
}
unsigned char dig2bit(float a, float b, float c, float d) {
		unsigned char A, B, C, D;	
		A = (unsigned char) (quant2bit(a) & 0x03) << 6;
		//std::cout << " a =  " << a << " A =  " << std::bitset<8>(A) << std::endl;
		B = (unsigned char) (quant2bit(b) & 0x03) << 4;
		//std::cout << " b =  " << b << " B =  " << std::bitset<8>(B) << std::endl;
		C = (unsigned char) (quant2bit(c) & 0x03) << 2;
		//std::cout << " c =  " << c << " C =  " << std::bitset<8>(C) << std::endl;
		D = (unsigned char) (quant2bit(d) & 0x03) << 0;
		//std::cout << " d =  " << d << " D =  " << std::bitset<8>(D) << std::endl;
		return A|B|C|D;
}
unsigned char dig4bit(float a, float b) {
		unsigned char A, B;	
		A = (unsigned char) (quant4bit(a) & 0x07) << 4;
		//std::cout << " a =  " << a << " A =  " << std::bitset<8>(A) << std::endl;
		B = (unsigned char) (quant4bit(b) & 0x07) << 0;
		//std::cout << " b =  " << b << " B =  " << std::bitset<8>(B) << std::endl;
		return A|B;
}
unsigned char dig8bit(float a) {
		return (unsigned char) quant8bit(a);
}
#endif // _REDIGIT_H_
