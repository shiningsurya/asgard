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
 // col 0
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
 else if(x < -2.941) return 10;
 else if(x < -2.872) return 11;
 else if(x < -2.807) return 12;
 else if(x < -2.746) return 13;
 else if(x < -2.688) return 14;
 else if(x < -2.633) return 15;
 // col 1
 else if(x < -2.581) return 16;
 else if(x < -2.531) return 17;
 else if(x < -2.482) return 18;
 else if(x < -2.436) return 19;
 else if(x < -2.392) return 20;
 else if(x < -2.348) return 21;
 else if(x < -2.307) return 22;
 else if(x < -2.266) return 23;
 else if(x < -2.227) return 24;
 else if(x < -2.189) return 25;
 else if(x < -2.152) return 26;
 else if(x < -2.116) return 27;
 else if(x < -2.081) return 28;
 else if(x < -2.046) return 29;
 else if(x < -2.013) return 30;
 else if(x < -1.980) return 31;
 // col 2
 else if(x < -1.948) return 32;
 else if(x < -1.916) return 33;
 else if(x < -1.885) return 34;
 else if(x < -1.855) return 35;
 else if(x < -1.826) return 36;
 else if(x < -1.796) return 37;
 else if(x < -1.768) return 38;
 else if(x < -1.739) return 39;
 else if(x < -1.712) return 40;
 else if(x < -1.684) return 41;
 else if(x < -1.657) return 42;
 else if(x < -1.631) return 43;
 else if(x < -1.603) return 44;
 else if(x < -1.579) return 45;
 else if(x < -1.554) return 46;
 else if(x < -1.528) return 47;
 // col 3
 else if(x < -1.504) return 48;
 else if(x < -1.479) return 49;
 else if(x < -1.455) return 50;
 else if(x < -1.431) return 51;
 else if(x < -1.407) return 52;
 else if(x < -1.384) return 53;
 else if(x < -1.361) return 54;
 else if(x < -1.338) return 55;
 else if(x < -1.316) return 56;
 else if(x < -1.293) return 57;
 else if(x < -1.271) return 58;
 else if(x < -1.249) return 59;
 else if(x < -1.227) return 60;
 else if(x < -1.205) return 61;
 else if(x < -1.184) return 62;
 else if(x < -1.163) return 63;
 // col 4
 else if(x < -1.142) return 64;
 else if(x < -1.121) return 65;
 else if(x < -1.100) return 66;
 else if(x < -1.079) return 67;
 else if(x < -1.059) return 68;
 else if(x < -1.039) return 69;
 else if(x < -1.018) return 70;
 else if(x < -0.9983) return 71;
 else if(x < -0.9784) return 72;
 else if(x < -0.9586) return 73;
 else if(x < -0.9390) return 74;
 else if(x < -0.9195) return 75;
 else if(x < -0.9001) return 76;
 else if(x < -0.8808) return 77;
 else if(x < -0.8616) return 78;
 else if(x < -0.8425) return 79;
 // col 5
 else if(x < -0.8236) return 80;
 else if(x < -0.8047) return 81;
 else if(x < -0.7859) return 82;
 else if(x < -0.7672) return 83;
 else if(x < -0.7486) return 84;
 else if(x < -0.7301) return 85;
 else if(x < -0.7117) return 86;
 else if(x < -0.6933) return 87;
 else if(x < -0.6751) return 88;
 else if(x < -0.6569) return 89;
 else if(x < -0.6387) return 90;
 else if(x < -0.6207) return 91;
 else if(x < -0.6027) return 92;
 else if(x < -0.5848) return 93;
 else if(x < -0.5669) return 94;
 else if(x < -0.5491) return 95;
 // col 6
 else if(x < -0.5314) return 96;
 else if(x < -0.5137) return 97;
 else if(x < -0.4961) return 98;
 else if(x < -0.4785) return 99;
 else if(x < -0.4609) return 100;
 else if(x < -0.4435) return 101;
 else if(x < -0.4260) return 102;
 else if(x < -0.4086) return 103;
 else if(x < -0.3913) return 104;
 else if(x < -0.3740) return 105;
 else if(x < -0.3567) return 106;
 else if(x < -0.3394) return 107;
 else if(x < -0.3222) return 108;
 else if(x < -0.3050) return 109;
 else if(x < -0.2879) return 110;
 else if(x < -0.2708) return 111;
 // col 7
 else if(x < -0.2537) return 112;
 else if(x < -0.2366) return 113;
 else if(x < -0.2196) return 114;
 else if(x < -0.2025) return 115;
 else if(x < -0.1855) return 116;
 else if(x < -0.1686) return 117;
 else if(x < -0.1516) return 118;
 else if(x < -0.1346) return 119;
 else if(x < -0.1177) return 120;
 else if(x < -0.1008) return 121;
 else if(x < -0.08384) return 122;
 else if(x < -0.06694) return 123;
 else if(x < -0.05003) return 124;
 else if(x < -0.03318) return 125;
 else if(x < -0.01637) return 126;
 else if(x < 0.0) return 127;
 // col 8 -- this is tricky
 else if(x == 0.0) return 128;
 else if(x < 0.01637) return 129;
 else if(x < 0.03318) return 130;
 else if(x < 0.05003) return 131;
 else if(x < 0.06694) return 132;
 else if(x < 0.08384) return 133;
 else if(x < 0.1008) return 134;
 else if(x < 0.1177) return 135;
 else if(x < 0.1346) return 136;
 else if(x < 0.1516) return 137;
 else if(x < 0.1686) return 138;
 else if(x < 0.1855) return 139;
 else if(x < 0.2025) return 140;
 else if(x < 0.2196) return 141;
 else if(x < 0.2366) return 142;
 else if(x < 0.2537) return 143;
 // col 9
 else if(x < 0.2708) return 144;
 else if(x < 0.2879) return 145;
 else if(x < 0.3050) return 146;
 else if(x < 0.3222) return 147;
 else if(x < 0.3394) return 148;
 else if(x < 0.3567) return 149;
 else if(x < 0.3740) return 150;
 else if(x < 0.3913) return 151;
 else if(x < 0.4086) return 152;
 else if(x < 0.4260) return 153;
 else if(x < 0.4435) return 154;
 else if(x < 0.4609) return 155;
 else if(x < 0.4785) return 156;
 else if(x < 0.4961) return 157;
 else if(x < 0.5137) return 158;
 else if(x < 0.5314) return 159;
 // col 10
 else if(x < 0.5491) return 160;
 else if(x < 0.5669) return 161;
 else if(x < 0.5848) return 162;
 else if(x < 0.6027) return 163;
 else if(x < 0.6207) return 164;
 else if(x < 0.6387) return 165;
 else if(x < 0.6569) return 166;
 else if(x < 0.6751) return 167;
 else if(x < 0.6933) return 168;
 else if(x < 0.7117) return 169;
 else if(x < 0.7301) return 170;
 else if(x < 0.7486) return 171;
 else if(x < 0.7672) return 172;
 else if(x < 0.7859) return 173;
 else if(x < 0.8047) return 174;
 else if(x < 0.8236) return 175;
 // col 11
 else if(x < 0.8425) return 176;
 else if(x < 0.8616) return 177;
 else if(x < 0.8808) return 178;
 else if(x < 0.9001) return 179;
 else if(x < 0.9195) return 180;
 else if(x < 0.9390) return 181;
 else if(x < 0.9586) return 182;
 else if(x < 0.9784) return 183;
 else if(x < 0.9983) return 184;
 else if(x < 1.018) return 185;
 else if(x < 1.039) return 186;
 else if(x < 1.059) return 187;
 else if(x < 1.079) return 188;
 else if(x < 1.100) return 189;
 else if(x < 1.121) return 190;
 else if(x < 1.142) return 191;
 // col 12
 else if(x < 1.163) return 192;
 else if(x < 1.184) return 193;
 else if(x < 1.205) return 194;
 else if(x < 1.227) return 195;
 else if(x < 1.249) return 196;
 else if(x < 1.271) return 197;
 else if(x < 1.293) return 198;
 else if(x < 1.316) return 199;
 else if(x < 1.338) return 200;
 else if(x < 1.361) return 201;
 else if(x < 1.384) return 202;
 else if(x < 1.407) return 203;
 else if(x < 1.431) return 204;
 else if(x < 1.455) return 205;
 else if(x < 1.479) return 206;
 else if(x < 1.504) return 207;
 // col 13
 else if(x < 1.528) return 208;
 else if(x < 1.554) return 209;
 else if(x < 1.579) return 210;
 else if(x < 1.603) return 211;
 else if(x < 1.631) return 212;
 else if(x < 1.657) return 213;
 else if(x < 1.684) return 214;
 else if(x < 1.712) return 215;
 else if(x < 1.739) return 216;
 else if(x < 1.768) return 217;
 else if(x < 1.796) return 218;
 else if(x < 1.826) return 219;
 else if(x < 1.855) return 220;
 else if(x < 1.885) return 221;
 else if(x < 1.916) return 222;
 else if(x < 1.948) return 223;
 // col 14
 else if(x < 1.980) return 224;
 else if(x < 2.013) return 225;
 else if(x < 2.046) return 226;
 else if(x < 2.081) return 227;
 else if(x < 2.116) return 228;
 else if(x < 2.152) return 229;
 else if(x < 2.189) return 230;
 else if(x < 2.227) return 231;
 else if(x < 2.266) return 232;
 else if(x < 2.307) return 233;
 else if(x < 2.348) return 234;
 else if(x < 2.392) return 235;
 else if(x < 2.436) return 236;
 else if(x < 2.482) return 237;
 else if(x < 2.531) return 238;
 else if(x < 2.581) return 239;
 // col 15
 else if(x < 2.633) return 240;
 else if(x < 2.688) return 241;
 else if(x < 2.746) return 242;
 else if(x < 2.807) return 243;
 else if(x < 2.872) return 244;
 else if(x < 2.941) return 245;
 else if(x < 3.015) return 246;
 else if(x < 3.095) return 247;
 else if(x < 3.183) return 248;
 else if(x < 3.279) return 249;
 else if(x < 3.387) return 250;
 else if(x < 3.510) return 251;
 else if(x < 3.654) return 252;
 else if(x < 3.829) return 253;
 else if(x < 4.056) return 254;
 else return 255;
}
static inline int quant2bit(float x) {
		if(x < -0.9674) return 0;
		else if(x < 0) return 1;
		else if(x < 0.9674) return 2;
		else return 3;
}
unsigned char dig2bit_sf(float a, float b, float c, float d) {
		unsigned char A, B, C, D;	
		A = (unsigned char) (static_cast<unsigned char>(a) & 0x03) << 6;
		//std::cout << " a =  " << a << " A =  " << std::bitset<8>(A) << std::endl;
		B = (unsigned char) (static_cast<unsigned char>(b) & 0x03) << 4;
		//std::cout << " b =  " << b << " B =  " << std::bitset<8>(B) << std::endl;
		C = (unsigned char) (static_cast<unsigned char>(c) & 0x03) << 2;
		//std::cout << " c =  " << c << " C =  " << std::bitset<8>(C) << std::endl;
		D = (unsigned char) (static_cast<unsigned char>(d) & 0x03) << 0;
		//std::cout << " d =  " << d << " D =  " << std::bitset<8>(D) << std::endl;
		return A|B|C|D;
}
unsigned char dig4bit_sf(float a, float b) {
		unsigned char A, B;	
		A = (unsigned char) (static_cast<unsigned char>(a) & 0x07) << 4;
		//std::cout << " a =  " << a << " A =  " << std::bitset<8>(A) << std::endl;
		B = (unsigned char) (static_cast<unsigned char>(b) & 0x07) << 0;
		//std::cout << " b =  " << b << " B =  " << std::bitset<8>(B) << std::endl;
		return A|B;
}
unsigned char dig8bit_sf(float a) {
		return (unsigned char) static_cast<unsigned char>(a);
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

void unpack2bit(unsigned char& dc, float& a, float& b, float& c, float& d) {
  a  = (float) (dc & LO2BITS); 
  b  = (float) ((dc & LOMED2BITS) >> 2); 
  c  = (float) ((dc & UPMED2BITS) >> 4); 
  d  = (float) ((dc & HI2BITS) >> 6); 
}
void unpack4bit(unsigned char& dc, float& a, float& b) {
  a  = (float) (dc & LO4BITS); 
  b  = (float) ((dc & UP4BITS) >> 2); 
}
void unpack8bit(unsigned char& dc, float& a) {
  a  = (float) (dc); 
}
#endif // _REDIGIT_H_
