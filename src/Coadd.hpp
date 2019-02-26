#ifndef ASGARD_H
#include "asgard.hpp"
#endif
#ifndef COADD_H
#define COADD_H
#include "Filterbank.hpp"

FilterbankList FLFromDE(DEList& x) {
		FilterbankList ret;
		FilterbankReader fbr;
		for(fs::directory_entry& de : x) {
				Filterbank xx;
				fbr.Read(xx, de.path().string()); 
				ret.push_back(xx);
		}
		return ret;
		// I would like to take time to tell you that
		// I took 10 mins into debugging why this function was 
		// going haywire.
		// And the reason is self explanatory:
		// I forgot "return ret"
}

class Coadd {
		private:
				//std::string method("MAD");
				bool returnFil, returnDada;
				std::string filename;
				double load_secs;
				float mintstart, maxdur;
				timeslice retsize, i0, ddit;
				FilterbankWriter fbw;
				void _work_2(Filterbank& x, Filterbank& y, PtrFloatUnique  ret ) {
						// ret is initialized before
				}
				void _serial(FilterbankList& fl, FilterbankWriter& bw) {
						/**
						 * I am making a heavy assumption:
						 * same tsamp and same nchans in all the filterbanks
						 * TODO Make this general
						 * */
						// serial code
						timeslice tstep = load_secs / fl[0].tsamp;
						timeslice nsteps = maxdur / load_secs;
						timeslice i0 = 0;
						int NumAnt = 0;
						timeslice tdat_size = tstep * fl[0].nchans;
						std::size_t tdat_memsize = tdat_size * sizeof(float);
						PtrFloat tdat = new float[tdat_size](); // uniform initialization
						PtrFloat fbdat = new float[tdat_size](); // uniform initialization
						float currTime = 0.0f;
						for(timeslice i = 0; i < nsteps; i++) {
								NumAnt = 0;
								for(Filterbank& f : fl) {
										// i0 * f.tsamp <-- timestep
										currTime = i0 * f.tsamp;
										if(currTime < f.duration && currTime >= (f.tstart - mintstart) ) {  
												f.Unpack(fbdat, i0, tstep);
												NumAnt++;
												for(timeslice j = 0; j < tdat_size; j++) {
														tdat[j] += fbdat[j]; 
														fbdat[j] = 0.0f;
												}
										}
								}
								for(timeslice j = 0; j < tdat_size; j++) {
										if(NumAnt == 0) tdat[j] = 0.0f;
										else tdat[j] = std::floor(tdat[j] / NumAnt);
										//else tdat[j] = std::floor(tdat[j]);
								}
								bw.WriteFBdata(tdat,ddit, tdat_size);
								for(timeslice j = 0; j < tdat_size; j++) tdat[j] = 0.0f;
								i0 += tstep;
								ddit += tdat_size/4;
								// * 4 bc 1 byte = 4 samples
						}	
						delete[] tdat;
						delete[] fbdat;
				}
		public:
				Coadd(std::string fn) {
						returnFil = true;
						filename = fn;
						load_secs = 2.0f;
				}
				Coadd() {
						returnFil = false;
				}
				void Work(DEList& x) {
						FilterbankList fl; 
						fl = FLFromDE(x);
						Work( fl );
				}
				void Work(FilterbankList& fl) {
						// compute max duration here
						FloatVector duration_l, tstart_l;
						std::for_each(fl.begin(), fl.end(), [&duration_l, &tstart_l](Filterbank& bf) { duration_l.push_back( (float)bf.duration ); tstart_l.push_back( (float) bf.tstart );  });
						maxdur = *(std::max_element(duration_l.begin(), duration_l.end()));
						mintstart = *(std::min_element(tstart_l.begin(), tstart_l.end()));
						ddit = fbw.Initialize(filename, fl[0], maxdur, mintstart);
						// until writing to filterbank is done in steps
						_serial(fl, fbw);
				}
};
#endif
