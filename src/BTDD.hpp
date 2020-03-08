#pragma once
#include "Incoherent.hpp"
#include "FDMT.hpp"
#include <vector>
#include <limits>

//#define TIMING
//#ifdef TIMING
//#undef TIMING
//#endif

#ifdef TIMING
#include "Timer.hpp"
#endif

template<typename T>
class BTDD {
	using vc = std::vector<T>;
	using vf = std::vector<float>;
	private:
		// parameters
		double           tsamp;
		unsigned         nchans;
		float            fch1; 
		float            foff;
		float            dmwidth;
		float            dmcount;
		float            hdmwidth;
		// dd'ers
		Incoherent<T> first_incoh;
		Incoherent<T> sec_incoh;
		FDMT<T>       bter;
		timeslice     fmaxd;
		timeslice     bmaxd;
		timeslice     smaxd;
	public:
		timeslice     ddnsamps;
		timeslice     btnsamps;
		BTDD (
				const double   _tsamp,
				const unsigned _nchans, 
				const float    _freq_ch1,
				const float    _freq_off,
				float _dmwidth = 50,
				float _dmcount = 256
				) :
			tsamp(_tsamp), nchans (_nchans), 
			fch1 (_freq_ch1), foff (_freq_off),
			dmwidth (_dmwidth), dmcount (_dmcount),
			hdmwidth (0.5*dmwidth),
			first_incoh (tsamp/1E6, nchans, fch1, foff),
			sec_incoh (tsamp/1E6, nchans, fch1, foff),
			bter (tsamp/1E6, nchans, fch1, -foff) {
				sec_incoh.SetDM (hdmwidth);
				bter.SetDM (0, dmwidth, dmcount);
				fmaxd = 0;
				bmaxd = bter.MaxSampDelay ();
				smaxd = sec_incoh.MaxSampDelay ();
			}
	
		void SetDM (float dm) {
			first_incoh.SetDM (dm - hdmwidth);
			//
			fmaxd = first_incoh.MaxSampDelay ();
		}

		void Execute (const vc& in, timeslice nsamps, vf& bt, vc& dd) {
			vc fdd;
			timeslice fddnsamps = nsamps-fmaxd;
#ifdef TIMING
			Timer btdd("BTDD");
			Timer fin("FirstIncoh");
			Timer sin("SecondIncoh");
			Timer btme("FDMT");
			btdd.Start ();
			fin.Start ();
#endif
			first_incoh.Execute (in,nsamps, fdd);
#ifdef TIMING
			fin.StopPrint (std::cout);
			btme.Start ();
#endif
			bter.Execute (fdd, fddnsamps, bt);
#ifdef TIMING
			btme.StopPrint (std::cout);
			sin.Start ();
#endif
			sec_incoh.Execute (fdd, fddnsamps, dd);
#ifdef TIMING
			sin.StopPrint (std::cout);
			btdd.StopPrint (std::cout);
#endif
			//
			ddnsamps = fmaxd + smaxd;
			btnsamps = fmaxd + bmaxd;
		}



};
