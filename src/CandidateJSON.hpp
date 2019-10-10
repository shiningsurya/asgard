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
 PtrFloat dd_fb_fs, d_fb_fs;
 const unsigned int chanout, bitsout;
 inline FloatVector VectorizeFloat(PtrFloat in, timeslice size) {
   // returns a FloatVector from pointer to be fed into json
   FloatVector ret(in, in + size);
   return ret;
 } 
 inline std::vector<unsigned char> VectorizeByte(PtrFloat in, timeslice size, unsigned int nbits) {
     // returns a ByteVector from pointer to be fed into json
     std::vector<unsigned char> ret;
     ret.reserve(size * nbits / 8);
     float alpha, beta, gamma, delta;
     float rmean = 1.12f, rstd = 0.92f;
     if(nbits == 2) {
      // 4 floats into one byte
      for(timeslice i = 0; i < size;) {
      alpha = (in[i++] - rmean) / rstd;
      beta  = (in[i++] - rmean) / rstd;
      gamma = (in[i++] - rmean) / rstd;
      delta = (in[i++] - rmean) / rstd;
      ret.push_back(dig2bit(alpha, beta, gamma, delta));
    }
   }
   else if(nbits == 4) {
    // 2 floats into one byte
    for(timeslice i = 0; i < size;) {
    alpha = (in[i++] - rmean) / rstd;
    beta  = (in[i++] - rmean) / rstd;
    ret.push_back(dig4bit(alpha, beta));
    }
   }
   else if(nbits == 8) {
    // 1 float into one byte
    for(timeslice i = 0; i < size;) {
    alpha = (in[i++] - rmean) / rstd;
    ret.push_back(dig8bit(alpha));
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
    std::cout << " count=" << count << " size:" << std::setprecision(2) << j_bson.size()/1e6 << " MB";
    std::cout << " dm=" << j["dm"] << " dd_nsamps=" << j["indices"]["dd_nsamps"]  << std::endl;
    std::ostream_iterator<uint8_t> oo(o);
    std::copy(j_bson.begin(), j_bson.end(), oo);
   }
   else {
    // defaults to JSON
    o << j;
   }
    o << std::endl;
    o.close();
   return true;
 }
 public:
 CandidateJSON(std::string path = std::string("./"), bool _bson = false, unsigned int chanout_ = 64, unsigned int bitsout_ = 2):
	opath(path),
	chanout(chanout_),
	bitsout(bitsout_),
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
	 j["frequency"]["foff"] = fbc.foff * fbc.nchans / chanout;
	 j["frequency"]["nchans"] = chanout;
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
	 j["parameters"]["nbits"] = bitsout;
	 j["parameters"]["isKur"] = fbc.isKur;
	 j["parameters"]["antenna"] = fbc.antenna;
	 j["parameters"]["group"] = fbc.group;
	 j["parameters"]["source_name"] = fbc.source_name;
	 // data
	 d_fb_fs = new float[fbc.nsamps * chanout];
	 dd_fb_fs = new float[dd_nsamps * chanout];
	 operations::Fscrunch(fbc.d_fb, fbc.nchans, fbc.nsamps, chanout, d_fb_fs);
	 operations::Fscrunch(fbc.dd_fb, fbc.nchans, dd_nsamps, chanout, dd_fb_fs);
	 j["d_fb"] =   VectorizeByte(d_fb_fs, fbc.nsamps * chanout, bitsout);
	 j["dd_tim"] = VectorizeFloat(fbc.dd_tim, dd_nsamps);
	 j["dd_fb"] =  VectorizeByte(dd_fb_fs, dd_nsamps * chanout, bitsout);
	 std::cout << " size dd_fb=" << VectorizeByte(dd_fb_fs, dd_nsamps * chanout, bitsout).size() << std::endl;
	 delete[] d_fb_fs; delete[] dd_fb_fs;
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
