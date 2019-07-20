#pragma once
#include "asgard.hpp"
// Filterbank/Candidate reading
#include "Filterbank.hpp"
#include "Candidate.hpp"
// Crawling
#include "Analyzer.hpp"
// Plotting
#include "Plotter.hpp"
#include "Operations.hpp"
// MPI
#include "MPIBase.hpp"

class MPIWaterfallCandidate : protected Plotter, MPIBase {
 private:
 	// reading
 	FilterbankReader fbr;
 	// data arrays
	std::vector<PtrFloat> inptrs, outptrs;
	std::vector<FloatVector> plot_f, o_plot_f;
	// antenna array
	StringVector antenna_array, o_antenna_array;
	// plotting variables
	timeslice wid, wid_plot, i0, numelems, numelems_plot;
	unsigned int nchans, nbits, nchans_plot, nant, hant;
	unsigned int bmin, bmax;
	uint64_t nsteps, istep;
	float l_maxdur,maxdur, mintstart, tstart, tsamp, foff;
	FloatVector duration_list, tstart_list;
	char idx_string[16], txt[8];
	float axis[4];
	float xlin[2], ylin[2];
	////////////////////////////////////////////////////
	void plotter(const std::string& group, bool isKur, float timestep) {
	 // name resolution
	 PathList filpathlist = CrawlFils(group, isKur);
	 PathList candpathlist = CrawlCands(group, isKur);
	 assert(filpathlist.size() != 0);
	 StringVector filstringlist, candstringlist;
	 std::for_each(filpathlist.begin(), filpathlist.end(), 
		 [&filstringlist](fs::path _fp) {filstringlist.push_back(_fp.string());}
		 );
	 std::for_each(candpathlist.begin(), candpathlist.end(), 
		 [&candstringlist](fs::path _fp) {candstringlist.push_back(_fp.string());}
		 );
	 // print filenames, hostnames in root
	 gatherDataHostname(filstringlist);
	 gatherDataHostname(candstringlist);
	 // reading
	 FilterbankList flist;
	 for(auto& xpl : filpathlist) {
		// read filterbanks into f
		Filterbank ffb;
		fbr.Read(ffb, xpl.string());
		flist.push_back(ffb);
		// keep times
		duration_list.push_back(ffb.duration);
		antenna_array.push_back(ffb.antenna);
		// initializing data pointers
		inptrs.push_back(nullptr);
		outptrs.push_back(nullptr);
	 }
	 // setup
	 tsamp = flist[0].tsamp;
	 nchans = flist[0].nchans;
	 isKur = flist[0].isKur;
	 foff  = flist[0].foff;
	 bmin = flist[0].bmin;
	 bmax = flist[0].bmax;
	 // candidate logic
	 std::vector<FloatVector> calist_sn, calist_dm, calist_pt;
	 StringVector calist_ant;
	 for(auto& xpl : candstringlist) {
	 	FloatVector _sn, _dm, _pt;
		auto c_list = ReadCandidates( xpl, 0.0f);
		std::for_each(c_list.begin(), c_list.end(), 
			[&_dm, &_sn, &_pt](const Candidate& cc)
			{
			 _sn.push_back(cc.sn);
			 _dm.push_back(cc.dm);
			 _pt.push_back(cc.peak_time);
			}
			);
		calist_sn.push_back(std::move(_sn));
		calist_dm.push_back(std::move(_dm));
		calist_pt.push_back(std::move(_pt));
		calist_ant.push_back(c_list[0].antenna);
	 }
	 auto o_calist_sn = gatherPayload(calist_sn);
	 auto o_calist_dm = gatherPayload(calist_dm);
	 auto o_calist_pt = gatherPayload(calist_pt);
	 auto o_calist_ant = gatherPayload(calist_ant);
	 assert(o_calist_sn.size() == o_calist_dm.size());
	 assert(o_calist_pt.size() == o_calist_dm.size());
	 // antenna logic
	 o_antenna_array = gatherPayload(antenna_array);
	 if(isRoot()) {
		strider(o_antenna_array);
		nant = o_antenna_array.size();
		hant = nant / 2;
	 }
	 // Plotting
	 // figure out local 
	 l_maxdur = *std::max_element(duration_list.begin(), duration_list.end());
	 // compute duration
	 mpi::all_reduce(world, l_maxdur, maxdur, mpi::maximum<float>());
	 // figure out tstep(width)
	 wid = timestep / tsamp;				
	 numelems = wid * nchans;
	 nsteps = maxdur / timestep + 1;
	 numelems_plot = wid_plot * nchans_plot;
	 // allocate memory
	 for(int i_ = 0; i_ < flist.size(); i_++) {
		inptrs[i_] = new float[numelems];
		outptrs[i_] = new float[numelems_plot];
	 }
	 // tr set in root
	 if(isRoot()) {
		auto ft = operations::FreqTable(flist[0]);
		tr[3] = ft.front();
		tr[2] = (float)tsamp * wid / wid_plot;
		tr[4] = 1.f*(float)foff * nchans / nchans_plot;
		tr[3] -= 0.5*tr[4];
		axis[2] = ft.back();
		axis[3] = ft.front();
	 }
	 /////////////////////////////
	 istep = 0;
	 while(istep != nsteps) {
		// broadcast loop variable
		mpi::broadcast(world, istep, root);
		// extract filterbankdata
		i0 = istep * timestep / tsamp;
		// broadcast i0 variable
		mpi::broadcast(world, i0, root);
		for(int i_ = 0; i_ < flist.size(); i_++) {
		 flist[i_].Unpack(inptrs[i_], i0, wid);
		 operations::Crunch(inptrs[i_], nchans, wid, nchans_plot, wid_plot, outptrs[i_]);
		 plot_f.emplace_back(outptrs[i_], outptrs[i_] + numelems_plot);
		}
		o_plot_f = gatherPayload(plot_f);
		// clear plot_f for future use
		plot_f.clear();
		///////// PLOTTING
		if(isRoot()) {
		 // begin
		 if(count != 0) cpgpage();
		 w = (0.88f/nant) - 0.02f;
		 xmin = 0.1;
		 ymin = 0.05;
		 xmax = 0.9;
		 ymax = w + ymin;
		 for(auto iant : stride) {
			// axis variables
			axis[0] = istep * timestep;
			axis[1] = axis[0] + timestep;
			cpgsci(1); // color index
			cpgsvp(xmin, xmax, ymin, ymax);
			cpgswin(axis[0],axis[1],axis[2],axis[3]);
			if(iant == stride.front()) {
			 cpgmtxt("B",2,.5,0.5,"Time (s)"); 
			 cpgbox("BCN",0.0,0,"BC",40.0,0);
			 cpgmtxt("LV",3,0.2,0.0,std::to_string((int)axis[2]).c_str());
			 cpgmtxt("LV",3,0.8,0.0,std::to_string((int)axis[3]).c_str());
			}
			else if( iant == stride[hant]) {
			 cpgbox("BC",0.0,0,"BC",40.0,0);
			 cpgmtxt("L",4,0.,0.5,"Freq (MHz)");
			 cpgmtxt("LV",3,0.2,0.0,std::to_string((int)axis[2]).c_str());
			 cpgmtxt("LV",3,0.8,0.0,std::to_string((int)axis[3]).c_str());
			}
			else if(iant == stride.back()) {
			 cpgbox("BC",0.0,0,"BC",40.0,0);
			 cpgmtxt("T",1,.5,0.5,group.c_str()); // group at middle
			 if(isKur) cpgmtxt("T",1,.8,0.5,"KUR"); // group at middle
			 else cpgmtxt("T",1,.8,0.5,"NOKUR");
			 cpgmtxt("LV",3,0.2,0.0,std::to_string((int)axis[2]).c_str());
			 cpgmtxt("LV",3,0.8,0.0,std::to_string((int)axis[3]).c_str());
			 cpgbox("BC",0.0,0,"BC",40.0,0);
			 snprintf(idx_string,16, "Slice: %lu/%lu", istep+1, nsteps);
			 cpgmtxt("T",.5,.0,0.0, idx_string);   // slice index
			}
			else {
			 cpgbox("BC",0.0,0,"BC",40.0,0);
			 cpgmtxt("LV",3,0.2,0.0,std::to_string((int)axis[2]).c_str());
			 cpgmtxt("LV",3,0.8,0.0,std::to_string((int)axis[3]).c_str());
			}
			cpgsfs(1);
			cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
			tr[0] = axis[0] - 0.5 * tr[2];
			cpgimag(o_plot_f[iant].data(), nchans_plot, wid_plot, 1, nchans_plot, 1, wid_plot, bmin, bmax, tr);
			cpgmtxt("RV",2,.5,0.5,o_antenna_array[iant].c_str());
			/////// Candidate logic -- begin
			auto map_iant = std::find(o_calist_ant.begin(), o_calist_ant.end(), o_antenna_array[iant]);
			if(map_iant != o_calist_ant.end()){
			 auto iiant = std::distance(o_calist_ant.begin(), map_iant);
			 for(size_t ii = 0; ii < o_calist_dm[iiant].size(); ii++) {
				float peak_time = o_calist_pt[iiant][ii];
				float dm = o_calist_dm[iiant][ii];
				float sn = o_calist_sn[iiant][ii];
				// selection logic here -- begin
				if(peak_time < axis[0] || peak_time >= axis[1]) continue;
				//if(sn < 10) continue;
				if(dm < 25 || dm > 27) continue;
				// selection logic here -- end
				// this c can be plotted
				float cid = ( peak_time - axis[0] ) / timestep;
				if(cid < 0 || cid > 1.0f) continue;
				// line at start
				xlin[0] = peak_time;
				xlin[1] = peak_time;
				ylin[0] = axis[2];
				ylin[1] = axis[3];
				cpgsci(2);
				cpgline(2,xlin, ylin);
				// DM at top
				cpgsci(3);
				snprintf(txt, 8, "%3.2f", dm);
				cpgmtxt("T", 0.0f, cid, 0.5f, txt);
				// S/N at bottom 
				cpgsci(5);
				snprintf(txt, 8, "%3.2f", sn);
				cpgmtxt("B", 0.0f, cid, 0.5f, txt);
				cpgsci(1);
			 }
			}
			/////// Candidate logic -- end
			ymin = ymax + 0.02;
			ymax += w   + 0.02 ;
		 } // for each antenna
		 count++;
		} // plot in root
		istep++;
		// clear o_plot_f for future use
		o_plot_f.clear();
	 } // for each timestep
	 // free up memory
	 for(int i_ = 0; i_ < flist.size(); i_++) {
		delete[] inptrs[i_];
		delete[] outptrs[i_];
	 }
	}
 public:
	MPIWaterfallCandidate(std::string fn, std::string _rootpath, int _root, bool _verbose)
	 : Plotter(fn), MPIBase(_root, _verbose, _rootpath) {}
	void Plot(const std::string& group, bool isKur, float timestep) {
	 wid_plot = 1024;
	 nchans_plot = 512;
	 plotter(group, isKur, timestep);
	}
};
