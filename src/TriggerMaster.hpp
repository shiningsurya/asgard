#pragma once
#include <asgard.hpp>
#include <exception>
// trigger DADA
#include "TrigDADA.hpp"
// Header stuff
#include "Header.hpp"
// de-disperser
#include "FDMT.hpp"
#include "Incoherent.hpp"
// dbson-plot
#include "TriggerJSON.hpp"
#include "TriggerPlot.hpp"
// timer
#include "Timer.hpp"

DUMP_DIR


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
		// headers
		Header_t  head;
		trigger_t trig;
		// data
		timeslice datasamps;
		timeslice maxdatasamps;
		unsigned nsamps;
		unsigned maxdelay;
		vb  bdata;
		vf  bt;
		vb  incoh;
		// bter
		void bter () {
			FDMT<Byte> mybt (head.tsamp/1E6, head.nchans, head.fch1, -head.foff);
			mybt.SetDM (trig.dm - h_dmwidth, trig.dm + h_dmwidth, dm_count);
			mybt.Execute (bdata, nsamps, bt);
			maxdelay = mybt.MaxSampDelay ();
			// dump bt
			const auto& dml = mybt.dm_list;
			tj.DumpBT (bt, dcnsamps, dml.front(), dml[1] - dml[0]);
		}
		void incoher () {
			Incoherent<Byte> myincoh (head.tsamp/1E6, head.nchans, head.fch1, head.foff);
			myincoh.SetDM (trig.dm);
			myincoh.Execute (bdata, nsamps, incoh);
			// dump dd
			tj.DumpDD (incoh, nsamps, x.nchans);
		}
		void dumper () {
			tj.DumpHead (h, t);
			tj.WritePayload ();
			tj.Clear ();
		}
		void plotter () {
			tp.Plot (trighead, bt.data(), incoh.data());
		}

	public:
		TriggerMaster () : hkey (hkey_), dkey(dkey_), 
		td (hkey, dkey), tj(dump_dir), tp (plot_dir) {
			bdata.resize (maxdatasamps, 0);
		}
		// main
		void FollowDADA () {
			td.ReadLock (true);
			// false
			while (false || td.ReadHeader (head, trig)) {
				datasamps = td.ReadData (bdata.data(), maxdatasamps);
				// wait for them to complete
			}
			td.ReadLock (false);
		}
};
