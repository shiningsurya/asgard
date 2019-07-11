#pragma once
#include "asgard.hpp"
#include "nlohmann/json.hpp"
#include <sstream>

class CandidateJSON {
		using json = nlohmann::json;
		private:
		fs::path opath;
		bool o_bson;
		inline FloatVector Vectorize(PtrFloat in, timeslice size) {
				// returns a FloatVector from pointer to be fed into json
				FloatVector ret(in, in + size);
				return ret;
		} 
		bool write(const json&  j, const std::string& filename) const {
				auto op = opath / filename;
				std::ofstream o(op.string());
				if(!o.is_open()) {
						std::cerr << "CandidateJSON::File not open!\n";
						return false;
				}
				if(o_bson) {
						// write to BSON
						std::vector<std::uint8_t> j_bson = json::to_bson(j);
						//o << j_bson;
				}
				else {
						// defaults to JSON
						o << j;
				}
				o.close();
				return true;
		}
		public:
				CandidateJSON(std::string path = std::string("./"), bool _bson = false):
						opath(path),
						o_bson(_bson)
				{
						// if path doesn't exist
						// create
						if(! fs::exists(opath)) fs::create_directory(opath);
						
				}
				~CandidateJSON() {
						// clear any dynamic memory?
				}
				void Write(FilterbankCandidate& fbc) {
						timeslice dd_nsamps;
						do {
								json j;
								// header
								j["sn"] = fbc.sn;
								j["dm"] = fbc.dm;
								j["filterwidth"] = fbc.filterwidth;
								// header time
								j["time"]["peak_time"] = fbc.peak_time;
								j["time"]["duration"] = fbc.duration;
								j["time"]["tstart"] = fbc.tstart;
								j["time"]["tsamp"] = fbc.tsamp;
								// header frequency
								j["frequency"]["fch1"] = fbc.fch1;
								j["frequency"]["foff"] = fbc.foff;
								j["frequency"]["nchans"] = fbc.nchans;
								// header indices
								j["indices"]["i0"] = fbc.i0;
								j["indices"]["i1"] = fbc.i1;
								j["indices"]["peak_index"] = fbc.peak_index;
								j["indices"]["maxdelay"] = fbc.maxdelay;
								j["indices"]["istart"] = fbc.istart;
								j["indices"]["istop"] = fbc.istop;
								j["indices"]["nsamps"] = fbc.nsamps;
								j["indices"]["dd_nsamps"] = fbc.nsamps - fbc.maxdelay;
								dd_nsamps = fbc.nsamps - fbc.maxdelay;
								// header parameters
								j["parameters"]["nbits"] = fbc.nbits;
								j["parameters"]["isKur"] = fbc.isKur;
								j["parameters"]["antenna"] = fbc.antenna;
								j["parameters"]["group"] = fbc.group;
								j["parameters"]["source_name"] = fbc.source_name;
								// data
								j["d_fb"] = Vectorize(fbc.d_fb, fbc.nsamps * fbc.nchans);
								j["dd_tim"] = Vectorize(fbc.dd_tim, dd_nsamps);
								j["dd_fb"] = Vectorize(fbc.dd_fb, dd_nsamps * fbc.nchans);
								// write
								std::stringstream ss;
								ss << fbc.group << "_" << fbc.antenna << "_" << fbc.curr;
								ss << ".json"; // extension
								write(j, ss.str());
						}  while(fbc.Next());
				}
};
