#pragma once
#include <asgard.hpp>
#include <numeric> // iota
// Filterbank reading
#include "Filterbank.hpp"
// Crawling
#include "Analyzer.hpp"
// Plotting
#include "Plotter.hpp"
#include "Operations.hpp"
// MPI
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/operations.hpp>
namespace mpi = boost::mpi;

class MPIPlot : protected Plotter {
 private:
	int root;
	mpi::environment env;
	mpi::communicator world;
	std::string rootpath;
	// filenames
	PathList filpathlist;
	StringVector filstringlist, All_hostnames;
	std::vector<StringVector> All_filpathlist;
	// reading
	FilterbankReader fbr;
	// data array
	std::vector<PtrFloat> inptrs, outptrs;
	std::vector<FloatVector> plot_f;
	// antenna array
	StringVector antenna_array;
	// root arrays
	std::vector<std::vector<FloatVector>> o_plot_f;
	std::vector<StringVector> o_antenna_array;
	std::vector<std::string> flat_antenna_array;
	std::vector<unsigned int> sort_antenna;
	// plotting variables
	timeslice wid, wid_plot, i0, numelems, numelems_plot;
	unsigned int nchans, nbits, nchans_plot, nant, hant;
	unsigned int bmin, bmax;
	uint64_t nsteps, istep;
	float l_maxdur,maxdur, mintstart, tstart, tsamp, foff;
	FloatVector duration_list, tstart_list;
	char idx_string[16];
	////////////////////////////////////////////////////
	void plotter(const std::string& group, bool isKur, float timestep) {
	 // name resolution
	 AnalyzeFB afb;
	 afb.Crawl(rootpath);
	 if(isKur) filpathlist = afb.kfils[group];
	 else filpathlist = afb.fils[group];
	 for(auto& fpl : filpathlist) filstringlist.push_back(fpl.string());
	 // root debugging
	 mpi::gather(world, filstringlist , All_filpathlist, root);
	 mpi::gather(world, env.processor_name(), All_hostnames, root);
	 // printing debug info
	 if(world.rank() == root) {
		for(int iii = 0; iii !=  All_filpathlist.size(); iii++)
		 for(auto& Sv : All_filpathlist[iii]) {
			std::cout << " I " << Sv << " @ " << All_hostnames[iii];
			if(All_hostnames[iii] == env.processor_name()) std::cout << "*";
			std::cout << std::endl;
		 }
	 }
	 // read filterbanks
	 FilterbankList f;
	 for(auto& xpl : filpathlist) {
		// read filterbanks into f
		Filterbank ffb;
		fbr.Read(ffb, xpl.string());
		f.push_back(ffb);
		// keep times
		duration_list.push_back(ffb.duration);
		antenna_array.push_back(ffb.antenna);
		// initializing data pointers
		inptrs.push_back(nullptr);
		outptrs.push_back(nullptr);
	 }
	 assert(f.size() != 0);
	 assert(f.size() == inptrs.size());
	 assert(f.size() == outptrs.size());
	 // setup
	 tsamp = f[0].tsamp;
	 nchans = f[0].nchans;
	 isKur = f[0].isKur;
	 foff  = f[0].foff;
	 bmin = f[0].bmin;
	 bmax = f[0].bmax;
	 // axis
	 float axis[] = {0.0f, 2.0f, 320.f, 360.f};
	 // figure out local 
	 l_maxdur = *std::max_element(duration_list.begin(), duration_list.end());
	 // compute duration
	 mpi::all_reduce(world, l_maxdur, maxdur, mpi::maximum<float>());
	 if(world.rank() == root) {
		std::cout << "Maximum duration: " << maxdur << " (s)"<< std::endl;
	 }
	 // figure out tstep(width)
	 wid = timestep / tsamp;				
	 numelems = wid * nchans;
	 nsteps = maxdur / timestep;
	 numelems_plot = wid_plot * nchans_plot;
	 // allocate memory
	 for(int i_ = 0; i_ < f.size(); i_++) {
		inptrs[i_] = new float[numelems];
		outptrs[i_] = new float[numelems_plot];
	 }
	 //// tr set in root
	 if(world.rank() == root) {
		auto ft = operations::FreqTable(f[0]);
		tr[3] = ft.front();
		tr[2] = (float)tsamp * wid / wid_plot;
		tr[4] = 1.f*(float)foff * nchans / nchans_plot;
		tr[3] -= 0.5*tr[4];
		axis[2] = ft.back();
		axis[3] = ft.front();
	 }
	 // gather antennas in root
	 mpi::gather(world, antenna_array, o_antenna_array, root);
	 // flatten it in root
	 if(world.rank() == root) {
		assert(o_antenna_array.size() == world.size());
		// flatting gathered
		for(int i_ = 0; i_ < world.size(); i_++) {
		 std::copy(o_antenna_array[i_].begin(), o_antenna_array[i_].end(), back_inserter(flat_antenna_array));
		}
		nant = flat_antenna_array.size();
		hant = nant / 2;
		sort_antenna.resize(nant);
		// sort antenna -- can't believe STL doesn't have an argsort 
		// https://stackoverflow.com/questions/10580982/c-sort-keeping-track-of-indices
		std::iota(sort_antenna.begin(), sort_antenna.end(), 0);
		std::sort(sort_antenna.begin(), sort_antenna.end(), 
		 [&](unsigned int a, unsigned int b){ return flat_antenna_array[a] < flat_antenna_array[b];});
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
		for(int i_ = 0; i_ < f.size(); i_++) {
		 f[i_].Unpack(inptrs[i_], i0, wid);
		 operations::Crunch(inptrs[i_], nchans, wid, nchans_plot, wid_plot, outptrs[i_]);
		 plot_f.emplace_back(outptrs[i_], outptrs[i_] + numelems_plot);
		}
		mpi::gather(world, plot_f, o_plot_f, root);
		// clear plot_f for future use
		plot_f.clear();
		///////// PLOTTING
		//// in root
		if(world.rank() == root) {
		 // flat 
		 std::vector<FloatVector> flat_plot_f;
		 for(int i_ = 0; i_ < world.size(); i_++) {
			std::copy(o_plot_f[i_].begin(), o_plot_f[i_].end(), back_inserter(flat_plot_f));
		 }
		 // clear o_plot_f for future use
		 o_plot_f.clear();
		 // begin
		 if(count != 0) cpgpage();
		 w = (0.88f/nant) - 0.02f;
		 xmin = 0.1;
		 ymin = 0.05;
		 xmax = 0.9;
		 ymax = w + ymin;
		 for(auto iant : sort_antenna) {
			// axis variables
			axis[0] = istep * timestep;
			axis[1] = axis[0] + timestep;
			cpgsci(1); // color index
			cpgsvp(xmin, xmax, ymin, ymax);
			cpgswin(axis[0],axis[1],axis[2],axis[3]);
			if(iant == sort_antenna.front()) {
			 cpgmtxt("B",2,.5,0.5,"Time (s)"); 
			 cpgbox("BCN",0.0,0,"BC",40.0,0);
			 cpgmtxt("LV",3,0.2,0.0,std::to_string((int)axis[2]).c_str());
			 cpgmtxt("LV",3,0.8,0.0,std::to_string((int)axis[3]).c_str());
			}
			else if( iant == sort_antenna[hant]) {
			 cpgbox("BC",0.0,0,"BC",40.0,0);
			 cpgmtxt("L",4,0.,0.5,"Freq (MHz)");
			 cpgmtxt("LV",3,0.2,0.0,std::to_string((int)axis[2]).c_str());
			 cpgmtxt("LV",3,0.8,0.0,std::to_string((int)axis[3]).c_str());
			}
			else if(iant == sort_antenna.back()) {
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
			cpgimag(flat_plot_f[iant].data(), nchans_plot, wid_plot, 1, nchans_plot, 1, wid_plot, bmin, bmax, tr);
			cpgmtxt("RV",2,.5,0.5,flat_antenna_array[iant].c_str());
			ymin = ymax + 0.02;
			ymax += w   + 0.02 ;
		 } // for each antenna
		 count++;
		} // plot in root
		istep++;
	 } // for each timestep
	 // free up memory
	 for(int i_ = 0; i_ < f.size(); i_++) {
		delete[] inptrs[i_];
		delete[] outptrs[i_];
	 }
	} // end of plotter
 public:
	MPIPlot(std::string fn, std::string _rootpath, int _root) : 
	 rootpath(_rootpath), root(_root), Plotter(fn) {}
	void Plot(const std::string& group, bool isKur, float timestep) {
	 wid_plot = 1024;
	 nchans_plot = 512;
	 plotter(group, isKur, timestep);
	}
};
