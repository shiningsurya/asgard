#pragma once
#include "asgard.hpp"

template<typename T>
class Incoherent {
	using vt = std::vector<T>;
	private:
	static inline float _single_dm_delay (float dm, float f0, float foff, unsigned nchans) {
		float f1 = f0 - (nchans*foff);
		return 4148.741601 * (1.0/f0/f0 - 1.0/f1/f1) * dm;
	}
	static inline float _single_dm_delay (float dm, float f0, float f1) {
		return 4148.741601 * (1.0/f0/f0 - 1.0/f1/f1) * dm;
	}
	// parameters
	double           tsamp;
	float            freq_ch1; 
	float            freq_off;
	unsigned         nchans;
	public:
	// search parameters
	std::vector<float> dm_list;
	// single dm
	float                 single_dm;
	std::vector<unsigned> idelays;

	unsigned MaxSampDelay () const { return idelays.back(); }
	float MaxTimeDelay () const { return idelays.back()*tsamp;}

	Incoherent (
			const double   _tsamp, 
			const unsigned _nchans, 
			const float    _fch1, 
			const float    _foff
			) {
		// copy parameters
		tsamp    = _tsamp;
		nchans   = _nchans;
		freq_ch1 = _fch1;
		freq_off = _foff;
	}
	~Incoherent () = default;

	void Execute (
			const vt& f, unsigned nsamps,
			vt& o
			) {
		unsigned idx = 0;
		// This is for single DM case ONLY!
		for (unsigned i = 0; i < nsamps; i++) {
			for (unsigned j = 0; j < nchans; j++) {
				idx = nchans * (i + idelays[j]) + j;
				o.push_back (f[idx]);
			}
		}
	}

	void SetDM (float_t _dm) {
		// clear all other delays
		idelays.clear ();
		single_dm = _dm;
		float if02 = 1.0f/freq_ch1/freq_ch1;
		float dd = 0.0f;
		float f0 = freq_ch1;
		float f1 = 0.0f;
		float df = freq_off;
		if (df > 0) df *= -1.0f;
		//
		for (unsigned i = 0; i < nchans; i++) {
			f1 = f0 + (i * df);
			dd = 4148.741601 * ((1.0f/f1/f1) - if02) * single_dm; 
			idelays.push_back (dd / tsamp);
		}
	} 

};

