#ifndef ASGARD_H
#include "asgard.hpp"
#endif
#ifndef COADD_H
#define COADD_H
#include "Filterbank.hpp"

class Coadd {
		private:
				//std::string method("MAD");
				bool returnFil, returnDada;
				std::string filename;
				int numRuns;
				double load_secs;
				PtrFloat ret;
				timeslice retsize;
				FilterbankWriter fbw;
				void _work_2(Filterbank& x, Filterbank& y, PtrFloatUnique  ret ) {
						// ret is initialized before
				}
				void _serial(FilterbankList& fl, PtrFloat ret) {
						// serial code
						timeslice tstep = load_secs / fl[0].tsamp;
						timeslice i0 = 0, nsteps;
						int NumAnt = fl.size();
						PtrFloat tdat = new float[tstep * fl[0].nchans];
						for(Filterbank& f :fl) {
								nsteps = f.totalsamp / tstep;
								while(nsteps--) {
										f.Unpack(tdat, i0, tstep);
										for(timeslice i = 0; i < tstep*f.nchans; i++) ret[i] += tdat[i] / NumAnt;
										i0 += tstep;
								}
						}
						delete[] tdat;
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
				void Work(FilterbankList& fl) {
						// compute max duration here
						FloatVector duration_list;
						std::for_each(fl.begin(), fl.end(), [&duration_list](Filterbank& bf) { duration_list.push_back( (float)bf.duration ); } );
						float maxdur = *(std::max_element(duration_list.begin(), duration_list.end()));
						retsize = maxdur / fl[0].tsamp;	// hoping every tsamp is the same	
						ret = new float[retsize * fl[0].nchans]();
						// until writing to filterbank is done in steps
						_serial(fl, ret);
						//
						fbw.Write(filename, fl[0], ret);
						delete[] ret;
				}
};
#endif
