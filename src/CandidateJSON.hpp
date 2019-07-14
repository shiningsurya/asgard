#pragma once
#include "asgard.hpp"
// JSON
#include "nlohmann/json.hpp"
#include <sstream>
#include <iomanip>
// Digitizing
#include "Redigitizer.hpp"

class CandidateJSON {
 using json = nlohmann::json;
 private:
 fs::path opath;
 bool o_bson;
 uint64_t count;
 inline FloatVector VectorizeFloat(PtrFloat in, timeslice size) {
	// returns a FloatVector from pointer to be fed into json
	FloatVector ret(in, in + size);
	return ret;
 } 
 inline std::vector<unsigned char> VectorizeByte(PtrFloat in, timeslice size, unsigned int nbits) {
	// returns a ByteVector from pointer to be fed into json
	std::vector<unsigned char> ret;
	float alpha, beta, gamma, delta;
	if(nbits == 2) {
	 // 4 floats into one byte
	 for(timeslice i = 0; i < size;) {
		alpha = in[i++];
		beta  = in[i++];
		gamma = in[i++];
		delta = in[i++];
		ret.push_back(dig2bit(alpha, beta, gamma, delta, 0));
	 }
	}
	else if(nbits == 4) {
	 // 2 floats into one byte
	 for(timeslice i = 0; i < size;) {
		alpha = in[i++];
		beta  = in[i++];
		ret.push_back(dig4bit(alpha, beta, 0));
	 }
	}
	else if(nbits == 8) {
	 // 1 float into one byte
	 for(timeslice i = 0; i < size;) {
		alpha = in[i++];
		ret.push_back(dig8bit(alpha, 0));
	 }
	}
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
	 std::vector<std::uint8_t> j_bson = json::to_ubjson(j);
	 std::cout << " count=" << count << " size:" << std::setprecision(2) << j_bson.size()/1e6 << " MB" << std::endl;
	 std::ostream_iterator<uint8_t> oo(o);
	 std::copy(j_bson.begin(), j_bson.end(), oo);
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
	count = 0;
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
	 if(fbc.nsamps >= fbc.maxdelay)
		dd_nsamps = fbc.nsamps - fbc.maxdelay;
	 else
		dd_nsamps = fbc.maxdelay;
	 j["indices"]["dd_nsamps"] = dd_nsamps;
	 // header parameters
	 j["parameters"]["nbits"] = fbc.nbits;
	 j["parameters"]["isKur"] = fbc.isKur;
	 j["parameters"]["antenna"] = fbc.antenna;
	 j["parameters"]["group"] = fbc.group;
	 j["parameters"]["source_name"] = fbc.source_name;
	 // data
	 j["d_fb"] =   VectorizeByte(fbc.d_fb, fbc.nsamps * fbc.nchans, fbc.nbits);
	 j["dd_tim"] = VectorizeFloat(fbc.dd_tim, dd_nsamps);
	 j["dd_fb"] =  VectorizeByte(fbc.dd_fb, dd_nsamps * fbc.nchans, fbc.nbits);
	 // write
	 std::stringstream ss;
	 ss << fbc.group << "_" << fbc.antenna << "_" << fbc.curr;
	 ss << ".json"; // extension
	 std::cout << " filename=" << ss.str() << std::endl;
	 write(j, ss.str());
	 count++;
	}  while(fbc.Next());
 }
};
