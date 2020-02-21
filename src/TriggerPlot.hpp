#pragma once
#include "asgard.hpp"
#include "cpgplot.h"

class TriggerPlot  {
	using vf = std::vector<float>;
	private:
		// for triggerplot
    fs::path dir;
    char filename[256];
    char group[256];
    vf   axtm;
    vf   axdm;
    vf   tsn;
    vf   dsn;
		// for pgplot
		static constexpr float contrast = 1.0f; 
		static constexpr float brightness = 0.5f;
		float tr[6];
		float charh;
		char txt[32];
	public:
		static constexpr std::array<float,5> heat_l = {0.0, 0.2, 0.4, 0.6, 1.0};
		static constexpr std::array<float,5> heat_r = {0.0, 0.5, 1.0, 1.0, 1.0};
		static constexpr std::array<float,5> heat_g = {0.0, 0.0, 0.5, 1.0, 1.0};
		static constexpr std::array<float,5> heat_b = {0.0, 0.0, 0.0, 0.3, 1.0};
		static constexpr std::array<float,9> rain_l = {-0.5, 0.0, 0.17, 0.33, 0.50, 0.67, 0.83, 1.0, 1.7};
		static constexpr std::array<float,9> rain_r = { 0.0, 0.0,  0.0,  0.0,  0.6,  1.0,  1.0, 1.0, 1.0};
		static constexpr std::array<float,9> rain_g = { 0.0, 0.0,  0.0,  1.0,  1.0,  1.0,  0.6, 0.0, 1.0};
		static constexpr std::array<float,9> rain_b = { 0.0, 0.3,  0.8,  1.0,  0.3,  0.0,  0.0, 0.0, 1.0};
		TriggerPlot (std::string dir_)  : dir(dir_) {
			charh = 0.65;
			tr[0] = 0.0f;
			tr[1] = 0.0f;
			tr[2] = 0.0f;
			tr[3] = 0.0f;
			tr[4] = 0.0f;
			tr[5] = 0.0f;
		}
		void Plot (
			const trigHead_t&     th,
			PtrFloat              bt,
			PtrFloat              dd
				) {
			// ALL PGPLOT routines are here
			snprintf (group, sizeof(group), "%s_muos_sn%03.2f_dm%04.2f_wd%04.2f", th.sigproc_file, th.sn, th.dm, th.width*1e3f);
			snprintf (filename, sizeof(filename), "%s.ps/vcps", group);
			auto fn = dir / filename;
			cpgbeg (0,fn.c_str(), 1, 1);      // begin plotting
			cpgsch(charh);                    // character height
			cpgask (0);                       // non interactive
			cpgpap (0.0,1.);               //10.0, width and aspect ratio
			// prepare data products for plotting
			float tleft   = th.i0 - th.epoch;
			float btdur   = th.btnsamps * th.tsamp / 1E6;
			float dddur   = th.ddnsamps * th.tsamp / 1E6;
			float dleft   = th.dm1;
			float dright  = dleft + (th.dmoff*th.ndm);
			float fleft   = th.fch1;
			float foff    = th.foff >= 0 ? th.foff : -th.foff;
			float fright  = fleft - (th.nchans*foff);
			float pt      = th.peak_time + (0.5*th.width);
			unsigned ipt  = pt / th.tsamp * 1E6;
			axtm.resize (th.btnsamps, 0.0f);
			tsn.resize  (th.btnsamps, 0.0f);
			{
				float last_time = tleft;
				float ttsamp = th.tsamp/1E6;
				unsigned start  = th.ndm/2;
				unsigned stride = th.btnsamps;
				for (unsigned i = 0; i < th.btnsamps; i++) {
					axtm[i] = last_time = last_time + ttsamp;
					tsn[i]  = bt[start*stride + i];
				}
			}
			axdm.resize (th.ndm, 0.0f);
			dsn.resize  (th.ndm, 0.0f);
			{
				float last_time = 0.0f;
				float ttdm = th.dmoff;
				unsigned start  = ipt;
				unsigned stride = th.btnsamps;
				for (unsigned i = 0; i < th.ndm; i++) {
					axdm[i] = last_time = last_time + ttdm;
					dsn[i]  = bt[start + (i*stride)];
				}
			}
			// PLOTTING
			// DD
			cpgsvp (0.1, 0.45, 0.1, 0.45);
			cpgswin (tleft, tleft+dddur, fleft, fright);
			cpgbox ("BCN",0.0,0,"BCMV",0.0,0);
			cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
			tr[0] = tleft;   tr[1] = 0.0f;    tr[2] = th.tsamp/1E6;
			tr[3] = th.fch1; tr[4] = -foff;   tr[5] = 0.0f;
			cpgimag (dd, th.nchans, th.ddnsamps,
					1, th.nchans, 1, th.ddnsamps,
					0, 255, 
					tr
			);
			cpgmtxt ("B", 2.5, 0.5, 0.5, "Time [s]");
			cpgmtxt ("L",1,0.5,0.5,"Freq [MHz]");
			// BT
			cpgsvp (0.1, 0.45, 0.55, 0.9);
			cpgswin (tleft, tleft+btdur, dleft, dright);
			cpgbox ("BCN",0.0,0,"BCMV",0.0,0);
			cpgctab (rain_l.data(), rain_r.data(), rain_g.data(), rain_b.data(), 5, contrast, brightness);
			tr[0] = tleft;   tr[1] = 0.0f;    tr[2] = th.tsamp/1E6;
			tr[3] = th.dm1; tr[4] = th.dmoff; tr[5] = 0.0f;
			cpgimag (bt, th.ndm, th.btnsamps,
					1, th.ndm, 1, th.btnsamps,
					0, 255, 
					tr
			);
			cpgmtxt ("B", 2.5, 0.5, 0.5, "Time [s]");
			cpgmtxt ("L",1,0.5,0.5,"DM [pc/cc]");
			// filesig
			cpgmtxt ("T",1.5*charh, 1.25, 0.5, group);
			// dSN
			cpgsvp (0.55, 0.9, 0.1, 0.45);
			cpgswin (dleft, dright, 0, 256);
			cpgbox ("ABN",0.0,0,"",0.0,0);
			cpgline (th.ndm, axdm.data(), dsn.data());
			cpgmtxt ("B", 2.5, 0.5, 0.5, "DM [pc/cc]");
			// source-name
			snprintf (txt, sizeof (txt), "Source=%s", th.name);
			cpgmtxt ("T",0.0f, 1.0, 1.0f, txt);
			// tSN
			cpgsvp (0.55, 0.9, 0.55, 0.9);
			cpgswin (tleft, tleft+btdur, 0, 256);
			cpgbox ("ABN",0.0,0,"",0.0,0);
			cpgline (th.btnsamps, axtm.data(), tsn.data());
			cpgmtxt ("B", 2.5, 0.5, 0.5, "Time [s]");
			// source-name
			snprintf (txt, sizeof(txt), "Peak time=%3.2fs",pt+tleft);
			cpgmtxt ("T",0.0f, 1.0, 1.0f, txt);
			cpgend ();
		}
};

constexpr std::array<float,5> TriggerPlot::heat_l;
constexpr std::array<float,5> TriggerPlot::heat_r;
constexpr std::array<float,5> TriggerPlot::heat_g;
constexpr std::array<float,5> TriggerPlot::heat_b;
constexpr std::array<float,9> TriggerPlot::rain_l;
constexpr std::array<float,9> TriggerPlot::rain_r;
constexpr std::array<float,9> TriggerPlot::rain_g;
constexpr std::array<float,9> TriggerPlot::rain_b;
