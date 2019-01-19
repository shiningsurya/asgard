#include "asgard.hpp"
#include "Filterbank.hpp"


class Coadd_MAD {
		private:
				std::string method("MAD");
				bool returnFil, returnFloat;
				std::string filename;
				int numRuns;
		public:
				Coadd_MAD(std::string fn) {
						returnFil = true;
						returnFloat = false;
						filename = fn;
						numRuns = 0;
				}
				Coadd_MAD() {
						returnFil = false;
						returnFloat = true;
						numRuns = 0;
				}
				Filterbank Coadd(FilterbankList& in) {
						Filterbank fb;


						return fb;
				}


};
