

// NOTE: I am making a move from float* to FloatVector
// https://github.com/isocpp/CppCoreGuidelines/issues/493
// ^ convinced me that FloatVector is just as fast as float* 

class FilterbankCandidate {
		private:
				DedispManager ddm;
				FloatVector dat;
				size_type curr;
				FilterbankReader fbr;
				Filterbank fb;
				CandidateList cl;
				void read() {
						//what reads and changes public members
						if(curr >= cl.size()) {
								std::cerr << "FilterbankCandidate : Reached the end of CandidateList, resuming from start.\n" << std::endl; 
								curr = 0;
						}
						// load required candidate info
						sn = cl[curr].sn;
						dm = cl[curr].dm;
						filterwidth = cl[curr].filterwidth;
						i0 = cl[curr].i0;
						i1 = cl[curr].i1;
						peak_time = cl[curr].peak_time;
						peak_index = cl[curr].peak_index;
						// load filterbank
						//
						dat = fb.Unpack(...);


				}
		public:
				float sn, dm;
				long unsigned peak_index;
				double peak_time;
				int filterwidth;
				timeslice i0,i1, maxdelay;
				FloatVector d_fb, dd_fb, dd_tim;
				timeslice d_fb_i0, dd_fb_i0, dd_tim_i0;
				timeslice d_fb_i1, dd_fb_i1, dd_tim_i1;
				// d_fb -> dispersed filterbank
				// dd_fb > de-dispersed filterbank
				// dd-tim> de-dispersed time series
				Candidate candidate;
				FilterbankCandidate(Filterbank& f, CandidateList& cl) : ddm(f.nchans, f.tsamp, f.fch1, f.foff) {
				// should I do it this way?
						fb = f;
						cl = cl; 
						// initializing
						curr = 0;
				}
				size_type size() { return cl.size(); }
};
