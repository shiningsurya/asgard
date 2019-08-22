#include "asgard.hpp"
#ifndef FILTERBANK_H
#include "Filterbank.hpp"
#endif
#ifndef CANDIDATE_H
#include "Candidate.hpp"
#endif
#include "Operations.hpp"
#include "Dedisperser.hpp"
#ifndef FILTERBANKCANDIDATE_H
#define FILTERBANKCANDIDATE_H
// NOTE: I am making a move from float* to FloatVector
// https://github.com/isocpp/CppCoreGuidelines/issues/493
// ^ convinced me that FloatVector is just as fast as float* 
class FilterbankCandidate {
 private:
 // filters
 float dm_l, dm_h;
 float sn_l, sn_h;
 // filterbank
	Filterbank fb;
	// candidate
	CandidateList cl;
	// ddm
	DedispManager ddm;
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
	 // delays
	 delay_table = operations::Delays(frequency_table, (double) dm, (double) tsamp); 
	 dm_delay = delay_table.back() * tsamp;
	 // slicing logic
	 timeslice mid = .5 * (i0 + i1);
	 if(mid < (1 * maxdelay)) istart = 0;
	 else istart = mid - (1 * maxdelay);
	 istop  = mid + (2 * maxdelay);
	 // load filterbank
	 nsamps = istop - istart;
	 dd_nsamps = nsamps - maxdelay;
	 fb.Unpack(d_fb, istart, nsamps);
	 // coherent dedisp
	 ddm.CoherentDD (d_fb, nsamps, sndm);
	 // profile
	 auto closest_dm = ddm.ClosestDM ( dm );
	 cdm = dmlist[closest_dm];
	 PtrFloat dd_sndm = sndm + (closest_dm * dd_nsamps);
	 std::copy ( dd_sndm, dd_sndm + dd_nsamps, dd_tim );
	 // incoherent dedisp
	 ddm.InCoherentDD(d_fb, delay_table, dd_nsamps, dd_fb); 
	 curr++;
	 return true;
	}
 public:
	timeslice curr;
 double duration, tstart;
 int nbits;
	unsigned int bmin, bmax;
	std::string group, antenna, source_name;
	bool isKur;
	float sn, dm, cdm;
	long unsigned peak_index;
	double peak_time, dm_delay;
	int filterwidth;
	timeslice i0,i1, maxdelay;
	double fch1, foff, tsamp;
	int nchans;
	PtrFloat d_fb, dd_fb, dd_tim, sndm;
	// d_fb   -> dispersed filterbank
	// dd_fb  -> de-dispersed filterbank
	// dd_tim -> de-dispersed time series
	// sndm   -> S/N as function of DM, time
	FloatVector frequency_table;
	FloatVector dmlist;
	timeslice dm_count;
	timeslice istart, istop, nsamps, dd_nsamps; 
	std::vector<timeslice> delay_table;
	FilterbankCandidate(std::string f, std::string c) : 
	d_fb(nullptr), dd_fb(nullptr), dd_tim(nullptr), sndm(nullptr) {
	 // read filterbank
  FilterbankReader fbr;
	 fbr.Read(fb, f);
	 group   = fb.group;
	 antenna = fb.antenna;
	 isKur   = fb.isKur;
	 bmin    = fb.bmin;
	 bmax    = fb.bmax;
	 source_name = fb.source_name;
	 duration = fb.duration;
	 nbits = fb.nbits;
	 tstart = fb.tstart;
	 nchans = fb.nchans;
	 fch1   = fb.fch1;
	 foff   = fb.foff;
	 tsamp  = fb.tsamp;
	 // read candidates
	 cl = ReadCandidates(c, fb.tsamp);
	 // initialize
	 curr = 0;
	 frequency_table = operations::FreqTable(fch1, foff, nchans);
	}
	void PrepDedispManager (float dd0, float dd1, unsigned int fw) {
	 ddm.CreatePlan(nchans, tsamp, fch1, foff);
	 maxdelay = ddm.SetDM (dd0, dd1, fw);
	 dm_count   = ddm.DMList (dmlist);
	 nsamps = 3 * maxdelay;
	 dd_nsamps = nsamps - maxdelay;
	 // allocate memory
	 d_fb   = new float[ nsamps * nchans ];
	 dd_fb  = new float[ dd_nsamps * nchans ];
	 dd_tim = new float[ dd_nsamps];
	 sndm   = new float[ dd_nsamps * dm_count];
	}
	void DM_Filter (float low = 0, float high = 2000) {
	 dm_l = low; dm_h = high;
	 CandidateList filtercl;
	 std::for_each (cl.cbegin(), cl.cend(), [&filtercl, &low, &high] (const Candidate& x) {
	   if ( x.dm > low && x.dm < high) filtercl.push_back ( x );
	 });
	 std::swap ( cl, filtercl );
	}
	void SN_Filter (float low = 0, float high = 5000) {
	 sn_l = low; sn_h = high;
	 CandidateList filtercl;
	 std::for_each (cl.cbegin(), cl.cend(), [&filtercl, &low, &high] (const Candidate& x) {
	   if ( x.sn > low && x.sn < high) filtercl.push_back ( x );
	 });
	 std::swap ( cl, filtercl );
	}
	~FilterbankCandidate() {
	 // I will have to use smart pointers at one point 
	 if(d_fb   != nullptr) {delete[] d_fb; d_fb = nullptr;}
	 if(dd_fb  != nullptr) {delete[] dd_fb; dd_fb = nullptr;}
	 if(dd_tim != nullptr) {delete[] dd_tim; dd_tim = nullptr;}
	 if(sndm   != nullptr) {delete[] sndm; sndm= nullptr;}
	}
	timeslice size() { return (timeslice) cl.size(); }
	bool Next() {
	 return read();
	}
};
#endif
