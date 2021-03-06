#pragma once
#include "asgard.hpp"
#include "Header.hpp"
// JSON
#include "nlohmann/json.hpp"
using json = nlohmann::json;
#include <sstream>
#include <iomanip>
// Digitizing
#include "Redigitizer.hpp"
// Plotting
#include "Plotter.hpp"

class FilterbankJSON {
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
        std::cerr << "Mismatch in size! should be size="  << siz;
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
      // header frequency
      j["frequency"]["fch1"] = head.fch1;
      j["frequency"]["foff"] = head.foff;
      j["frequency"]["nchans"] = head.nchans;
      // header indices
      j["indices"]["i0"] = trig.i0;
      j["indices"]["i1"] = trig.i1;
      j["indices"]["epoch"] = head.epoch;
      float dur = std::ceil(trig.i1 - trig.i0);
      nsamps = trig.i1 >= trig.i0 ? dur/head.tsamp/1e-6 * head.nchans * head.nbits / 8 : 0L;
      j["indices"]["nsamps"] = nsamps;
      // header parameters
      j["parameters"]["nbits"] = head.nbits;
      j["parameters"]["antenna"] = head.stationid;
      j["parameters"]["source_name"] = head.name;
      j["parameters"]["ra"] = head.ra;
      j["parameters"]["dec"] = head.dec;
      // header time
      j["time"]["tsamp"] = head.tsamp;
      j["time"]["duration"] = dur;
      double sec_from_start = trig.i0 - head.epoch;
      j["time"]["tstart"] = head.tstart + (sec_from_start/86400.0f);
      j["time"]["peak_time"] = (head.epoch + trig.peak_time) - trig.i0;
      j["time"]["tpeak"] = trig.peak_time; 

      // group
      struct tm utc_time;
      time_t hepoch = head.epoch;
      gmtime_r (&hepoch, &utc_time);
      strftime (group, sizeof(group), "%Y%m%d_%H%M%S", &utc_time);
      j["parameters"]["group"] = std::string(group);
      snprintf(filename, sizeof(filename),
          "%s_muos_ea%02d_dm%05.2f_sn%05.2f_wd%05.2f.fbson", group, head.stationid,trig.dm,trig.sn,trig.width*1e3f
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

class FBDump : public Header_t, public trigger_t {
	private:
		std::string filename;
  public:
    FBDump (std::string _file) : filename(_file) {
      // defaulting to ubson
      std::ifstream ifs(filename, std::ios::binary);
      std::vector<uint8_t> vb;
      if( ifs.good() ) {
         while( !ifs.eof() ) vb.push_back(ifs.get()); 
      }
      else {
          std::cerr << "FBDump::ctor file not open!"<< std::endl;
      }
      // need these bc I was dumb that I wrote std::endl in bson
      vb.pop_back(); vb.pop_back();
      //
      json j = json::from_ubjson(vb);
      // alright now parameter writing
      // header
			try {
				sn = j["sn"];
				dm = j["dm"];
				width = j["width"];
				// header time
				peak_time = j["time"]["peak_time"];
				tpeak     = j["time"]["tpeak"];
				tstart = j["time"]["tstart"];
				tsamp = j["time"]["tsamp"];
				duration = j["time"]["duration"];
				// header frequency
				fch1 = j["frequency"]["fch1"];
				foff = j["frequency"]["foff"];
				nchans = j["frequency"]["nchans"];
				// header indices
				nsamps = j["indices"]["nsamps"];
				i0 = j["indices"]["i0"];
				i1 = j["indices"]["i1"];
				epoch = j["indices"]["epoch"];
				// header parameters
				nbits = j["parameters"]["nbits"];
				stationid = j["parameters"]["antenna"];
				strcpy (name, std::string(j["parameters"]["source_name"]).c_str());
				ra = j["parameters"]["ra"];
				dec = j["parameters"]["dec"];
				strcpy (sigproc_file, std::string(j["parameters"]["group"]).c_str());
			} 
			catch (json::exception& e) {
				std::cout << "exception found while reading!" << std::endl;
				std::cout << "msg: " << e.what () << std::endl;
			}
			// -- payload
			auto fb_ = j["fb"];
			std::copy (fb_.cbegin(), fb_.cend(), std::back_inserter(fb));
      //
      ifs.close();
    }
    FBDump() = default;
    ~FBDump() = default;

		std::vector<unsigned char> fb;
    float tpeak;
    float duration;
    timeslice nsamps;
};

Header_t getHead (const FBDump& f) {
	Header_t dummyhead;
	dummyhead.stationid = f.stationid;
	dummyhead.ra        = f.ra;
	dummyhead.dec       = f.dec;
	dummyhead.fch1      = f.fch1;
	dummyhead.foff      = f.foff;
	dummyhead.tsamp     = f.tsamp;
	dummyhead.tstart    = f.tstart;
	dummyhead.epoch     = f.epoch;
	dummyhead.nbits     = f.nbits;
	dummyhead.nchans    = f.nchans;
	strcpy (dummyhead.name, f.name);
	strcpy (dummyhead.sigproc_file, f.sigproc_file);
	return dummyhead;
}

trigger_t getTrig (const FBDump& f) {
	trigger_t dummytrig;
	dummytrig.sn        = f.sn;
	dummytrig.dm        = f.dm;
	dummytrig.width     = f.width;
	dummytrig.i0        = f.i0;
	dummytrig.i1        = f.i1;
	dummytrig.peak_time = f.tpeak;
	return dummytrig;
}

std::ostream& operator<<(std::ostream& os, const FBDump& f) {
  os << std::fixed;
  os << "S/N          " << f.sn << std::endl;
  os << "DM (pc/cc)   " << f.dm << std::endl;
  os << "Width (ms)   " << f.width << std::endl;
  os << "Duration(s)  " << f.duration << std::endl;
  os << "Peak_time    " << f.peak_time << std::endl;
  os << "Source       " << f.name << std::endl;
  os << "Antenna      ea" << f.stationid<< std::endl;
  os << "tsamp        " << f.tsamp << std::endl;
  os << "foff        " << f.foff << std::endl;
  os << "Nsamps=" << f.nsamps << "  Nchans=" << f.nchans << "  Nbits=" << f.nbits << std::endl;
  return os;
}

#if 0
class FBDPlot: protected Plotter {
  private:
    unsigned int count;
    float max, min, xxmin, xxmax, dd_range;
    PtrFloat fb_fscrunched, fb_tshape, fb_fshape, fb_dd;
    unsigned int nchans, chanout;
    timeslice wid;
    FloatVector axfreq_vec, axtime_vec;
    // 
    float sn;
    float dm;
    float width;
    std::string group, name;
    int stationid;
    double tstart;
    int nbits;
    float peak_time;
    inline void _safe_delete(PtrFloat& x) {
      if(x) delete[] x;
      x = nullptr;
    }
  protected:
    int imin, imax;
    float xlin[2], ylin[2];
    char txt[256];
    unsigned int txtrow, csize;
    float txtheight;
    float tmax;
  public:
    FBDPlot(std::string fn, int _chout = 512) : Plotter(fn) {
      cpgpap (0.0,0.618); //10.0, width and aspect ratio
      fac = 1e-2f;
      txtheight = -1.5 * charh;
      //
      chanout = _chout;
      fb_fscrunched = nullptr;
      fb_tshape = nullptr; fb_fshape = nullptr;
      fb_dd = nullptr;
      wid = 0; 
      count = 0;
    }
    void Plot1(const FBDump& fcl) {
      if(count) _cleanup();
      nchans = fcl.nchans;
      // wid
      wid = fcl.nsamps;
      fb_tshape = new float[wid]();
      fb_fshape = new float[chanout]();
      // freq axis
      axfreq_vec = operations::FreqTable((float)fcl.fch1, (float)fcl.foff*fcl.nchans/chanout, chanout);
      auto axfreq_vec_full = operations::FreqTable((float)fcl.fch1, (float)fcl.foff, fcl.nchans);
      assert (axfreq_vec_full.size() == fcl.nchans);
      // dm
      auto idelays = operations::Delays( 
        axfreq_vec_full, fcl.dm, fcl.tsamp/1E6
      );
      wid = fcl.nsamps - idelays.back();
      // time axis
      axtime_vec = operations::TimeAxis(fcl.tsamp/1E6, 0L, wid);
      tmax = fcl.peak_time + (10*fcl.width);
      tmax = tmax <= 1 ? tmax : 1.0f;
      fb_dd = new float[wid * fcl.nchans]();
      operations::InCoherentDD(
        fcl.fb, idelays, wid, fb_dd
      );
      // fscrunch, shapes
      fb_fscrunched = new float[(wid * chanout)]();
      operations::Fscrunch(
        fb_dd, fcl.nchans, wid, chanout, fb_fscrunched
      );
      operations::TimeFreqShape(
        fb_fscrunched, wid, chanout, fb_tshape, fb_fshape 
      );
      //// plotting
      sn = fcl.sn;
      dm = fcl.dm;
      width = fcl.width*1e3;
      group = fcl.group;
      stationid = fcl.stationid;
      tstart = fcl.tstart;
      name = fcl.name;
      peak_time = fcl.peak_time;
      nbits = fcl.nbits;
      //operations::DynamicColor(
      //  fb_fscrunched, wid, chanout, fcl.nbits
      //);
      tr[3] = axfreq_vec.front(); 
      tr[0] = axtime_vec.front(); 
      tr[2] = (float)fcl.tsamp/1E6;
      tr[4] = 1.f*(float)fcl.foff * fcl.nchans / chanout; 
      _plot();
    }
    ~FBDPlot() {
      _cleanup();
    }
  private:
    void _cleanup() {
      std::fill(tr, tr + 6, 0.0f);
      _safe_delete (fb_fscrunched);
      _safe_delete (fb_tshape); _safe_delete (fb_fshape);
      _safe_delete (fb_dd);
    }
    void ddcpgimag(PtrFloat img, timeslice dim1, timeslice dim2,
      timeslice idim1, timeslice jdim1, timeslice idim2, timeslice jdim2,
      timeslice a, timeslice b,
      PtrFloat t
    ) {
      float ii,jj;
      for(;idim1 < jdim1; idim1++)
        for(;idim2 < jdim2; idim2++) {
            ii = t[0] + (idim1*t[1]) + (idim2*t[2]);
            jj = t[3] + (idim1*t[4]) + (idim2*t[5]);
            std::cout << ii << "," << jj << std::endl;
        }
    }
    void _plot() {
      if(count++) {
        cpgpage(); 
      }
      csize = 6;
      float light[] = {0.0f, 0.2f, 0.4f, 0.6f, 0.8f, 1.0f};
      float red[]   = {0.0f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f};
      float green[] = {0.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f};
      float blue[]  = {0.0f, 0.5f, 0.0f, 0.0f, 0.3f, 1.0f};
      // dedispersed waterfall
      cpgsvp(0.1, 0.65, 0.1, 0.65); // de-dispersed waterfall
      cpgswin(axtime_vec.front(), tmax, axfreq_vec.front(), axfreq_vec.back());
      cpgbox("BCN",0.0,0,"BCNV",0.0,0);
      cpgsfs(1);
      cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), heat_l.size(), contrast, brightness);
      cpgimag (
          fb_fscrunched, chanout, wid, 
          1, chanout, 1, wid,
          -1, csize -1, tr
          );
      cpgmtxt("B",2.5,.5,0.5,"Time (s)"); 
      cpgmtxt("L",4,0.5,0.5,"Freq (MHz)");
      // bandshape
      cpgsci(1); // color index
      cpgsvp(0.65, 0.90, 0.1, 0.65); // bandshape
      min = *std::min_element(fb_fshape, fb_fshape + chanout);
      max = *std::max_element(fb_fshape, fb_fshape + chanout);
      dd_range = max - min;
      xxmin = min - .1 * dd_range;
      xxmax = max + .1 * dd_range;
      cpgswin(xxmin, xxmax, axfreq_vec.front(),  axfreq_vec.back());
      cpgbox("BCN",0.0,0,"BCV",0.0,0);
      cpgsfs(1);
      cpgline(chanout, fb_fshape, reinterpret_cast<float*>(axfreq_vec.data()));
      cpgmtxt("B",2.5,.5,0.5,"Intensity (a.u.)"); 
      cpgmtxt("T",-1*charh,.5,0.5, "Bandshape");
      // profile
      cpgsci(1); // color index
      cpgsvp(0.1, 0.65, 0.65, 0.9); // fscrunched profile 
      min = *std::min_element(fb_tshape, fb_tshape + wid);
      max = *std::max_element(fb_tshape, fb_tshape + wid);
      dd_range = max - min;
      xxmin = min - .1 * dd_range;
      xxmax = max + .1 * dd_range;
      cpgswin(axtime_vec.front(), tmax, xxmin, xxmax );
      cpgbox("BC",0.0,0,"BCNV",0.0,0);
      cpgline(wid, axtime_vec.data(), fb_tshape); 
      cpgmtxt("R",1.2,0.5,0.5,"Intensity (a.u.)");
      cpgmtxt("T",.3,.5,0.5, "De-Dispersed Integrated Profile and Waterfall");
      cpgmtxt("T",2.0,0.0,0.5,group.c_str());
      // text
      cpgsci(1); // color index
      cpgsvp(0.65, 0.90, 0.65, 0.9); // Meta data
      txtrow = 0;
      snprintf(txt, 256, "S/N: %3.2f", sn);
      cpgmtxt("T",txtheight * txtrow++, 0.12, 0.0, txt);
      snprintf(txt, 256, "DM: %3.2f pc/cc", dm);
      cpgmtxt("T",txtheight * txtrow++, 0.12, 0.0, txt);
      snprintf(txt, 256, "Width: %3.2f ms", width);
      cpgmtxt("T",txtheight * txtrow++, 0.12, 0.0, txt);
      snprintf(txt, 256, "Peak Time: %4.3f s", peak_time);
      cpgmtxt("T",txtheight * txtrow++, 0.12, 0.0, txt);
      snprintf(txt, 256, "Antenna: ea%02d", stationid);
      cpgmtxt("T",txtheight * txtrow++, 0.12, 0.0, txt);
      snprintf(txt, 256, "Source: %s", name.c_str());
      cpgmtxt("T",txtheight * txtrow++, 0.12, 0.0, txt);
      snprintf(txt, 256, "Tstart(MJD): %3.2f", tstart);
      cpgmtxt("T",txtheight * txtrow++, 0.12, 0.0, txt);
      snprintf(txt, 256, "NBits: %d", nbits);
      cpgmtxt("T",txtheight * txtrow++, 0.12, 0.0, txt);
      snprintf(txt, 256, "NChans: %d", nchans);
      cpgmtxt("T",txtheight * txtrow++, 0.12, 0.0, txt);
    }


};
#endif
