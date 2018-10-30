
// Handles de-dispersion as a class
// replaces operations::dedisperse
// which calls DEDISP all the time

class DedispManager {
		private:
				dedisp_plan  dplan;
				dedisp_error er;
				float dm; 
				double tsamp;
				bool setdm;
				int nchans;
				timeslice maxd;
				double f1, df;
		public:
				DedispManager(int nch, double ts, double fch1, double foff) {
						setdm = false;
						error = dedisp_create_plan(&dplan, nch, ts, fch1, foff);
						if( error != DEDISP_NO_ERROR ) std::cerr << "\nDedispManager: Could not create dedispersion plan: " <<  dedisp_get_error_string(error) << std::endl;
						tsamp = ts;
						f1 = fch1;
						df = foff;
				}
				~DedispManager() {
						dedisp_destroy_plan(dplan);
				}
				timeslice SetDM(float d, int filw) {
						dm = d;
						error = dedisp_generate_dm_list(dplan, d, d, filw*tsamp*1e6f, 1.0f);
						if( error != DEDISP_NO_ERROR ) std::cerr << "\nDedispManager: Could not create dmlist: " <<  dedisp_get_error_string(error) << std::endl;
						setdm = true;
						maxd  = (timeslice) dedisp_get_max_delay(dplan);
						return maxd;
				}
				FloatVector CoherentDD(FloatVector& in) {
						if(!setdm) {
								std::cerr << "DedispManager: DM not set and DD called\n";
								std::cerr << "Fatal Error\n";
						}
						FloatVector ret (...);
						timeslice nsamps = in.size() / nchans;
						error = dedisp_execute(dplan, (dedisp_size)nsamps, (const dedisp_byte*)in.data(), sizeof(float)*8, (dedisp_byte*)ret.data(), 8*sizeof(float), DEDISP_USE_DEFAULT);
						/// Dedisp 
						if( error != DEDISP_NO_ERROR ) std::cerr << "\nDedispManager: Could not execute dedispersion plan: " <<  dedisp_get_error_string(error) << std::endl;
						// following should be second last
						setdm = false;
						return ret;
				}
				FloatVector InCoherentDD(FloatVector& in, std::vector<timeslice>& idlays){
						if(!setdm) {
								std::cerr << "DedispManager: DM not set and DD called\n";
								std::cerr << "Fatal Error\n";
						}
						FloatVector output(...);
						timeslice nsamps = in.size() / nchans;
						int lidx, ridx, idx;
						idx = nsamps*f.nchans;
						for(int i = 0; i < nsamps; i++) {
								for(int j = 0; j < nchans; j++) {
										lidx = nchans*((int)nsamps - (int)idlays[j] + i) + j;
										ridx = nchans*(i - (int)idlays[j]) + j;
										/*
										 *if(i < idlays[j]) std::cout << "left index: " << lidx << " index: " << idx << std::endl;
										 *else std::cout << "right index: " << ridx << " index: " << idx << std::endl;
										 */
										if(i < idlays[j]) output[lidx] = input[nchans*i + j];	
										else output[ridx] = input[nchans*i + j]; 
										/*
										 *if(i < idlays[j]) output[nsamps*i + j] = input[f.nchans*(nsamps - idlays[j] + 1)];
										 *else output[nsamps*i + j] = input[f.nchans*(i - idlays[j])]; 
										 */
								}
						}
						return output; 
				}


};
