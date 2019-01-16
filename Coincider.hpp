#include "asgard.hpp"
#include "Filterbank.hpp"
#include "Candidate.hpp"

class Coincider {
		private:
				int numAnts;
				timeslice maxdelay;
				float ** mat;
				timeslice istart, istop;

		public:
				Coincider() {
						FilterbankReader fbreader;
						DedispManager ddm;
						maxdelay = 20;
						// parameters
				}
				void Coincide(StringVector& sfl, StringVector& scant) {
						numAnts = sfl.size();
						if(numAnts != scant.size()) {
								std::cerr << "COINCIDER::INPUT sizes don't match\n";
								return;
						}
						for(int iant = 0; iant < numAnts; iant++) {
								FilterbankCandidate thisFBC(sfl[iant], scant[iant]);
								for(int jant = 0; jant < numAnts; jant++) {
										if(jant == iant) continue;
										else {
												Filterbank otherfb;
												fbreader.Read(otherfb, sfl[jant]);
										}
								}
								

				}




};
