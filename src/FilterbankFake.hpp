#pragma once
#include <random>

class FilterbankFake {
	using vf   = std::vector<float>;
	using vi   = std::vector<timeslice>;
	using Byte = unsigned char;
	using vb   = std::vector<Byte>;
private:
	// random stuff
	std::random_device rd;
	std::mt19937 mt;
	// simulation parameters
	float              tsamp;
	unsigned int       nchans;
	float              fch1;
	float              foff;
	// rmss
	Byte               rms;
	// arrays
	vf                 freqs;
	vi                 delays;
	// to calculate delays
	void set_delays (float dm) {
		float if02  =  1.0 / fch1 / fch1; 
		float fac   =  dm * 4148.741601 / tsamp;
		// work loop
		delays[0]   = 0;
		for (unsigned int i = 1; i < nchans; i++) {
			float fi02 = 1.0 / freqs[i] / freqs[i];
			delays[i]   = static_cast<timeslice> (fac * (fi02 - if02)); 
		}
	}
public:
	FilterbankFake (
			float tsamp_     = 781.25E-6,
			float nchans_    = 4096,
			float fch1_      = 361.941448,
			float foff_      = -0.01023,
			Byte  rms_       = 4
			) :
		mt (rd()),
		tsamp (tsamp_),
		nchans (nchans_), 
		fch1 (fch1_),
		foff (foff_),
		rms (rms_),
		freqs (nchans, 0.0f), delays (nchans, 0.0f)
	{
		// create frequency array
		float fi = fch1;
		for (unsigned i = 0; i < nchans; i++) {
			freqs[i] = fi;
			fi += foff;
		}
	}

	vb Signal (float sn, float dm, float wd, float it = 0.2, float jt = 0.1) {
		// amplitudes
		Byte sigamp     = sn  * (1 - (rms/100));
		Byte noiseamp  = 128 * (1 - (rms/100));
		set_delays (dm);
		// indices
		timeslice iit   = it / tsamp;
		timeslice wit   = wd / tsamp;
		timeslice ijt   = jt / tsamp;
		timeslice maxd  = delays[nchans-1];
		timeslice tsize = iit + wit + maxd + ijt;
		timeslice bsize = tsize * nchans;
		// noise floor
		std::normal_distribution<float> norm (
      static_cast<float>(noiseamp), 
      static_cast<float>(rms)
		);
		vb  fb (bsize, 0);
		for (timeslice i = 0; i < bsize; i++) {
		  auto x = norm (mt);
		  if (x < 0) fb[i] = 0;
		  else if (x >= 255) fb[i] = 255;
		  else fb[i]  = static_cast<Byte>(x);
		}
		// add signal
		for (unsigned int ichan = 0; ichan < nchans; ichan++) {
			timeslice iwt = delays[ichan] + iit;
			timeslice jwt = iwt + wit;
			for (timeslice ii = iwt; ii < jwt; ii++) {
				auto x = fb [ichan + nchans*ii];
				fb [ichan + nchans*ii] = static_cast<Byte>(std::min (255, static_cast<int>(x+sigamp)));
			}
		}
		// return
		return fb;
	}
	vb WhiteNoise (float it = 0.2, Byte irms = 0) {
		// amplitudes
		if (irms == 0) irms = rms;
		Byte noiseamp  = 128 * (1 - (irms/100));
		// indices
		timeslice tsize = it / tsamp;
		timeslice bsize = tsize * nchans;
		// noise floor
		std::normal_distribution<float> norm (
      static_cast<float>(noiseamp), 
      static_cast<float>(irms)
		);
		vb  fb (bsize, 0);
		for (timeslice i = 0; i < bsize; i++) {
		  auto x = norm (mt);
		  if (x < 0) fb[i] = 0;
		  else if (x >= 255) fb[i] = 255;
		  else fb[i]  = static_cast<Byte>(x);
		}
		// return
		return fb;
	}
};
