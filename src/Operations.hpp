#include "asgard.hpp"
#ifndef OPERATIONS_H
#define OPERATIONS_H
namespace operations {
 void Float2Ptr(FloatVector in, PtrFloat out) {
	for(int i = 0; i < in.size(); i++) out[i] = in[i];
 }
 void Whiten(PtrFloat in, timeslice tnsamp);
 void FreqShape(float * in, timeslice nsamps, int nchans, float * out);
 void TimeShape(float * in, timeslice nsamps, int nchans, float * out);
 void Fscrunch(float * in, int nchans_in, timeslice nsamps, int nchans_out, float * ret);
 void Crunch(float * in, const int nchans_in, const timeslice nsamps_in, const int nchans_out, const timeslice nsamps_out, float * ret); 
 FloatVector FreqTable(Filterbank& f);
 FloatVector FreqTable(float fch1, float foff, int nchans);
 void FreqTable(float fch1, float foff, int nchans, PtrFloat out);
 std::vector<timeslice> Delays(FloatVector freqs, double dm, double tsamp);
 FloatVector TimeAxis(float dt, timeslice start, timeslice stop); 
 auto Delay = [](float f1, float f2, double dm) ->  double {return(4148.741601*((1.0/f1/f1)-(1.0/f2/f2))*dm);};
 void DynamicColor(PtrFloat inp, timeslice nsamps, int nchans, int csize);
}
void operations::Whiten(PtrFloat inp, timeslice tnsamp) {
 float mean = 0.0f, variance = 0.0f, sd = 0.0f;
 // mean
 for(timeslice i = 0; i < tnsamp; i++) mean += inp[i];
 mean /= tnsamp;
 // variance
 for(timeslice i = 0; i < tnsamp; i++) variance += pow(inp[i] - mean, 2);
 sd = sqrt(variance/tnsamp);
 // whiten
 for(timeslice i = 0; i < tnsamp; i++)
	inp[i] = (inp[i] - mean) / sd;
}
void operations::FreqTable(float fch1, float foff, int nchans, PtrFloat out) {
 for(int i = 0; i < nchans; i++) out[i] = ( fch1 + i * foff );
}
void operations::DynamicColor(PtrFloat inp, timeslice nsamps, int nchans, int csize) {
 if(csize != 6) 
	std::cerr << "opertions::DynamicColor csize " << csize << " not supported!" << std::endl;
 // inplace changing the input array to 
 // -1 ---> csize-1
 // to have dynamic color scheme
 // estimate mean and variance
 double mean=0.0f, variance=0.0f, sd=0.0f;
 float M = 3.0f, L = 1.0f, temp = 0.0f;
 unsigned int fac = 3;
 float slope, intercept;
 timeslice tnsamp = nsamps * nchans;
 // mean
 for(timeslice i = 0; i < tnsamp; i++) mean += inp[i];
 mean /= tnsamp;
 // variance
 for(timeslice i = 0; i < tnsamp; i++) variance += pow(inp[i] - mean, 2);
 sd = sqrt(variance/tnsamp);
 // mapping
 slope = M - L;
 intercept = M;
 for(timeslice i = 0; i < tnsamp; i++) {
	temp = (inp[i] - mean) / sd;
	inp[i] = (slope * temp) - intercept;
 }
}
void operations::FreqShape(float * in, timeslice nsamps, int nchans, float * out) {
 for(int ichan = 0; ichan < nchans; ichan++) {
	out[ichan] = 0.0f;
	for(timeslice isamp = 0; isamp < nsamps; isamp++) {
	 out[ichan] += in[isamp*nchans + ichan];
	}
	out[ichan] /= nsamps;
 }
}
void operations::TimeShape(float * in, timeslice nsamps, int nchans, float * out) {
 for(timeslice isamp = 0; isamp < nsamps; isamp++) {
	out[isamp] = 0.0f;
	for(int ichan = 0; ichan < nchans; ichan++) {
	 out[isamp] += in[isamp*nchans + ichan];
	}
	out[isamp] /= nchans;
 }
}
void operations::Crunch(float * in, const int nchans_in, const timeslice nsamps_in, const int nchans_out, const timeslice nsamps_out, float * ret) {
 // assuming in's and out's are sensible
 int df = nchans_in / nchans_out;
 timeslice di = nsamps_in / nsamps_out;
 float dd = df * di;
 float xf;
 // get going
 for(timeslice i = 0; i < nsamps_out; i++) {
	for(int c = 0; c < nchans_out; c++) {
	 xf = 0.0f;
	 for(timeslice ii = i*di; ii < (i+1)*di; ii++)
		for(int cc = c*df; cc < (c+1)*df; cc++)
		 xf += in[ii*nchans_in + cc];
	 ret[i*nchans_out + c] = xf/dd;
	}
 }
}
FloatVector operations::TimeAxis(float dt, timeslice start, timeslice stop) {
 FloatVector ret (stop - start);
 for(int i = 0; i < stop - start; i++) ret[i] = (start + i)*dt;
 return ret;
}
FloatVector operations::FreqTable(float fch1, float foff, int nchans) {
 FloatVector ret; 
 for(int i = 0; i < nchans; i++) ret.push_back( fch1 + i * foff );
 return ret;
}
FloatVector operations::FreqTable(Filterbank& f) {
 FloatVector ret;
 for(int i = 0; i < f.nchans; i++) ret.push_back( (float)f.fch1 + (i * (float)f.foff) );
 return ret;
}	
std::vector<timeslice> operations::Delays(FloatVector freqs, double dm, double tsamp) {
 std::vector<timeslice> ret;
 float f1 = freqs[0];
 for(float f : freqs) ret.push_back( (timeslice) (operations::Delay(f, f1, dm) / tsamp) );
 return ret;
}
void operations::Fscrunch(float * in, int nchans_in, timeslice nsamps, int nchans_out, float * ret) {
 // Assuming in's and out's are sensible
 // fastest changing axis is frequency
 int df = nchans_in / nchans_out;
 float xf, k1 = 1.0f/df;
 // get going
 for(timeslice i = 0; i < nsamps; i++) {
	for(int k = 0; k < df; k++) {
	 xf = 0.0f;
	 for(int j = 0; j < nchans_out; j++) {
		//std::cout << idf << " " << k << "  " << i << " " << nsamps << std::endl;
		ret[i*nchans_out + j] += k1 * in[i*nchans_in + j + k*nchans_out];
	 }
	 //ret[i*nchans_out + j] = xf;
	}
 }
}
#endif
