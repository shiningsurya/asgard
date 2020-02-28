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
    // payloads
    json j;
    vb v_bt;
    vb v_dd;
    char group[32];
    char filename[256];
    // sizes
    unsigned dm_count;
    unsigned nchans;
    unsigned nsamps;
    timeslice btsize;
    timeslice ddsize;
    // slicing logic
    unsigned start;
    unsigned stop;
		static void center (unsigned nsize, unsigned cen, unsigned wid, unsigned& llimit, unsigned& rlimit) {
			unsigned hwid =  0.5 * wid;
			// min below because unsigned will overflow
			llimit = std::max (0, static_cast<int>(cen - hwid));
			rlimit = std::min (cen + wid + 1, nsize);
		}
  public:
    TriggerJSON ( 
    		std::string path_,
    		unsigned dmc = 256,
    		unsigned nmc = 256,
    		unsigned cmc = 64
				) : dirpath(path_), dm_count (dmc), nsamps(nmc), nchans(cmc){
			btsize = dm_count * nsamps;
			ddsize = nsamps * nchans;
		}
    bool WritePayload () {
      j["dd"] = v_dd;
      j["bt"] = v_bt;
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
      // clear payloads
      v_dd.clear();
      v_bt.clear();
      j.clear();
      return true;
    }
    void DumpHead (const Header_t& h, const trigger_t& t) {
      // header
      j["sn"]    = t.sn;
      j["dm"]    = t.dm;
      j["width"] = t.width;
      // header frequency
      j["frequency"]["fch1"] = h.fch1;
      j["frequency"]["foff"] = h.foff * h.nchans / nchans;
      j["frequency"]["nchans"] = nchans;
      // header parameters
      j["parameters"]["nbits"]       = h.nbits;
      j["parameters"]["antenna"]     = h.stationid;
      j["parameters"]["source_name"] = h.name;
      j["parameters"]["ra"] = h.ra;
      j["parameters"]["dec"] = h.dec;
      // header time
      j["time"]["tsamp"] = h.tsamp;
      j["time"]["tstart"] = h.tstart;
      j["time"]["nsamps"] = nsamps;
      float duration = nsamps * h.tsamp / 1E6;
      j["time"]["duration"] = duration;
      timeslice ipt = t.peak_time / h.tsamp * 1E6;
      start = std::max (0, static_cast<int>(ipt - (0.5*nsamps)));
      stop  = start + nsamps;
      // header indices
      float cut = start * h.tsamp / 1e6;
      j["time"]["peak_time"] = t.peak_time - cut;
      j["indices"]["i0"] = t.i0 + cut;
      j["indices"]["i1"] = t.i0 + cut + duration;
      j["indices"]["epoch"] = h.epoch;
      // name
      // group
      struct tm utc_time;
      time_t hepoch = h.epoch;
      gmtime_r (&hepoch, &utc_time);
      strftime (group, sizeof(group), "%Y%m%d_%H%M%S", &utc_time);
      j["parameters"]["group"] = std::string(group);
      snprintf(filename, sizeof(filename),
          "%s_muos_ea%02d_sn%05.2f_dm%05.2f_wd%05.2f.dbson", group, h.stationid,t.sn,t.dm,t.width*1e3f
      );
    }
    // it is assumed that
    // the data products 
    // are time-aligned
    // and if peak_time is the same for both slicing
    // both slices will be identical
    // however it is not assumed that 
    // bt and dd are of same size in time axis
		void DumpBT (const vf& bt, unsigned nbt, float dm1, float dmoff) {
      std::cout << "TriggerJSON::DumpBT shape=("<< dm_count << "," << nbt << ")" << std::endl;
			if (1) {
				std::ofstream ofs("ubt.dat");
				std::copy (bt.begin(),bt.end(), std::ostream_iterator<float>(ofs,"\n"));
			}
      // dms
      j["dms"]["dm1"]   = dm1;
      j["dms"]["dmoff"] = dmoff;
      j["dms"]["ndm"]   = dm_count;
      // shouldn't happen but safety first logic
      auto istop = std::min (nbt, stop);
      vf ret;
			for (unsigned idm = 0; idm < dm_count; idm++) {
				for (timeslice ii = start; ii < istop; ii++) {
					ret.push_back (bt[ii + nbt*idm]);
				}
			}
			// perform linear coding
			float mmin = std::numeric_limits<float>::max();
			float mmax = std::numeric_limits<float>::min();
			std::for_each (ret.cbegin(), ret.cend(), [&mmin, &mmax] (const float& ss) {
					if (ss > mmax) mmax = ss;
					if (ss < mmin) mmin = ss;
			});
			// when mmax == mmin (when input is null)
			// default to 1.0f
			float idm  = mmax == mmin ? 1.0f : 1.0f / (mmax - mmin);
			std::transform (ret.cbegin(), ret.cend(), std::back_inserter(v_bt), [&idm, &mmin](const float& ss) -> Byte {
					return 255 * idm * (ss - mmin);
			});
	if (1) {
		std::ofstream ofs("ssbt.dat");
		std::copy (v_bt.begin(), v_bt.end(), std::ostream_iterator<float>(ofs,"\n"));
	}
		}
		void DumpDD (const vb& dd, unsigned ndd, unsigned nchansin) {
      std::cout << "TriggerJSON::DumpDD shape=(" << nsamps << "," << nchans << ")" << std::endl;
      timeslice istart = start * nchansin;
      // shouldn't happen but safety first logic
      timeslice istop  = std::min (ndd, stop) * nchansin;
      // fscrunch
      unsigned fac = nchansin / nchans;
      float ifac = 1.0f/ fac;
      float xf = 0.0f;
      // algo
      // slicing and fscrunching at the same time
      vb& ret = v_dd;
			for (timeslice ii = istart; ii < istop; ii+=fac) {
				xf = 0.0f;
				for (unsigned ik = 0; ik < fac; ik++) {
					xf += dd[ii + ik];
				}
				ret.push_back (static_cast<Byte>( xf * ifac ));
			}
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
