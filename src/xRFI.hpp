#include "asgard.hpp"
#include "Operations.hpp"
#ifndef XRFI_H
#define XRFI_H
#define MAD_RMS_FAC 1.0f
#define HIST_RMS_FAC 1.0f
namespace excision {
		struct xestimate {
				float CentralTendency;
				float rms;
				float rmsfac;
				bool isMad, isHist;
		};
		struct xestimate  MAD(float * in, timeslice length, char * flag) {
				// Median Absolute Deviation
				float median, rms;
				int hlen= length / 2;
				// initializing
				float * tin = new float[length];
				std::copy(in, in + length, tin);
				std::sort(tin, tin + length);
				// median
				if(length %2 == 0) 
						median = 0.5f * (tin[hlen - 1] + tin[hlen] );
				else
						median = tin[hlen];
				// deviation
				for(timeslice i = 0; i < length; i++) 
						tin[i] = fabs(tin[i] - median);
				// rms
				std::sort(tin, tin + length);
				if(length %2 == 0) 
						rms = 0.5f * (tin[hlen - 1] + tin[hlen] );
				else
						rms = tin[hlen];
				rms *= 1.4826;
				// flagging
				float cutoff = rms * MAD_RMS_FAC;
				for(timeslice i = 0; i < length; i++)
						if( fabs(in[i] - median) > cutoff )
								flag[i] = 'm';
						else
								flag[i] = 'u';
				struct xestimate ret;
				ret.CentralTendency = median;
				ret.rms = rms;
				ret.rmsfac = MAD_RMS_FAC;
				ret.isMad = true;
				ret.isHist = false;
				return ret;
		}
		struct xestimate  Histogram(float * in, timeslice length, char * flag, FloatVector& bins) {
				// Histogram
				float mode, hmode, rms;
				timeslice modepoint, fwhm_1, fwhm_2;
				int hlen= length / 2;
				// initializing
				float * histogram = new float [bins.size()];
				histogram = {};
				float bmin, bstep;
				bmin = bins.front();
				bstep = ( bins.back() - bins.front() ) / bins.size();
				// histogram computing
				int hid = 0;
				for(timeslice i = 0; i < length; i++) {
						hid = (in[i] - bmin) / bstep;
						histogram[hid]++;
				}
				// find mode
				modepoint = 0;
				modepoint = *std::max_element(in, in + length);	
				mode = histogram[modepoint];
				hmode = 0.5 * mode;
				// fwhm
				fwhm_1 = modepoint;
				while(histogram[fwhm_1] >= hmode && fwhm_1) fwhm_1--;
				fwhm_2 = modepoint;
				while(histogram[fwhm_2] >= hmode && fwhm_2 < length) fwhm_2++;
				// rms
				rms = (fwhm_2 - fwhm_1) * bstep / 2.355;
				// flagging
				float cutoff = rms * HIST_RMS_FAC;
				for(timeslice i = 0; i < length; i++)
						if( fabs(in[i] - mode) > cutoff )
								flag[i] = 'h';
						else
								flag[i] = 'n';
				struct xestimate ret;
				ret.CentralTendency = mode;
				ret.rms = rms;
				ret.rmsfac = HIST_RMS_FAC;
				ret.isMad = false;
				ret.isHist = true;
				return ret;
		}
}


/*
 *class xRFI {
 *        private:
 *                double timestep;
 *                timeslice i0, i1, wid;
 *                FloatVector bins = {0.0f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f};
 *                bool plot;
 *                int method;
 *                float cutoff, MAD_rms2cutoff, HIS_rms2cutoff;
 *        public:
 *                xRFI(int m, bool p, float madc, float hisc) : 
 *                        method(m),
 *                        plot(p),
 *                        MAD_rms2cutoff(madc),
 *                        HIS_rms2cutoff(hisc) {
 *
 *                }
 *                Work(Filterbank& f, int method) {
 *                        // variables
 *                        double duration = f.duration;
 *                        int nsteps;
 *                        int nchans = f.nchans;
 *                        if(timestep == 0.0f) {
 *                                timestep = duration;
 *                                nsteps = 1;
 *                        }
 *                        else {
 *                                nsteps = duration/timestep + 1;
 *                        }
 *                        wid = timestep / f.tsamp;
 *                        i0 = 0; // one time initialization
 *                        // RAII
 *                        // Resource acquisition is initialization
 *                        PtrFloatUnique dat = std::make_unique<float>(wid*nchans);
 *                        PtrFloatUnique bandshape = std::make_unique<float>(nchans);
 *                        PtrFloatUnique timeshape = std::make_unique<float>(wid);
 *                        PtrCharUnique bandflags = std::make_unique<char>(nchans);
 *                        PtrCharUnique timeflags = std::make_unique<char>(wid);
 *                        // plot initialization 
 *                        if(plot) {
 *                                xRFIPlot xp(nsteps, f, wid, nchans);
 *                        }
 *                        // work loop
 *                        for(int i = 0; i < nsteps; i++, i0+=wid) {
 *                                // work part
 *                                f.Unpack(dat, i0, wid);
 *                                operation::FreqShape(dat, wid, nchans, bandshape);
 *                                operations::TimeShape(dat, wid, nchans, timeshape);
 *                                // xrfi part
 *                                if(method == 1) {
 *                                        excision::MAD(bandshape, nchans, bandflags);
 *                                        excision::MAD(timeshape, wid, timeflags);
 *                                }
 *                                else(method == 2) {
 *                                        excision::Histogram(bandshape, nchans, bandflags);
 *                                        excision::Histogram(timeshape, wid, timeflags);
 *                                }
 *                                // plot part
 *                                if(plot) {
 *                                        xp.Plot(dat, bandshape, timeshape, bandflags, timeflags);
 *                                }
 *                        }
 *                }
 *};
 */
#endif
