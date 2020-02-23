#pragma once
#include <vector>
#include <limits>

// Adapted from trishul/Dedispersion/FDMT_CPU
// Written by me

using vf = std::vector<float>;
using vb = std::vector<unsigned char>;

// input output should be byte arrays
// internally all should be floats

template<typename T>
class FDMT {
	private:
		// Main state	
		timeslice nsamps;
		std::vector<float> State;
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
		~FDMT () = default;

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
			std::cout << "maxdelay=" << bDT << std::endl;
		}


		void Iteration (const unsigned& i) {
			float deltaF    = std::pow (2, i) * freq_off;
			unsigned fjumps = nchans / std::pow (2,i);
			float    corr   = i > 0 ? 0.5 * freq_off : 0.0f;

			timeslice deltaT = std::ceil ( 
					(maxDT-1)                 *
					dff (fmin+deltaF, fmin)   /
					dff (fmax, fmin)
					);

			std::vector<float> Output (fjumps*nsamps*(deltaT+1));
			timeslice alpha = deltaT+1;
			timeslice beta  = alpha*fjumps;

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
					for (timeslice itt = bp; itt <nsamps; itt++) 
						//Output.at (iF, idt, itt) = State.at (2*iF, midt, itt);
						Output [idt  +alpha*iF    + beta*itt]   =
							State  [midt +salpha*iF*2 + sbeta*itt]  ;
					// 
					for (timeslice itt = 0; itt <bp; itt++)
						//Output.at (iF,idt, itt) = State.at (2*iF   , midt, itt) + State.at (2*iF +1, idt-midtl, itt+midtl);
					{
						Output [idt       +alpha*iF             +beta*itt]                =
							State  [midt      +2*salpha*iF          +sbeta*itt]               +
							State  [idt-midtl +2*salpha*iF  +salpha +sbeta*itt +sbeta*midtl]  ;
					}
				}
			}

			State  = std::move (Output);
			salpha = alpha;
			sbeta  = beta;
		}

		void Initialization (const std::vector<T>& Image) {
			maxDT = std::min(bDT+1, static_cast<timeslice>(nsamps));
			timeslice deltaT = std::ceil ( 
					(maxDT-1)                 *
					dff (fmin, fmin+freq_off) /
					dff (fmin, fmax)
					);

			State.clear ();
			std::vector<float> Output (Image.size() * (deltaT+2));
			timeslice alpha = deltaT+2;
			timeslice beta  = alpha*nchans;
			for (unsigned isamp = 0; isamp < nsamps; isamp++)
				for (unsigned ichan = 0; ichan < nchans; ichan++)
					// Output.at (isamp, ichan, 0) = Image.at (isamp, ichan)
					Output [alpha*ichan +beta*isamp] =
						Image  [ichan +nchans*isamp];

			for (timeslice idt = 1; idt <= deltaT; idt++) 
				for (unsigned isamp = idt; isamp < nsamps; isamp++)
					for (unsigned ichan = 0; ichan < nchans; ichan++)
						//Output.at (isamp, ichan, idt) = Output.at (isamp, ichan, idt-1) + Image.at  (nsamps-isamp+1, ichan);
						Output [idt   +alpha*ichan +beta*isamp] =
							Output [idt-1 +alpha*ichan +beta*isamp] +
							Image  [ichan +nchans*(nsamps-isamp)];

			State  = std::move (Output);
			salpha = alpha;
			sbeta  = beta;
		}

		void Execute (
				const std::vector<T>& in,
				timeslice _nsamps,
				std::vector<float>& out) {
			nsamps = _nsamps;

			// FDMT start
			Initialization (in);
			for (unsigned i = 1; i <= l2nch; i++) {
				Iteration (i);
			}
			// FDMT end

			// slicing State(T,1,aDT:bDT) = State(T,aDT:bDT)
			// out is (dT,T)
#if 1 
			// since internal state is float
			// output is float
			// XXX I err'd by coding before slicing
			// which caused us to not use the entire dyn spec
			auto ddnsamps = nsamps-maxDT;
			out.resize (idelays.size() * ddnsamps);
			for (timeslice idt = 0  ; idt<idelays.size(); idt++)
				for (timeslice isamp = 0; isamp <ddnsamps	 ; isamp++)
					out [isamp +ddnsamps*idt] =  State [idelays[idt] +sbeta*isamp];
			//
#else
			out = State;
#endif
		}

};
