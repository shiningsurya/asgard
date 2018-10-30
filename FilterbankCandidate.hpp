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
								std::cerr << "FilterbankCandidate : Reached the end of CandidateList, resuming from start.\n" << std::endl; 
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
						fb.Unpack(d_fb, istart, nsamps);
						ddm.CoherentDD(d_fb, dd_tim);
						Freqs = operations::FreqTable(fch1, foff, nchans);
						idlays = operations::Delays(Freqs, (double) dm, (double) tsamp); 
						d_fb = operations::Fscrunch(d_fb, nchans, 32);
						ddm.InCoherentDD(d_fb, idlays, dd_fb); 
						dd_fb = operations::Fscrunch(dd_fb, nchans, 32);
						nchans = 32;
						curr++;
						return true;
				}
		public:
				std::string group, antenna;
				bool isKur;
				float sn, dm;
				long unsigned peak_index;
				double peak_time;
				int filterwidth;
				timeslice i0,i1, maxdelay;
				double fch1, foff, tsamp;
				int nchans;
				FloatVector d_fb, dd_fb, dd_tim;
				// d_fb -> dispersed filterbank
				// dd_fb > de-dispersed filterbank
				// dd-tim> de-dispersed time series
				FloatVector cldm, clsn, clwd;
				FloatVector Freqs;
				std::vector<timeslice> idlays;
				timeslice istart, istop, nsamps; 
				/*
				 *FilterbankCandidate(Filterbank& f, CandidateList& c) : ddm(f.nchans, f.tsamp, f.fch1, f.foff) {
				 *        group = f.group;
				 *        antenna = f.antenna;
				 *        isKur = f.isKur;
				 * should I do it this way?
				 *        fb = f;
				 *        cl = c; 
				 *        nchans = fb.nchans;
				 *        fch1 = fb.fch1;
				 *        foff = fb.foff;
				 *        tsamp = f.tsamp;
				 *        // initializing
				 *        // reading
				 *        read();	
				 *        }
				 */
				FilterbankCandidate(std::string f, std::string c) {
						// read filterbank
						fbr.Read(fb, f);
						group = fb.group;
						antenna = fb.antenna;
						isKur = fb.isKur;
						// read candidates
						cl = ReadCandidates(c, fb.tsamp);
						nchans = fb.nchans;
						fch1 = fb.fch1;
						foff = fb.foff;
						tsamp = fb.tsamp;
						// initialize
						curr = 0;
						std::for_each(cl.begin(), cl.end(), [this](Candidate& x) { cldm.push_back( (float) x.dm ); clsn.push_back( (float) x.sn ); clwd.push_back( (float) (x.filterwidth * tsamp )); });
						// reading
						ddm.CreatePlan(nchans, tsamp, fch1, foff);
						read();
				}
				timeslice size() { return (timeslice) cl.size(); }
				bool Next() {
						return read();
				}
};
#endif
