#include "asgard.hpp"
#include "Operations.hpp"
#include "Dedisperser.hpp"
#ifndef FILTERBANKCANDIDATE_H
#define FILTERBANKCANDIDATE_H
// NOTE: I am making a move from float* to FloatVector
// https://github.com/isocpp/CppCoreGuidelines/issues/493
// ^ convinced me that FloatVector is just as fast as float* 
class FilterbankCandidate {
 private:
	DedispManager ddm;
	FloatVector dat;
	timeslice curr;
	FilterbankReader fbr;
	Filterbank fb;
	CandidateList cl;
	bool read() {
	 //what reads and changes public members
	 if(curr >= cl.size()) {
		std::cerr << "FilterbankCandidate : Reached the end of CandidateList." << std::endl; 
		curr = 0;
		return false;
	 }
	 // load required candidate info
	 sn = cl[curr].sn;
	 dm = cl[curr].dm;
	 filterwidth = cl[curr].filterwidth;
	 i0 = cl[curr].i0;
	 i1 = cl[curr].i1;
	 peak_time = cl[curr].peak_time;
	 peak_index = cl[curr].peak_index;
	 maxdelay = ddm.SetDM(dm, filterwidth);
	 // logic
	 timeslice mid = .5 * (i0 + i1);
	 istart = mid - (1 * maxdelay);
	 istop  = mid + (2 * maxdelay);
	 // load filterbank
	 nsamps = istop - istart;
	 // take juice
	 d_fb = new float[nsamps*nchans];
	 fb.Unpack(d_fb, istart, nsamps);
	 // coherent dedisp
	 dd_tim = new float[nsamps - maxdelay];
	 ddm.CoherentDD(d_fb, nsamps, dd_tim);
	 // incoherent dedisp
	 delay_table = operations::Delays(frequency_table, (double) dm, (double) tsamp); 
	 dd_fb = new float[nchans*(nsamps - maxdelay)];
	 ddm.InCoherentDD(d_fb, delay_table, nsamps - maxdelay, dd_fb); 
	 //for(int i = 0; i < nsamps*nchans; i++) std::cout << dd_fb[i] << std::endl;
	 // Fscrunching
	 /*
		*d_fb = operations::Fscrunch(d_fb, nchans, 32);
		*dd_fb = operations::Fscrunch(dd_fb, nchans, 32);
		*foff *= nchans/32;
		*nchans = 32;
		*/
	 curr++;
	 return true;
	}
 public:
	unsigned int bmin, bmax;
	std::string group, antenna;
	bool isKur;
	float sn, dm;
	long unsigned peak_index;
	double peak_time;
	int filterwidth;
	timeslice i0,i1, maxdelay;
	double fch1, foff, tsamp;
	int nchans;
	PtrFloat d_fb, dd_fb, dd_tim;
	// d_fb -> dispersed filterbank
	// dd_fb > de-dispersed filterbank
	// dd_tim> de-dispersed time series
	FloatVector cldm, clsn, clwd;
	FloatVector frequency_table;
	std::vector<timeslice> delay_table;
	timeslice istart, istop, nsamps; 
	FilterbankCandidate(std::string f, std::string c) {
	 // read filterbank
	 fbr.Read(fb, f);
	 group   = fb.group;
	 antenna = fb.antenna;
	 isKur   = fb.isKur;
	 bmin    = fb.bmin;
	 bmax    = fb.bmax;
	 // read candidates
	 cl = ReadCandidates(c, fb.tsamp);
	 nchans = fb.nchans;
	 fch1   = fb.fch1;
	 foff   = fb.foff;
	 tsamp  = fb.tsamp;
	 // initialize
	 curr = 0;
	 std::for_each(cl.begin(), cl.end(), [this](Candidate& x) { cldm.push_back( (float) x.dm ); clsn.push_back( (float) x.sn ); clwd.push_back( (float) (x.filterwidth * tsamp )); });
	 frequency_table = operations::FreqTable(fch1, foff, nchans);
	 // reading
	 ddm.CreatePlan(nchans, tsamp, fch1, foff);
	 read();
	}
	~FilterbankCandidate() {
	 // I will have to use smart pointers at one point 
	 if(d_fb != nullptr) {delete[] d_fb; d_fb = nullptr;}
	 if(dd_fb != nullptr) {delete[] dd_fb; dd_fb = nullptr;}
	 if(dd_tim != nullptr) {delete[] dd_tim; dd_tim = nullptr;}
	}
	timeslice size() { return (timeslice) cl.size(); }
	bool Next() {
	 if(d_fb != nullptr) {delete[] d_fb; d_fb = nullptr;}
	 if(dd_fb != nullptr) {delete[] dd_fb; dd_fb = nullptr;}
	 if(dd_tim != nullptr) {delete[] dd_tim; dd_tim = nullptr;}
	 return read();
	}
};
#endif
