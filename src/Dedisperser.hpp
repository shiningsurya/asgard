#include "asgard.hpp"
#ifndef DEDISPERSER_H
#define DEDISPERSER_H
#include "dedisp.h"
// Handles de-dispersion as a class
// replaces operations::dedisperse
// which calls DEDISP all the time
#include "Operations.hpp"
#include <limits>

class DedispManager {
		protected:
				dedisp_plan  dplan;
				dedisp_error error;
				float dm; 
				double tsamp;
				int nchans;
				timeslice max_delay, dm_count;
				double f0, df;
				FloatVector freqs;
		public:
				DedispManager() : dplan(nullptr) {}
				DedispManager (int nch, double ts, double fch1, double foff) :
				nchans(nch), tsamp(ts), f0(fch1), df(foff) {
						error = dedisp_create_plan(&dplan, nchans, tsamp , f0, df);
						if( error != DEDISP_NO_ERROR ) std::cerr << "DedispManager::ctor Could not create dedispersion plan: " <<  dedisp_get_error_string(error) << std::endl;
						freqs = operations::FreqTable (f0, df, nchans);
				}
				void CreatePlan(int nch, double ts, double fch1, double foff) {
						error = dedisp_create_plan(&dplan, nch, ts, fch1, foff);
						if( error != DEDISP_NO_ERROR ) std::cerr << "\nDedispManager: Could not create dedispersion plan: " <<  dedisp_get_error_string(error) << std::endl;
						tsamp = ts;
						f0 = fch1;
						df = foff;
						nchans = nch;
				}
				~DedispManager() {
						dedisp_destroy_plan(dplan);
				}
				void MallocOut (PtrFloat& out, timeslice nsamps) {
				  nsamps -= max_delay;
				  assert (nsamps > 0);
				  std::size_t memsize =  nsamps * dm_count;
				  out = new float [ memsize ];
				}
				timeslice DMList (FloatVector& dmlist) {
				  auto dmlist_ptr = dedisp_get_dm_list (dplan);
				  dm_count = dedisp_get_dm_count (dplan);
				  dmlist.clear(); dmlist.reserve (dm_count);
				  std::copy ( dmlist_ptr, dmlist_ptr + dm_count, 
				    std::back_inserter ( dmlist )
				    );
				  return dm_count;
				}
				timeslice ClosestDM (float dm) {
				  auto dmlist = dedisp_get_dm_list (dplan);
				  timeslice count = dedisp_get_dm_count (dplan);
				  timeslice ret = 0;
				  float close = std::numeric_limits<float>::max(), temp;
				  //
				  for (timeslice i = 0; i < count; i++) {
				    temp = fabs ( dm - dmlist[i] );
				    if (close > temp) {
				      ret = i;
				      close  = temp;
				    }
				  }
				  return ret;
				}
				timeslice SetDM (float d0, float d1, unsigned int filw) {
				  //std::cout << "d0=" << d0 << std::endl;
				  //std::cout << "d1=" << d1 << std::endl;
				  float dd0 = d0; float dd1 = d1;
						error = dedisp_generate_dm_list(dplan, dd0, dd1, filw*tsamp*1e6f, 1.25f);
						if( error != DEDISP_NO_ERROR ) std::cerr << "\nDedispManager::SetDM Could not create dmlist: " <<  dedisp_get_error_string(error) << std::endl;
						max_delay  = (timeslice) dedisp_get_max_delay(dplan);
						return max_delay;
				}
				void CoherentDD(PtrFloat& in, timeslice nsamps, PtrFloat& ret) {
						error = dedisp_execute(dplan, (dedisp_size)nsamps, (const dedisp_byte*)in, sizeof(float)*8, (dedisp_byte*)ret, 8*sizeof(float), DEDISP_USE_DEFAULT);
						/// Dedisp 
						if( error != DEDISP_NO_ERROR ) std::cerr << "DedispManager::CoherentDD Could not execute dedispersion plan: " <<  dedisp_get_error_string(error) << std::endl;
				}
				void InCoherentDD(PtrFloat input, const std::vector<timeslice>& idlays, timeslice nsamps, PtrFloat output){
						int ridx;
						for(timeslice i = 0; i < nsamps; i++) {
								for(int j = 0; j < nchans; j++) {
										ridx = nchans*(i + idlays[j]) + j;
										/*
										 *if(i > 2*idlays[i]) std::cerr << "DedispManager::InCoherentDD | Fatal Error" << std::endl;
										 *else 
										 */
										/*
										 *if(i < idlays[j]) std::cout << "left index: " << lidx << " index: " << idx << std::endl;
										 *else std::cout << "right index: " << ridx << " index: " << idx << std::endl;
										 */
										output[nchans*i + j] = input[ridx]; 
										/*
										 *if(i < idlays[j]) output[nsamps*i + j] = input[f.nchans*(nsamps - idlays[j] + 1)];
										 *else output[nsamps*i + j] = input[f.nchans*(i - idlays[j])]; 
										 */
								}
						}
				}
				void InCoherentDD(PtrFloat input, const float dm, timeslice nsamps, PtrFloat output){
				  auto idelays = operations::Delays (freqs, dm, tsamp);
				  InCoherentDD (input, idelays, nsamps, output);
				}
};
#endif
