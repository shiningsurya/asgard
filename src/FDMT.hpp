#pragma once
#include <vector>
#include <limits>

// Adapted from trishul/Dedispersion/FDMT_CPU
// alpha (right-most-dim), beta (second-right-most)
// Reference ordering   = (F,dT,T)
// Cache aware ordering = (F,dT,T)
// deltaT --> dimension is deltaT+2 because sometimes we go 
// to deltaT+1 (inclusive) in the iteration steps
// Usually towards the end of iterations
// Written by me

using vf = std::vector<float>;
using vb = std::vector<unsigned char>;

// input output should be byte arrays
// internally all should be floats

#ifdef TIMING
#include "Timer.hpp"
#endif

#define restrict __restrict

template<typename T>
class FDMT {
	private:
		// Main state	
		timeslice nsamps;
		std::vector<float> State;
		float * restrict  pState;
		float * restrict  qState;
		// strides for state
		timeslice      salpha;
		timeslice      sbeta;
		// DM ranges
		timeslice      aDT;
		timeslice      bDT;
		timeslice      maxDT;
		// Freq
		float         fmin;
		float         fmax;
		unsigned      l2nch;
		//
		// parameters
		double           tsamp;
		float            freq_ch1; 
		float            freq_off;
		unsigned         nchans;
		static inline float cff (float f0, float f1, float f2) {
			return (std::pow(f0,-2) - std::pow(f1,-2))/(std::pow(f0,-2) - std::pow(f2,-2));
		}
		static inline float dff (float f0, float f1) {
			return std::pow (f0,-2) - std::pow (f1,-2);
		}
		static inline float _single_dm_delay (float dm, float f0, float foff, unsigned nchans) {
			float f1 = f0 - (( nchans-1 )*foff);
			return 4148.741601 * (1.0/f0/f0 - 1.0/f1/f1) * dm;
		}
		static inline float _single_dm_delay (float dm, float f0, float f1) {
			return 4148.741601 * (1.0/f0/f0 - 1.0/f1/f1) * dm;
		}
	public:

		// search parameters
		std::vector<float> dm_list;
		std::vector<timeslice> idelays;

		FDMT (
				const double_t   _tsamp,
				const unsigned _nchans, 
				const float    _freq_ch1,
				const float    _freq_off
				) {
			// copy parameters
			tsamp    = _tsamp;
			nchans   = _nchans;
			l2nch    = log2 (nchans);
			freq_ch1 = _freq_ch1;
			fmax     = _freq_ch1;
			freq_off = _freq_off;
			fmin     = fmax - (( nchans-1 )*freq_off);
		}
		~FDMT () {
			std::cout << "FDMT::dtor called." << std::endl;
			std::cout << "FDMT::dtor State.size=" << State.size () << std::endl;
		}

		// please don't call this before initialization
		// i beg of you
		timeslice MaxSampDelay () noexcept {
			return maxDT;
		}

		void SetDM (const float a, const float b, const unsigned n) noexcept { 
			// Setting DM clears lists
			dm_list.clear ();
			idelays.clear ();
			// dms
			float start = a;
			// that n-1 necessary to have [a,b]
			float step = (b - a) / (n-1);
			// 
			for (unsigned i = 0; i < n; i++,start+=step) {
				dm_list.push_back (start);
				idelays.push_back ( _single_dm_delay (start, fmin, fmax) /tsamp);
			}
			aDT   =  idelays.front ();
			bDT   =  idelays.back  ();
		}


		void Iteration (const unsigned& i) {
			//std::cout << "FDMT::Iteration begin="<< i << std::endl;
			float deltaF    = std::pow (2, i) * freq_off;
			unsigned fjumps = nchans / std::pow (2,i);
			float    corr   = i > 0 ? 0.5 * freq_off : 0.0f;

			timeslice deltaT = std::ceil ( 
					(maxDT-1)                 *
					dff (fmin+deltaF, fmin)   /
					dff (fmax, fmin)
					);

			std::vector<float> Output (fjumps*nsamps*(deltaT+2));
			timeslice alpha = nsamps;
			timeslice beta  = alpha*( deltaT+2 );

			//float * pOut = Output.data ();
			//pState = State.data ();
			//qState = State.data ();

			for (unsigned iF = 0; iF < fjumps; iF++) {
				float fstart = fmax - ( deltaF*iF );
				float fend   = fmax - ( deltaF*( iF+1 ) );
				float fmidl  = 0.5*(fend + fstart) - corr;
				float fmid   = 0.5*(fend + fstart) + corr;

				timeslice l_dT = std::ceil (
						(maxDT-1)         *
						dff(fstart,fend)  /
						dff(fmax,  fmin)
						);
				for (timeslice idt = 0; idt <= l_dT; idt++) {
					timeslice midt = std::round ( idt * dff (fstart, fmid)  / dff (fstart, fend));
					timeslice midtl= std::round ( idt * dff (fstart, fmidl) / dff (fstart, fend));
					timeslice bp   = nsamps - midtl;
					// 
					//pState = State.data () + (2*iF*sbeta) + (midt*nsamps);
					//qState = State.data () + (2*iF*sbeta) + sbeta + ((idt-midtl)*nsamps);
					//qState += midtl;
					for (timeslice itt = 0; itt <bp; itt++) {
						//Output.at (iF,idt, itt) = State.at (2*iF   , midt, itt) + State.at (2*iF +1, idt-midtl, itt+midtl);
						Output [itt       +alpha*idt                +beta*iF]              =
						State  [itt       +salpha*midt              +sbeta*2*iF]           +
						State  [itt+midtl +salpha*idt -salpha*midtl +sbeta*2*iF +sbeta]    ;
						// --
						//*pOut = *pState + *qState; 
						//pOut   ++;
						//pState ++;
						//qState ++;
					}
					// 
					for (timeslice itt = bp; itt <nsamps; itt++)  {
						//Output.at (iF, idt, itt) = State.at (2*iF, midt, itt);
						Output [itt	 +alpha*idt	 + beta*iF]		=
						State	[itt	 +salpha*midt + sbeta*2*iF] ;
						// --
						//*pOut = *pState;
						//pOut   ++;
						//pState ++;
					}
				}
			}

			State = std::move(Output);
			salpha = alpha;
			sbeta  = beta;
			//std::cout << "FDMT::Iteration end="<< i << std::endl;
		}

		void Initialization (const std::vector<T>& Image) {
			maxDT = std::min(bDT+1, static_cast<timeslice>(nsamps));
			timeslice deltaT = std::ceil ( 
					(maxDT-1)                 *
					dff (fmin, fmin+freq_off) /
					dff (fmin, fmax)
					);

#ifdef TIMING
			Timer vv ("FDMT::Initialization::Vector_resize");
			vv.Start ();
#endif
			std::vector<float> Output(Image.size()* (deltaT+2), 0.0f);
			timeslice alpha = nsamps;
			timeslice beta  = alpha*(deltaT+2);

#ifdef TIMING
			vv.StopPrint (std::cout);
			Timer ii ("FDMT::Initialization::Transpose");
			ii.Start ();
#endif
			// This is a transpose operation
			pState = Output.data ();
			for (unsigned ichan = 0; ichan < nchans; ichan++) {
				const T * pImage = Image.data () + ichan; 
				pState = Output.data () + (ichan*beta);
				for (unsigned isamp = 0; isamp < nsamps; isamp++) {
					// Output.at (ichan, 0, isamp) = Image.at (ichan, isamp)
					//State [isamp + sbeta*ichan] = Image [ichan + nchans*isamp];
					*pState = *pImage;
					pImage += nchans;
					pState ++;
				}
			}

#ifdef TIMING
			ii.StopPrint (std::cout);
#endif

#ifdef TIMING
			Timer jj ("FDMT::Initialization::Linear");
			jj.Start ();
#endif
			qState = Output.data ();
			pState = Output.data () + nsamps;
			timeslice isize = Image.size();
			for (unsigned ichan = 0; ichan < nchans; ichan++) {
				for (timeslice idt = 1; idt <= deltaT; idt++)    {
					const T * pImage = Image.data () +isize - ichan - (idt*nchans); 
					pState += idt;
					qState += idt;
					for (unsigned isamp = idt; isamp < nsamps; isamp++) {
						//Output.at (isamp, ichan, idt) = Output.at (isamp, ichan, idt-1) + Image.at	(nsamps-isamp+1, ichan);
							//State [isamp +salpha*idt		 +sbeta*ichan]		=
							//State [isamp +salpha*(idt-1) +sbeta*ichan]		+
							//Image	[ichan +nchans*(nsamps-isamp)];
							//
							*pState = *qState + *pImage;
							pImage -= nchans;
							pState ++;
							qState ++; 
					}
				}
			}

#ifdef TIMING
			jj.StopPrint (std::cout);
			Timer mm ("FDMT::Initialization::Move");;
			mm.Start ();
#endif

			State  = std::move (Output);
			salpha = alpha;
			sbeta  = beta;
#ifdef TIMING
			mm.StopPrint (std::cout);
#endif
		}

		void Execute (
				const std::vector<T>& in,
				timeslice _nsamps,
				std::vector<float>& out) {
			nsamps = _nsamps;

#ifdef TIMING
			Timer ini("Initialization");
			Timer step("Iteration");
			Timer slicer ("Slicing");
#endif

			// FDMT start
#ifdef TIMING
			ini.Start ();
#endif
			Initialization (in);
#ifdef TIMING
			ini.StopPrint (std::cout);
#endif
			for (unsigned i = 1; i <= l2nch; i++) {
#ifdef TIMING
			step.Start ();
#endif
				Iteration (i);
#ifdef TIMING
			step.StopPrint (std::cout);
#endif
			}
			// FDMT end

			// slicing State(T,1,aDT:bDT) = State(T,aDT:bDT)
			// out is (dT,T)
#if 1 
			// since internal state is float
			// output is float
			// XXX I err'd by coding before slicing
			// which caused us to not use the entire dyn spec
#ifdef TIMING
			slicer.Start ();
#endif
			auto ddnsamps = nsamps-maxDT;
			out.resize (idelays.size() * ddnsamps);
			auto pOut = out.data ();
			pState = State.data ();
			for (timeslice idt = 0  ; idt<idelays.size(); idt++) {
				pState = State.data () + (idelays[idt]*nsamps);
				for (timeslice isamp = 0; isamp <ddnsamps	 ; isamp++) {
					//out [isamp +ddnsamps*idt] =	State [isamp +salpha*idelays[idt]];
					*pOut = *pState;
					pOut   ++;
					pState ++;
				}
			}
#ifdef TIMING
			slicer.StopPrint (std::cout);
#endif
			//
#else
			out = State;
#endif
		}

};
