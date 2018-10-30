#include "asgard.hpp"
#ifndef OPERATIONS_H
#define OPERATIONS_H
namespace operations {
		FloatVector Fscrunch(FloatVector& in, int nchans_in, int nchans_out);
	 	FloatVector FreqTable(Filterbank& f);
	 	FloatVector FreqTable(float fch1, float foff, int nchans);
		std::vector<timeslice> Delays(FloatVector freqs, double dm, double tsamp);
		FloatVector TimeAxis(float dt, timeslice start, timeslice stop); 
		auto Delay = [](float f1, float f2, double dm) ->  double {return(4148.741601*((1.0/f1/f1)-(1.0/f2/f2))*dm);};
}
FloatVector operations::TimeAxis(float dt, timeslice start, timeslice stop) {
		FloatVector ret (stop - start);
		for(int i = 0; i < stop - start; i++) ret[i] = (start + i)*dt;
		return ret;
}
FloatVector operations::FreqTable(float fch1, float foff, int nchans) {
		FloatVector ret (nchans);
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
FloatVector operations::Fscrunch(FloatVector& in, int nchans_in, int nchans_out) {
		// Assuming in's and out's are sensible
		// fastest changing axis is frequency
		int df = nchans_in / nchans_out;
		timeslice nsamps = in.size() / nchans_in;
		FloatVector ret;
		ret.reserve(nsamps * nchans_out);
		int idf;
		float xf;
		// get going
		for(timeslice i = 0; i < nsamps; i++) {
				idf = 0;
				for(int j = 0; j < nchans_out; j++) {
						xf = 0.0f;
						for(int k = 0; k < df; k++) {
								xf += in[i*nchans_in + idf];
								idf++;
						}
						ret[i*nchans_out + j] = xf;
				}
				if(idf != nchans_in) { std::cerr << "Operations::Fscrunch Fatal error\n";}
		}
		//
		return ret;
}
#endif
