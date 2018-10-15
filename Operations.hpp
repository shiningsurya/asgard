#include "asgard.hpp"

namespace operations {
		SingleAntennaCoadd(float * in, float * out, timeslice wid, int nchans);
		AntennaCoadd(std::vector<float*> in, float * out, timeslice wid, int nchans);
		Dedisperse(float * in, float * out, double dm);
}

operations::SingleAntennaCoadd(float * in, float * out, timeslice wid, int nchans) {
		for(timeslice i = 0; i < wid, i++) {
				out[i] = 0.0f;
				for(int j = 0; j < nchans; j++) {
						out[i] += in[i*nchans + j]
				}
				out[i] /= nchans;
		}
}

operations::Dedisperse(float * in, float * out, double dm) {


}
