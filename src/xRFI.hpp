#pragma once
#include "asgard.hpp"
#include "Operations.hpp"
#ifndef XRFI_H
#define XRFI_H
#include <random>
namespace excision {
  enum class Method { MAD, Histogram };
  enum class Filter { No, Zero, Noise };
  std::istream& operator>>(std::istream& in, Method& mm) {
    std::string tok;
    in >> tok;
    if(tok == "1")
      mm = Method::Histogram;
    else
      mm = Method::MAD;
    return in;
  }
  std::istream& operator>>(std::istream& in, Filter& ff) {
    std::string tok;
    in >> tok;
    if(tok == "0")
      ff = Filter::No;
    else if(tok == "1")
      ff = Filter::Zero;
    else 
      ff = Filter::Noise;
    return in;
  }
  std::ostream& operator<<(std::ostream& in, const Method& mm) {
    if(mm == Method::MAD)
      in << " Median Absolute Deviation";
    else if(mm == Method::Histogram)
      in << " Histogram";
    return in;
  }
  std::ostream& operator<<(std::ostream& in, const Filter& ff) {
    if(ff == Filter::No)
      in << " No Filtering";
    else if(ff == Filter::Zero)
      in << " Zero-ing";
    else
      in << " WhiteNoise";
    return in;
  }
  struct excisionParams {
    Method method;
    Filter filter;
    float tfac;
    float ffac;
  };
		struct xestimate {
				float CentralTendency;
				float rms;
				float rmsfac;
				bool isMad, isHist;
		};
		struct xestimate  MAD(PtrFloat in, timeslice length, PtrByte flag, float fac) {
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
				float cutoff = rms * fac;
				for(timeslice i = 0; i < length; i++)
						if( fabs(in[i] - median) > cutoff )
								flag[i] = 'o';
						else
								flag[i] = 'c';
				struct xestimate ret;
				ret.CentralTendency = median;
				ret.rms = rms;
				ret.rmsfac = fac;
				ret.isMad = true;
				ret.isHist = false;
				return ret;
		}
		struct xestimate  Histogram(PtrFloat in, timeslice length, PtrByte flag, const FloatVector& bins, float fac) {
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
				float cutoff = rms * fac;
				for(timeslice i = 0; i < length; i++)
						if( fabs(in[i] - mode) > cutoff )
								flag[i] = 'o';
						else
								flag[i] = 'c';
				struct xestimate ret;
				ret.CentralTendency = mode;
				ret.rms = rms;
				ret.rmsfac = fac;
				ret.isMad = false;
				ret.isHist = true;
				return ret;
		}
		class xRFI {
		  using eParams = struct excisionParams;
				private:
						// class parameters
						const Method method;
						const FloatVector bins = {0.0f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f};
				public:
						// random stuff
						std::random_device rd;
						std::mt19937 mt;
						// input sizes
						const timeslice wid;
						const int nchans;
						// exposing flags, shapes
						PtrByte bandflags, timeflags;
						PtrFloat bandshape, timeshape;
						// factors
						const float time_factor, freq_factor;
						// estimates
						struct xestimate estt, estf;
						// ctor
						xRFI(Method _method, float _timef, float _freqf, timeslice _wid, int _nchans)
								: method(_method), wid(_wid), nchans(_nchans),
								time_factor(_timef), freq_factor(_freqf),
								mt(rd()) {
										// flags
										bandflags = new unsigned char[nchans];
										timeflags = new unsigned char[wid];
										// shapes
										bandshape = new float[nchans]();
										timeshape = new float[wid]();
								}
						xRFI(const eParams& _xp, timeslice _wid, int _nchans)
								: method(_xp.method), wid(_wid), nchans(_nchans),
								time_factor(_xp.tfac), freq_factor(_xp.ffac),
								mt(rd()) {
										// flags
										bandflags = new unsigned char[nchans];
										timeflags = new unsigned char[wid];
										// shapes
										bandshape = new float[nchans]();
										timeshape = new float[wid]();
								}
						~xRFI() {
								// clear out heap-memory
								if(bandflags != nullptr) delete[] bandflags;
								if(timeflags != nullptr) delete[] timeflags;
								if(timeshape != nullptr) delete[] timeshape;
								if(bandshape != nullptr) delete[] bandshape;
						}
						void Excise(PtrFloat dat, Filter _filter = Filter::No) {
								// no need to reset any arrays bc every element is met
								// optimized for streams
								operations::TimeFreqShape(dat, wid, nchans, timeshape, bandshape);
								// xrfi part
								if(method == Method::MAD) {
										// MAD
										estf = excision::MAD(bandshape, nchans, bandflags, freq_factor);
										estt = excision::MAD(timeshape, wid, timeflags, time_factor);
								}
								else if(method == Method::Histogram) {
										// Histogram
										estf = excision::Histogram(bandshape, nchans, bandflags, bins, freq_factor);
										estt = excision::Histogram(timeshape, wid, timeflags, bins, time_factor);
								}
								// removing
								if(_filter == Filter::Zero)
										FilterZero(dat);
								else if(_filter == Filter::Noise) 
										FilterWhite(dat);
						} 
						void FilterZero(PtrFloat dat) {
								timeslice idx;
#ifdef AGOMP
#pragma omp parallel for collapse(2) private(idx)
#endif
								for(timeslice iwid = 0; iwid < wid; iwid++) {
										for(int ichan = 0; ichan < nchans; ichan++) {
												// AND or OR here .........vv
												if(bandflags[ichan] == 'o' || timeflags[iwid] == 'o') {
														idx = ( iwid * nchans ) + ichan;
														//TODO zero out? replace by mean?
														dat[idx] = 0.0f;
												}
										}
								}
						}
						void FilterWhite(PtrFloat dat) {
								std::normal_distribution<float> normt(estt.CentralTendency, estt.rms);
								std::normal_distribution<float> normf(estf.CentralTendency, estf.rms);
								timeslice idx;
#ifdef AGOMP
#pragma omp parallel for collapse(2) private(idx)
#endif
								for(timeslice iwid = 0; iwid < wid; iwid++) {
          for(int ichan = 0; ichan < nchans; ichan++) {
            idx = ( iwid * nchans ) + ichan;
            // AND or OR here .........vv
            if(timeflags[iwid] == 'o') {
              dat[idx] = normt(mt);
            }
            if(bandflags[ichan] == 'o') {
              dat[idx] = normf(mt);
            }
          }
        }
      }
  };
} // namespace excision
#endif
