#pragma once
#include "asgard.hpp"
#include "Header.hpp"
// JSON
#include "nlohmann/json.hpp"
using json = nlohmann::json;

class TriggerJSON {
  using Byte = unsigned char;
  using vb   = std::vector<Byte>;
  using vf   = std::vector<float>;
  private:
    fs::path dirpath;           // root directory
    // payload
    json j;
    char group[32];
    char filename[256];
  public:
    TriggerJSON ( 
    		std::string path_
				) : dirpath(path_){}
    bool WritePayload () {
      // write json as ubjson to file
      auto ofile = dirpath / filename;
      std::ofstream ofs(ofile.string());
      if(!ofs.is_open()) {
        std::cerr << "TriggerJSON::File not open!\n";
        return false;
      }
      // write to BSON
      std::vector<std::uint8_t> j_bson = json::to_ubjson(j);
      std::ostream_iterator<uint8_t> oo(ofs);
      std::copy(j_bson.begin(), j_bson.end(), oo);
      //ofs << std::endl;
      ofs.close();
      // clear payload
      j.clear();
      return true;
    }
    void DumpHead (const trigHead_t& th) {
      // header
      j["sn"]    = th.sn;
      j["dm"]    = th.dm;
      j["width"] = th.width;
      // header frequency
      j["frequency"]["fch1"] = th.fch1;
      j["frequency"]["foff"] = th.foff;
      j["frequency"]["nchans"] = th.nchans;
      // header parameters
      j["parameters"]["nbits"]       = th.nbits;
      j["parameters"]["antenna"]     = th.stationid;
      j["parameters"]["source_name"] = th.name;
      j["parameters"]["ra"] = th.ra;
      j["parameters"]["dec"] = th.dec;
      // header time
      j["time"]["tsamp"] = th.tsamp;
      j["time"]["tstart"] = th.tstart;
      j["time"]["nsamps"] = th.nsamps;
      j["time"]["duration"] = th.dur;
      // header indices
      j["time"]["peak_time"] = th.peak_time;
      j["indices"]["i0"] = th.i0;
      j["indices"]["i1"] = th.i1;
      j["indices"]["epoch"] = th.epoch;
      // header dm
      // rest go with DumpDD call
      j["dms"]["ndm"]   = th.ndm;
      // name
      // group
      struct tm utc_time;
      time_t hepoch = th.epoch;
      gmtime_r (&hepoch, &utc_time);
      strftime (group, sizeof(group), "%Y%m%d_%H%M%S", &utc_time);
      j["parameters"]["group"] = std::string(group);
      snprintf(filename, sizeof(filename),
          "%s_muos_ea%02d_sn%05.2f_dm%05.2f_wd%05.2f.dbson", group, th.stationid,th.sn,th.dm,th.width*1e3f
      );
    }
    // it is assumed that
    // the data products 
    // are time-aligned
    // and if peak_time is the same for both slicing
    // both slices will be identical
    // however it is not assumed that 
    // bt and dd are of same size in time axis
    // ----
    // all slicing logic is pushed onto trigger master
    // so everything beyond can benefit from it
		void DumpBT (const vb& bt, float dm1, float dmoff) {
      // dms
      j["dms"]["dm1"]   = dm1;
      j["dms"]["dmoff"] = dmoff;
      // bt
      j["bt"] = bt;
		}
		void DumpDD (const vb& dd) {
      // simple copy
      j["dd"] = dd;
		}
};

class DBSON {
	private:
		std::string filename;
	public:
		DBSON (std::string fn_) : filename (fn_) {
			std::ifstream ifs(filename, std::ios::binary);
			std::vector<uint8_t> vb;
			if( ifs.good() ) {
				while( !ifs.eof() ) vb.push_back(ifs.get()); 
			}
			else {
				std::cerr << "DBSON::ctor file not open!"<< std::endl;
			}
			// one endl is causing us have two pop backs?
			// only one pop back required.
			vb.pop_back(); 
			// that parsing step
      json j = json::from_ubjson(vb);
			// alright now parameter writing
			// header
			sn = j["sn"];
			dm = j["dm"];
			width = j["width"];
			// header time
			peak_time = j["time"]["peak_time"];
			tstart = j["time"]["tstart"];
			tsamp = j["time"]["tsamp"];
			dur =   j["time"]["duration"];
			nsamps = j["time"]["nsamps"];
			// header frequency
			fch1 = j["frequency"]["fch1"];
			foff = j["frequency"]["foff"];
			nchans = j["frequency"]["nchans"];
			// header indices
			i0 = j["indices"]["i0"];
			i1 = j["indices"]["i1"];
			epoch = j["indices"]["epoch"];
			// header parameters
			nbits = j["parameters"]["nbits"];
			stationid = j["parameters"]["antenna"];
			name = j["parameters"]["source_name"];
			ra = j["parameters"]["ra"];
			dec = j["parameters"]["dec"];
			sigproc_file = j["parameters"]["group"];
			// header dms
			dm1 = j["dms"]["dm1"];
			dmoff = j["dms"]["dmoff"];
			ndm = j["dms"]["ndm"];
			// data
			auto bdd = j["dd"];
			auto bbt = j["bt"];
			bt.clear (); dd.clear ();
			std::copy (bdd.cbegin(), bdd.cend(), std::back_inserter(dd));
			std::copy (bbt.cbegin(), bbt.cend(), std::back_inserter(bt));
		}
		trigHead_t GetTrigHead () const {
			trigHead_t ret;
      // header
      ret.sn    = sn;
      ret.dm    = dm;
      ret.width = width;
      // header frequency
      ret.fch1 = fch1;
      ret.foff = foff;
      ret.nchans = nchans;
      // header indices
      ret.i0 = i0;
      ret.i1 = i1;
      ret.epoch = epoch;
      // header parameters
      ret.nbits       = nbits;
      ret.stationid = stationid;
      strcpy (ret.name, name.c_str());
      strcpy (ret.sigproc_file, sigproc_file.c_str());
      ret.ra = ra;
      ret.dec = dec;
      // dms
      ret.dm1   = dm1;
      ret.dmoff = dmoff;
      ret.ndm   = ndm;
      ret.nsamps = nsamps;
      // header time
      ret.tsamp = tsamp > 1 ? tsamp : tsamp * 1E6;
      ret.dur = dur;
      ret.tstart = tstart;
      ret.peak_time = peak_time; 
      ret.nsamps = nsamps;
      //
      return ret;
		}
		// header
		double i0;
		double i1;
		float sn;
		float dm;
		float width;
		float peak_time;
		float dur;
		// station
		int stationid;
		// positions
		double ra, dec;
		// frequency
		double fch1, foff;
		// time
		double tsamp, tstart, epoch;
		// dm
		double dm1, dmoff;
		// memorys
		unsigned nbits, nchans, nsamps, ndm;
		// strs
		std::string name, sigproc_file;
		// data
		std::vector<float> bt;
		std::vector<float> dd;
};

std::ostream& operator<<(std::ostream& os, const DBSON& f) {
  os << std::fixed;
  os << "S/N          " << f.sn << std::endl;
  os << "DM (pc/cc)   " << f.dm << std::endl;
  os << "Width (ms)   " << f.width << std::endl;
  os << "Duration(s)  " << f.dur<< std::endl;
  os << "Peak_time    " << f.peak_time << std::endl;
  os << "Source       " << f.name << std::endl;
  os << "Antenna      ea" << f.stationid<< std::endl;
  os << "tsamp        " << f.tsamp << std::endl;
  os << "foff " << f.foff << std::endl;
  os << "Bowtie shape = (" << f.ndm << "," << f.nsamps << ")" << std::endl;
  os << "Ddfb shape = (" << f.nsamps << "," << f.nchans << ")" << std::endl;
  os << "BT.size=" << f.bt.size () << std::endl;
  os << "DD.size=" << f.dd.size () << std::endl;
  return os;
};
