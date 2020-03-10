#pragma once
#include <asgard.hpp>
#include <exception>
// trigger DADA
#include "TrigDADA.hpp"
// Header stuff
#include "Header.hpp"
// de-disperser
#include "BTDD.hpp"
// dbson-plot
#include "TriggerJSON.hpp"
#include "TriggerPlot.hpp"
// timer
#include "Timer.hpp"
// fbson read
#include "FilterbankJSON.hpp"


// There are six functions
// to be performed for every trigger
// slicerAndmerger <-- {bter,incoher} <-- {dumper,plotter}
//                      ^
//                      |
//                      \------------{mler}
// Using BTDD for rapid action
// slicerAndmerger <-- btdder <-- {dumper,plotter}
//                      ^
//                      |
//                      \------------{mler}
// XXX bter   crucially changes trigHead_t
// XXX btdder crucially changes trigHead_t
class TriggerMaster {
	using Byte = unsigned char;
	using vf   = std::vector<float>;
	using vb   = std::vector<Byte>;
	private:
		// dada interface
		key_t       hkey;
		key_t       dkey;
		TrigDADA    td;
		// dump-plot
		TriggerJSON tj;
		TriggerPlot tp;
		// data sizes
		unsigned nsamps;
    unsigned dm_count;
    unsigned nchans;
		unsigned maxdelay;
		unsigned readnsamps;
		timeslice datasamps;
		timeslice reqsamps;
		timeslice maxdatasamps;
    // data 
		vb  bdata;
		vb  bt;
		vb  incoh;
		vf  btf;
		vf  incohf;
		// headers
		Header_t  head;
		trigger_t trig;
		trigHead_t  th;
    // slicing logic
    timeslice start;
    timeslice stop;
    float cut;
		// bter
		// this makes dmwidth 50
		static constexpr float h_dmwidth = 25.0f;
		void btdder () {
		  // containers
		  vf mbt;
      vb mdd;
      // actual algo
			BTDD<Byte> btdd (head.tsamp, head.nchans, head.fch1, head.foff, 2.0*h_dmwidth, dm_count);
			btdd.SetDM (trig.dm);
			btdd.Execute (bdata, readnsamps, mbt, mdd);
			// BT
			// --------
			// slicing and linear coding
			timeslice nbt = std::max (static_cast<int>(readnsamps-btdd.btnsamps), 0);
			{
        float mmin = std::numeric_limits<float>::max();
        float mmax = std::numeric_limits<float>::min();
        for (unsigned idm = 0; idm < dm_count; idm++) {
          for (timeslice ii = start; ii < stop; ii++) {
            auto ss = mbt [ii + nbt*idm];
            if (ss > mmax) mmax = ss;
            if (ss < mmin) mmin = ss;
          }
        }
        float imm  = mmax == mmin ? 1.0f : 1.0f / (mmax - mmin);
        Byte pp;
        for (unsigned idm = 0; idm < dm_count; idm++) {
          for (timeslice ii = start; ii < stop; ii++) {
            auto ss = mbt [ii + nbt*idm];
            pp = static_cast<Byte>(255 * imm * (ss - mmin));
            bt.push_back (pp);
            btf.push_back (static_cast<float>(pp));
          }
        }
			}
			// dump bt
			tj.DumpBT (bt, trig.dm - h_dmwidth, 2.0*h_dmwidth/(dm_count-1));
			// XXX some re-arrangement would make this
			// more transparent
			th.dm1  = trig.dm - h_dmwidth;
			th.dmoff = 2.0f * h_dmwidth / (dm_count-1);
			// DD
			// --------
			// slicing and linear coding
			{
        timeslice istart = start * head.nchans;
        timeslice istop  = stop  * head.nchans;
        // fscrunch
        unsigned fac = head.nchans / nchans;
        float ifac = 1.0f/ fac;
        float xf = 0.0f;
        Byte  pp;
        for (timeslice ii = istart; ii < istop; ii+=fac) {
          xf = 0.0f;
          for (unsigned ik = 0; ik < fac; ik++) {
            xf += mdd[ii + ik];
          }
          incohf.push_back ( xf * ifac );
        }
        // linear coding
        auto mmin = std::numeric_limits<Byte>::max();
        auto mmax = std::numeric_limits<Byte>::min();
        for (const auto& ss : incohf) {
          if (ss > mmax) mmax = ss;
          if (ss < mmin) mmin = ss;
        }
        float imm  = mmax == mmin ? 1.0f : 1.0f / (mmax - mmin);
        for (auto& ss : incohf) {
          ss = imm * (ss - mmin) * 255;
          incoh.push_back (static_cast<Byte>(ss)); 
        }
			}
			// dump dd
			tj.DumpDD (incoh);
		}
		void bter () {
		  vf mbt;
			FDMT<Byte> mybt (head.tsamp/1E6, head.nchans, head.fch1, -head.foff);
			mybt.SetDM (trig.dm - h_dmwidth, trig.dm + h_dmwidth, dm_count);
			mybt.Execute (bdata, readnsamps, mbt);
			maxdelay = mybt.MaxSampDelay ();
			timeslice nbt = std::max (static_cast<int>(readnsamps-maxdelay), 0);
			// slicing and linear coding
			float mmin = std::numeric_limits<float>::max();
			float mmax = std::numeric_limits<float>::min();
			for (unsigned idm = 0; idm < dm_count; idm++) {
			  for (timeslice ii = start; ii < stop; ii++) {
			    auto ss = mbt [ii + nbt*idm];
			    if (ss > mmax) mmax = ss;
			    if (ss < mmin) mmin = ss;
			  }
			}
			float imm  = mmax == mmin ? 1.0f : 1.0f / (mmax - mmin);
			Byte pp;
			for (unsigned idm = 0; idm < dm_count; idm++) {
			  for (timeslice ii = start; ii < stop; ii++) {
			    auto ss = mbt [ii + nbt*idm];
			    pp = static_cast<Byte>(255 * imm * (ss - mmin));
			    bt.push_back (pp);
			    btf.push_back (static_cast<float>(pp));
			  }
			}
			// dump bt
			const auto& dml = mybt.dm_list;
			tj.DumpBT (bt, dml.front(), dml[1] - dml[0]);
			// XXX some re-arrangement would make this
			// more transparent
			th.dm1  = dml.front ();
			th.dmoff = dml[1] - dml[0];
		}
		void incoher () {
		  vb mincoh;
			Incoherent<Byte> myincoh (head.tsamp/1E6, head.nchans, head.fch1, head.foff);
			myincoh.SetDM (trig.dm);
			auto lmaxdelay = myincoh.MaxSampDelay ();
			auto lnsamps   = std::max (0, static_cast<int>(readnsamps-lmaxdelay));
			myincoh.Execute (bdata, lnsamps, mincoh);
			// slicing and fscrunching
      timeslice istart = start * head.nchans;
      timeslice istop  = stop  * head.nchans;
      // fscrunch
      unsigned fac = head.nchans / nchans;
      float ifac = 1.0f/ fac;
      float xf = 0.0f;
      Byte  pp;
			for (timeslice ii = istart; ii < istop; ii+=fac) {
				xf = 0.0f;
				for (unsigned ik = 0; ik < fac; ik++) {
					xf += mincoh[ii + ik];
				}
        incohf.push_back ( xf * ifac );
			}
			// linear coding
			auto mmin = std::numeric_limits<Byte>::max();
			auto mmax = std::numeric_limits<Byte>::min();
			for (const auto& ss : incohf) {
			  if (ss > mmax) mmax = ss;
			  if (ss < mmin) mmin = ss;
			}
			float imm  = mmax == mmin ? 1.0f : 1.0f / (mmax - mmin);
			for (auto& ss : incohf) {
			  ss = imm * (ss - mmin) * 255;
			  incoh.push_back (static_cast<Byte>(ss)); 
			}
			// dump dd
			tj.DumpDD (incoh);
		}
		void dumper () {
			tj.DumpHead (th);
			tj.WritePayload ();
		}
		void plotter () {
			tp.Plot (th, btf.data(), incohf.data());
		}
		void slicerAndmerger () {
		  // clear house
		  incoh.clear ();
		  incohf.clear ();
		  bt.clear ();
		  btf.clear ();
		  // slicing logic
      timeslice ipt = trig.peak_time / head.tsamp * 1E6;
		  start = std::max (0, static_cast<int>(ipt - (0.5 * nsamps)));
		  stop  = start + nsamps;
		  cut   = start * head.tsamp / 1E6;
		  // merging logic
		  // header copy
		  th.stationid = head.stationid;
		  th.ra        = head.ra;
		  th.dec       = head.dec;
		  th.fch1      = head.fch1;
		  th.foff      = head.foff * head.nchans / nchans;
		  th.tsamp     = head.tsamp;
		  th.tstart    = head.tstart;
		  th.epoch     = head.epoch;
		  th.nbits     = head.nbits;
		  // strs
		  strcpy (th.name, head.name);
		  strcpy (th.sigproc_file, head.sigproc_file);
		  // those three
		  th.nchans    = nchans;
		  th.ndm       = dm_count;
		  th.nsamps    = nsamps;
		  // trigger copy
		  th.sn    = trig.sn;
		  th.dm    = trig.dm;
		  th.width = trig.width;
		  // adjustments
		  float duration   = nsamps * head.tsamp / 1E6;
		  th.peak_time = trig.peak_time - cut;
		  th.i0    = trig.i0 + cut;
		  th.i1    = trig.i0 + cut + duration;
		  th.dur   = duration;
		}
		void mler () {
		  // this is where my ml action
		  // my telemetry
		}
	public:
		TriggerMaster (
		  // keys
		  key_t hkey_,
		  key_t dkey_,
		  // paths
      std::string ddir, std::string pdir,
      // maxdata samps
      timeslice maxds,
      // those three
      unsigned dmc = 256,
      unsigned nmc = 256,
      unsigned cmc = 64
		) : 
		hkey (hkey_), dkey(dkey_), 
		maxdatasamps(maxds),
		dm_count (dmc), nsamps(nmc), nchans(cmc),
		td (hkey, dkey), 
		// vv - DM_COUNT, NSAMPS, NCHANS - vv
		tj(ddir), tp (pdir) {
			bdata.resize (maxdatasamps, 0);
			bt.reserve (dm_count * nsamps);
			btf.reserve (dm_count * nsamps);
			incoh.reserve (nsamps * nchans);
			incohf.reserve (nsamps * nchans);
		}
		// for file reading
		TriggerMaster (
		  // paths
      std::string ddir, std::string pdir,
      // maxdata samps
      timeslice maxds,
      // those three
      unsigned dmc = 256,
      unsigned nmc = 256,
      unsigned cmc = 64
		) : 
		maxdatasamps(maxds),
		dm_count (dmc), nsamps(nmc), nchans(cmc),
		// vv - DM_COUNT, NSAMPS, NCHANS - vv
		tj(ddir), tp (pdir) {
			bdata.resize (maxdatasamps, 0);
			bt.reserve (dm_count * nsamps);
			btf.reserve (dm_count * nsamps);
			incoh.reserve (nsamps * nchans);
			incohf.reserve (nsamps * nchans);
		}
		// main
		void FollowDADA () {
      while (true) {
        td.ReadLock (true);
        // blocking code vvv
        td.ReadHeader (head, trig);
        // blocking code ^^^
        reqsamps  = std::ceil (trig.i1 - trig.i0) / head.tsamp * 1E6;
        reqsamps  *= head.nchans * head.nbits / 8;
        datasamps = td.ReadData (bdata.data(), reqsamps);
        if (datasamps == -1) {
          // this is weird
          std::cout << "TriggerMaster::FollowDADA read EOD" << std::endl;
          if (head.nchans == 0 && head.nbits == 0) {
            // this is hella wrong
            std::cout << "TriggerMaster::FollowDADA shouldn't have read EOD!!!!" << std::endl;
          }
        }
        else {
          readnsamps = datasamps / head.nchans / head.nbits * 8;
          // wait for them to complete
          Singleton ();
          // clear the header structs
          head = {};
          trig = {};
        }
        td.ReadLock (false);
      }
		}
		void Singleton () {
		  using std::cout;
		  Timer t("");
		  t.Start ();
		  slicerAndmerger ();
		  t.StopPrint (cout << "SlicerAndMerger");
		  t.Start ();
		  btdder ();
		  t.StopPrint (cout << "BTDD");
		  t.Start ();
		  dumper ();
		  t.StopPrint (cout << "DumpDBSON");
		  t.Start ();
		  plotter ();
		  t.StopPrint (cout << "PlotDBSON");
		  t.Start ();
		  plotter ();
		  mler ();
		  t.StopPrint (cout << "MLer");
		}
		void FollowFile (const std::string& ss) {
      FBDump fbson (ss);
      head = dynamic_cast<Header_t&>(fbson);
      trig = dynamic_cast<trigger_t&>(fbson);
      // tpeak mess up
      if (fbson.tpeak >= 0.0f && fbson.tpeak <= 1.0f) {
        std::cout << "TriggerMaster::FollowFile peak_time adjusted" << std::endl;
        trig.peak_time = fbson.tpeak;
      }
      //
      reqsamps  = std::ceil (trig.i1 - trig.i0) / head.tsamp * 1E6;
      reqsamps  *= head.nchans * head.nbits / 8;
      std::copy (fbson.fb.begin(), fbson.fb.end(), bdata.begin());
      datasamps = fbson.fb.size();
      if (datasamps == -1) {
        // this is weird
        std::cout << "TriggerMaster::FollowFile read error" << std::endl;
        if (head.nchans == 0 && head.nbits == 0) {
          // this is hella wrong
          std::cout << "TriggerMaster::FollowFile shouldn't have read EOD!!!!" << std::endl;
        }
      }
      else {
        readnsamps = datasamps / head.nchans / head.nbits * 8;
        // wait for them to complete
        Singleton ();
        // clear the header structs
        head = {};
        trig = {};
      }
		}
};
//TriggerMaster::h_dmwidth = 25.0f;
