#include "asgard.hpp"
#ifndef PLOTTER_H
#define PLOTTER_H
#include "cpgplot.h"
// base class
class Plotter {
		protected:
				std::string filename;
				bool interactive;
				float charh;
				int count, nant;
				// for images
				float contrast, brightness;
				float tr[6];
				std::array<float,5> heat_l;
				std::array<float,5> heat_r;
				std::array<float,5> heat_g;
				std::array<float,5> heat_b;
				float xmin, xmax, ymin, ymax, w;
				float fac;
		public:
				Plotter(std::string fn) {
						interactive = fn == std::string("?");
						filename = fn;
						count = 0;
						charh = 0.65;
						heat_l = {0.0, 0.2, 0.4, 0.6, 1.0};
						heat_r = {0.0, 0.5, 1.0, 1.0, 1.0};
						heat_g = {0.0, 0.0, 0.5, 1.0, 1.0};
	 					heat_b = {0.0, 0.0, 0.0, 0.3, 1.0};
						contrast = 1;
						brightness = 0.5;
						cpgbeg(0,filename.c_str(),1,1); // beginning of another journey
						cpgsch(charh); // character height
						if(interactive) cpgask(1); // go manual mode
						else cpgask(0);
						tr[0] = 0.0f;
						tr[1] = 0.0f;
						tr[2] = 0.0f;
						tr[3] = 0.0f;
						tr[4] = 0.0f;
						tr[5] = 0.0f;
				}
				~Plotter() {
						cpgend();
				}
};
#ifdef FILTERBANK_H
auto FCompare = [](Filterbank& x, Filterbank& y) { return x.antenna < y.antenna; };
FilterbankList FLFromPL(const PathList& x) {
		FilterbankList ret;
		FilterbankReader fbr;
		for(const auto& de : x) {
				Filterbank xx;
				fbr.Read(xx, de.string()); 
				ret.push_back(xx);
		}
		return ret;
		// I would like to take time to tell you that
		// I took 10 mins into debugging why this function was 
		// going haywire.
		// And the reason is self explanatory:
		// I forgot "return ret"
}
FilterbankList FLFromDE(DEList& x) {
		FilterbankList ret;
		FilterbankReader fbr;
		for(fs::directory_entry& de : x) {
				Filterbank xx;
				fbr.Read(xx, de.path().string()); 
				ret.push_back(xx);
		}
		return ret;
		// I would like to take time to tell you that
		// I took 10 mins into debugging why this function was 
		// going haywire.
		// And the reason is self explanatory:
		// I forgot "return ret"
}
#endif // FILTERBANK_H
#ifdef CANDIDATE_H
auto CLCompare = [](CandidateList& x, CandidateList& y) { return x[0].antenna < y[0].antenna; };
auto CCompare = [](Candidate& x, Candidate& y) { return x.antenna < y.antenna; };
auto SCompare = [](std::string& x, std::string& y) { return GetAntenna(x) < GetAntenna(y); };
auto iCompare = [](Candidate& x, Candidate& y) { return x.i0 < y.i0; };
CandidateAntenna CAFromPL(const PathList& x) {
		CandidateAntenna ret;
		double zero = 0.0;
		for(const auto& de : x) ret.push_back( ReadCandidates( de.string(), zero) );
		return ret;
}
CandidateAntenna CAFromDE(DEList& x) {
		CandidateAntenna ret;
		double zero = 0.0;
		for(fs::directory_entry& de : x) ret.push_back( ReadCandidates( de.path().string(), zero) );
		return ret;
}
#endif // CANDIDATE_H
StringVector SVFromDE(DEList& x) {
		StringVector ret;
		for(fs::directory_entry& de : x) ret.push_back( de.path().string() );
		return ret;
}
StringVector SVFromPL(PathList& x) {
		StringVector ret;
		for(auto& de : x) ret.push_back( de.string() );
		return ret;
}
#endif
