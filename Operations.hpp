#include "asgard.hpp"
#include "dedisp.h"

namespace operations {
		void SingleAntennaCoadd(float * in, float * out, timeslice wid, int nchans);
		void AntennaCoadd(std::vector<float*> in, float * out, timeslice wid, int nchans);
		std::vector<timeslice> ExtractJuice(Filterbank& f, Candidate& c, float * out, int wdfac);
		void Dedisperse(float * out, Filterbank& f, Candidate& c);
		void Dedisperse(float * in, float * out, double dm, int nchans, double tsamp, double fch1, double foff, timeslice nsamps);
	 	FloatVector FreqTable(Filterbank& f);
		FloatVector Delays(FloatVector freqs, double dm);
		auto Delay = [](float f1, float f2, double dm) ->  double {return(4148.741601*((1.0/f1/f1)-(1.0/f2/f2))*dm);};
}

std::vector<timeslice> operations::ExtractJuice(Filterbank& f, Candidate& c, float * out, int wdfac) {
		std::vector<timeslice> ret;
		if(out != NULL) {
				std::cerr << "Why do you want me to put juice in already initialized array?\n";
				std::cerr << "Freeing the memory\n";
				delete[] out;
		}
		timeslice ndur = c.i1 - c.i0;	
		timeslice wid = wdfac*ndur; 
		timeslice ndur_half = 0.5*(c.i1 + c.i0); // midpoint
		timeslice ni0 = ndur_half - .5*wid;
		timeslice ni1 = ndur_half + .5*wid;
		/*
		 *std::cout << "Totalsamp: " << f.totalsamp << std::endl;
		 *std::cout << ni0 << " " << ni1 << " " << ndur_half << " " << wid << " " ;
		 */
		if(ni1 > f.totalsamp) {
				ni1 = f.totalsamp;
				wid = ni1 - ndur_half;
				ni0 = ndur_half - .5*wid;
		}
		out = new float[wid*f.nchans];	
		// I am an idiot!
		f.Unpack(out, ni0, wid); // reading
		ret.push_back( wid );
		ret.push_back( ni0 );
		ret.push_back( ni1 );
		return ret; 
}
void operations::SingleAntennaCoadd(float * in, float * out, timeslice wid, int nchans) {
		for(timeslice i = 0; i < wid; i++) {
				out[i] = 0.0f;
				for(int j = 0; j < nchans; j++) {
						out[i] += in[i*nchans + j];
				}
				out[i] /= nchans;
		}
}
FloatVector operations::FreqTable(Filterbank& f) {
		FloatVector ret;
		for(int i = 0; i < f.nchans; i++) ret.push_back( (float)f.fch1 - (i * (float)f.foff) );
		return ret;
}	

void operations::Dedisperse(float * in, float * out, double dm, int nchans, double tsamp, double fch1, double foff, timeslice nsamps) {
		if(in == NULL || out == NULL) {
				std::cerr << "Input to Dedisperse are fatal!\n";
				if(in == NULL && out != NULL) std::cerr << "Uninitialized Input pointer\n";
				else if(in != NULL && out == NULL) std::cerr << "Uninitialized Output pointer\n";
				else std::cerr << "Uninitialized both pointers\n";
				//exit(1);
		}
		dedisp_plan dplan;
		dedisp_error error;
		error = dedisp_create_plan(&dplan, nchans, tsamp, fch1, foff);
		if( error != DEDISP_NO_ERROR ) std::cerr << "\nERROR: Could not create dedispersion plan: " <<  dedisp_get_error_string(error) << std::endl;
		/// Dedisp
		error = dedisp_execute(dplan, (dedisp_size)nsamps, (const dedisp_byte*)in, sizeof(float)*8, (dedisp_byte*)out, 8*sizeof(float), DEDISP_USE_DEFAULT);
		/// Dedisp 
		if( error != DEDISP_NO_ERROR ) std::cerr << "\nERROR: Could not execute dedispersion plan: " <<  dedisp_get_error_string(error) << std::endl;
		dedisp_destroy_plan(dplan);

}

void operations::Dedisperse(float * out, Filterbank& f, Candidate& c) {
		/// Read data 
		float * input;
		timeslice nsamps = (timeslice) (operations::ExtractJuice(f, c, input, 5)[0] * f.nchans);
		/// Read data
		if(out == NULL) {
				std::cerr << "Dedisperse output array not initialized\n";
				out = new float[nsamps*f.nchans];
				std::cerr << "Now initializing...\n";
		}
		operations::Dedisperse(input, out, c.dm, f.nchans, f.tsamp, f.fch1, f.foff, nsamps);
}

FloatVector operations::Delays(FloatVector freqs, double dm) {
		FloatVector ret;
		sort(freqs.begin(), freqs.end());
		float f1 = freqs[freqs.size()-1];
		for(float f : freqs) ret.push_back( operations::Delay(f, f1, dm) );
		return ret;
}
