#pragma once
#include "asgard.hpp"
#include "Header.hpp"
// JSON
#include "nlohmann/json.hpp"
#include <sstream>
#include <iomanip>
// Digitizing
#include "Redigitizer.hpp"

class FilterbankJSON {
  using json = nlohmann::json;
  using Byte = unsigned char;
  private:
    fs::path dirpath;           // root directory
    // payloads
    char filename[256];
    char group[16];
    json j;
    std::vector<Byte> v_fb;
  public:
    timeslice nsamps;
    FilterbankJSON ( std::string path_ ) : dirpath(path_) {}
    // 
    bool WritePayload (timeslice siz) {
      // check if we have full data
      if (v_fb.size() != siz) {
        // oh boi
        // this shouldn't happen
        std::cerr << "Mismatch in size! should be size="  << nsamps;
        std::cerr << " but is=" << v_fb.size() << std::endl;
      }
      j["fb"] = v_fb;
      // write json as ubjson to file
      auto ofile = dirpath / filename;
      std::ofstream ofs(ofile.string());
      if(!ofs.is_open()) {
        std::cerr << "FilterbankJSON::File not open!\n";
        return false;
      }
      // write to BSON
      std::vector<std::uint8_t> j_bson = json::to_ubjson(j);
std::cout << " size:" << std::setprecision(2) << j_bson.size()/1e6 << " MB" << std::endl;
      std::ostream_iterator<uint8_t> oo(ofs);
      std::copy(j_bson.begin(), j_bson.end(), oo);
      ofs << std::endl;
      ofs.close();
      // clear payloads
      v_fb.clear();
      j.clear();
      nsamps = 0;
      return true;
    }
    void DumpHead (const Header_t& head, const trigger_t& trig ) {
      // header
      j["sn"] = trig.sn;
      j["dm"] = trig.dm;
      j["width"] = trig.width;
      // header time
      j["time"]["tsamp"] = head.tsamp;
      // header frequency
      j["frequency"]["fch1"] = head.fch1;
      j["frequency"]["foff"] = head.foff;
      j["frequency"]["nchans"] = head.nchans;
      // header indices
      j["indices"]["i0"] = trig.i0;
      j["indices"]["i1"] = trig.i1;
      //j["indices"]["peak_index"] = fbc.peak_index;
      //j["indices"]["maxdelay"] = fbc.maxdelay;
      //j["indices"]["istart"] = fbc.istart;
      //j["indices"]["istop"] = fbc.istop;
      float dur = std::ceil(trig.i1 - trig.i0);
      nsamps = trig.i1 >= trig.i0 ? dur/head.tsamp/1e-6 * head.nchans * head.nbits / 8 : 0L;
      j["indices"]["nsamps"] = nsamps;
      // header parameters
      j["parameters"]["nbits"] = head.nbits;
      //j["parameters"]["isKur"] = fbc.isKur;
      j["parameters"]["antenna"] = head.stationid;
      j["parameters"]["source_name"] = head.name;

      // some book-keeping 
      j["time"]["duration"] = dur;
      double sec_from_start = trig.i0 - head.epoch;
      j["time"]["tstart"] = head.tstart + (sec_from_start/86400.0f);
      j["time"]["peak_time"] = trig.peak_time - sec_from_start;

      // group
      struct tm utc_time;
      time_t hepoch = head.epoch;
      gmtime_r (&hepoch, &utc_time);
      strftime (group, sizeof(group), "%Y%m%d_%H%M%S", &utc_time);
      j["parameters"]["group"] = std::string(group);
      snprintf(filename, sizeof(filename),
          "%s_muos_ea%02d_dm%3.2f_sn%2.2f.fbson", group, head.stationid,trig.dm,trig.sn 
      );
    }
    void DumpData (Byte* ptr, timeslice start, timeslice off) {
      std::cout << "FilterbankJSON::DumpData start=" << start << " off=" << off<< std::endl;
      std::copy (
          ptr + start, ptr + start + off,
          std::back_inserter(v_fb)
      );
    }
};
