#ifndef ASGARD_H
#include "asgard.hpp"
#endif
#ifndef COADD_H
#define COADD_H
#include "Filterbank.hpp"
#include "xRFI.hpp"
#include "Group.hpp"
#include <iomanip>
FilterbankList FLFromPL(PathList& x) {
		FilterbankList ret;
		FilterbankReader fbr;
		for(auto& de : x) {
				Filterbank xx;
				fbr.Read(xx, de.string()); 
				ret.push_back(xx);
		}
		return ret;
		// I would like to take time to tell you that
		// I took 10 mins into debugging why this function was 
		// going haywire.
		// And the reason is self explanatory:
		// I forgot "return ret"
}

class Coadd {
		private:
				//std::string method("MAD");
				bool returnFil, returnDada;
				std::string filename;
				double load_secs;
				float tfac, ffac;
				double mintstart, maxdur, maxtstop;
				timeslice retsize, i0, ddit;
				FilterbankWriter fbw;
				int method;
				float * bandshape, * timeshape;
				FloatVector bins;
				unsigned char * bandflags, * timeflags;
				bool xxinitialized;
				void _work_2(Filterbank& x, Filterbank& y, PtrFloatUnique  ret ) {
						// ret is initialized before
				}
				void _init_xxer(timeslice wid, int nchans) {
						// dynamic array declarations here
						// destroy them (if created) in destroyer
						// or make them unique ptr?
						bandshape = new float[nchans];
						timeshape = new float[wid];
						bandflags = new unsigned char[nchans];
						timeflags = new unsigned char[wid];
						bins = {0.0f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f};
						xxinitialized = true;
				}
				void _xxer(PtrFloat dat, timeslice wid, int nchans) {
						// Sending function pointer?
						// we do this inplace? 
						operations::FreqShape(dat, wid, nchans, bandshape);
						operations::TimeShape(dat, wid, nchans, timeshape);
						if(method == 1) {
								excision::MAD(bandshape, nchans, bandflags, ffac);
								excision::MAD(timeshape, wid, timeflags, tfac);
						}
						else if(method == 2) {
								excision::Histogram(bandshape, nchans, bandflags, bins, ffac);
								excision::Histogram(timeshape, wid, timeflags, bins, tfac);
						}
						// flagging 
						for(timeslice i = 0; i < wid; i++) {
								for(int j = 0; j < nchans; j++) {
										if( bandflags[j] == 'o' && timeflags[i] == 'o' ) {
												dat[i*nchans + j] = 0.0f;
										}
								}
						}
				}
				void _serial(FilterbankList& fl, FilterbankWriter& bw) {
						/**
						 * I am making a heavy assumption:
						 * same tsamp and same nchans in all the filterbanks
						 * TODO Make this general
						 * */
						// serial code
						timeslice tstep = load_secs / fl[0].tsamp;
						timeslice nsteps = maxdur / load_secs;
						timeslice i0 = 0;
						int NumAnt = 0;
						timeslice tdat_size = tstep * fl[0].nchans;
						std::size_t tdat_memsize = tdat_size * sizeof(float);
						PtrFloat tdat = new float[tdat_size](); // uniform initialization
						PtrFloat fbdat = new float[tdat_size](); // uniform initialization
						double currTime = 0.0f;
						for(timeslice i = 0; i < nsteps; i++) {
								NumAnt = 0;
								for(Filterbank& f : fl) {
										// i0 * f.tsamp <-- timestep
										currTime = mintstart + ( i * load_secs / 86400.0f );
										if(currTime < f.tstop && currTime >= f.tstart  ) {  
												i0 = ( currTime - f.tstart ) * 86400.0f / f.tsamp;
												f.Unpack(fbdat, i0, tstep);
												// <--------------------->
												// this is where you put your xrfi logic
												// call here
												if(!xxinitialized && method) _init_xxer(tstep, f.nchans);
												if(method)	_xxer(fbdat, tstep, f.nchans);
												// <--------------------->
												NumAnt++;
#pragma omp parallel for
												for(timeslice j = 0; j < tdat_size; j++) {
														tdat[j] += fbdat[j]; 
														fbdat[j] = 0.0f;
												}
										}
								}
								bw.WriteFBdata(tdat,ddit, tdat_size, NumAnt);
								for(timeslice j = 0; j < tdat_size; j++) tdat[j] = 0.0f;
								ddit += tdat_size/4;
								// * 4 bc 1 byte = 4 samples
						}	
						delete[] tdat;
						delete[] fbdat;
				}
		public:
				Coadd(std::string fn, float ls, float tf, float ff, int met) : 
						tfac(tf), 
						ffac(ff),
						method(met) {
								xxinitialized = false;
								Coadd(fn, ls);
						}
				Coadd(std::string fn, float ls) : 
						filename(fn), 
						load_secs(ls) {
								tfac = 0.0f;
								ffac = 0.0f;
								//
								bandshape = timeshape =  nullptr;
								bandflags = timeflags = nullptr;
								method = 0;
						}
				void flagger(PathList& x) {
						FilterbankList fl; 
						fl = FLFromPL(x);
						flagger( fl );
				}
				void flagger(FilterbankList& fl) {
						// TODO
				}
				void coadd(PathList& x) {
						FilterbankList fl; 
						fl = FLFromPL(x);
						coadd( fl );
				}
				void coadd(FilterbankList& fl) {
						// compute max duration here
						DoubleVector tstop_l, tstart_l;
						std::for_each(fl.begin(), fl.end(), [&tstop_l, &tstart_l](Filterbank& bf) { tstop_l.push_back( bf.tstop); tstart_l.push_back( bf.tstart );  });
						maxtstop = *(std::max_element(tstop_l.begin(), tstop_l.end()));
						mintstart = *(std::min_element(tstart_l.begin(), tstart_l.end()));
						maxdur = 86400.0f * ( maxtstop - mintstart );
						ddit = fbw.Initialize(filename, fl[0], maxdur, mintstart, fl[0].nbits);
						// until writing to filterbank is done in steps
						_serial(fl, fbw);
				}
				~Coadd() {
						if(!bandshape) delete[] bandshape;
						if(!timeshape) delete[] timeshape;
						if(!bandflags) delete[] bandflags;
						if(!timeflags) delete[] timeshape;
				}
};

#ifdef MPI
#include "Group.hpp"
#include "Analyzer.hpp"
#include "xRFI.hpp"
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/operations.hpp>
struct CoaddMPI_Params {
		bool same_for_all;
		StringVector rootpath;
		std::string outfile;
		std::string group_string;
		float loadsecs;
		bool kur;
		// excision
		float tfac;
		float ffac;
		excision::Method method;
		excision::Filter filter;
};
namespace mpi = boost::mpi;
/****
 *	An assumption is at play here.
 *	I am assuming that MPI coaddition is run from any vlite-difx* nodes.
 *  and not from vlite-nrl.
 *  This is by design of MPI. "in_value".
 *  One workaround for future would be to use 
 *  zero'd array as inptr on vlite-nrl
 *  = Ignore, I was dumb then. I can specify hosts and run mpirun from foreign hosts too.
 * */
class CoaddMPI {
		private:
				std::string filename;
				FilterbankWriter fbw;
				FilterbankReader fbr;
				mpi::environment env;
				mpi::communicator world;
				PathList filpathlist;
				StringVector filstringlist, All_hostnames;
				std::vector<StringVector> All_filpathlist;
				int root, nbits;
				void work_one_group(const CoaddMPI_Params& param) {
						// coadd variables
						double mintstart, maxtstop, duration, tstart, tstop;
						DoubleVector All_tstarts, All_tstops, l_tstarts, l_tstops;
						timeslice nsteps, numelems, boundcheck, i0, tstep;
						double timenow, dtimenow;
						// data holding ptrs
						PtrFloat outptr = nullptr;
						std::vector<PtrFloat> inptrs;
						// loop runner
						int numants = 0;
						timeslice i = 0;
						// name resolution
						AnalyzeFB afb;	
						if(param.same_for_all) afb.Crawl(param.rootpath[0]);
						else afb.Crawl(param.rootpath[ world.rank() ]);
						if(param.kur) filpathlist = afb.kfils[param.group_string];
						else filpathlist = afb.fils[param.group_string];
						for(auto& fpl : filpathlist) filstringlist.push_back(fpl.string());
						// name resolution  END
						/***
						 * To account for nodes with multiple files, 
						 * we add them in one process.
						 * **/
						// aggregate in root For debugging
						mpi::gather(world, filstringlist , All_filpathlist, root);
      mpi::gather(world, env.processor_name(), All_hostnames, root);
      if(world.rank() == root) {
        // output
        std::cout << "Asgard::agmcoadd\n";
        std::cout << "  will write out to " << param.outfile << std::endl;
        std::cout << "  RFI Excision=" << param.method << std::endl;
        std::cout << "  RFI Filter=" << param.filter << std::endl;
        std::cout << "  tfac=" << param.tfac << "  ffac=" << param.ffac << std::endl;
        for(int iii = 0; iii !=  All_filpathlist.size(); iii++)
          for(auto& Sv : All_filpathlist[iii]) {
            std::cout << " I " << Sv << " @ " << All_hostnames[iii];
            if(All_hostnames[iii] == env.processor_name()) std::cout << "*";
            std::cout << std::endl;
          }
      }
      FilterbankList f;
      for(auto& xpl : filpathlist) {
        // read filterbanks into f
        Filterbank ffb;
        fbr.Read(ffb, xpl.string());
        f.push_back(ffb);
        // keep times
        l_tstops.push_back(ffb.tstop);
        l_tstarts.push_back(ffb.tstart);
        // initializing data pointers
        inptrs.push_back(nullptr);
      }
      assert(f.size() != 0);
      assert(f.size() == inptrs.size());
      // figure out local mintstart, maxtstop
      tstart = *std::min_element(l_tstarts.begin(), l_tstarts.end());
      tstop  = *std::max_element(l_tstops.begin(), l_tstops.end());
      // figure out global mintstart and maxtstart
      // should I have all_reduce or reduce on root?
      mpi::all_reduce(world, tstart, mintstart, mpi::minimum<double>());
      mpi::all_reduce(world, tstop, maxtstop, mpi::maximum<double>());
      mpi::gather(world, tstart, All_tstarts, root);
      mpi::gather(world, tstop, All_tstops, root);
      // maxtstop - mintstart is in fractional mjd
      // duration of the resultant filterbank
      duration = 86400.0f * ( maxtstop - mintstart );
      // write fb header logic
      // reading f[0] lol
      if(world.rank() == root) {
        nbits = f[0].nbits;
        boundcheck = fbw.Initialize(param.outfile, f[0],  duration, mintstart, nbits);
      }
      mpi::broadcast(world, nbits, root);
      // figure out tstep(width)
      tstep = param.loadsecs / f[0].tsamp;				
      numelems = tstep * f[0].nchans;
      nsteps = duration / param.loadsecs;
      // create xrfi
      excision::xRFI xrfi(param.method, param.tfac, param.ffac, tstep, f[0].nchans);
      // initialize stuff
      for(auto& xin : inptrs) xin = new float[numelems]();
      if(world.rank() == root) outptr = new float[numelems]();
      // timenow is current time
      timenow = 0.0;
      i = 0;
      // coadd logic
      while (true) {
        // initialization
        numants = 0;
        // i is like loop index
        mpi::broadcast(world, i, root);
        if(i == nsteps) break;
        // work start
        timenow = mintstart + ( i * param.loadsecs / 86400.0f );
        // read fbdata into inptr
        for(int ifb = 0; ifb != f.size(); ifb++){
          if(timenow >= f[ifb].tstart && timenow < f[ifb].tstop) {
            i0 = ( timenow - f[ifb].tstart ) * 86400.0f / f[ifb].tsamp;
            f[ifb].Unpack(inptrs[ifb], i0, tstep);
            // Excision here -- begin
            xrfi.Excise(inptrs[ifb], param.filter);
            // Excision here -- end
          }
          else{
            // this only happens in the beginning hence not optimized
            std::fill(inptrs[ifb], inptrs[ifb] + numelems, 0);
          }
        }
        // local coaddition
        // result in inptrs[0]
        for(int ipt = 1; ipt != inptrs.size(); ipt++) {
          PtrFloat a_in = inptrs[0];
          PtrFloat b_in = inptrs[ipt];
#ifdef AGOMP
#pragma omp parallel for
#endif
          for(timeslice iii = 0; iii < numelems; iii++)
            a_in[iii] += b_in[iii];
        }
        // actual MPI coadd call
        mpi::reduce(world, inptrs[0], numelems, outptr, std::plus<float>(), root);
        // divide logic
        if(world.rank() == root) {
          // count numant
          for(int j = 0; j < world.size(); j++){
            if(timenow <= All_tstops[j] && timenow >= All_tstarts[j]) numants++;
          }
          // Excision here -- begin
          xrfi.Excise(outptr, param.filter);
          // Excision here -- end
          // write fb data logic
          fbw.WriteFBdata(outptr, boundcheck, numelems, numants);
          // incrementing as in serial
          // NB see comments in serial code
          boundcheck += (numelems * nbits / 8);
          i++;
        }
      }
      // gracefully exiting
      for(auto& ipf : inptrs) if(ipf != nullptr)  delete[] ipf;
      if(world.rank() == root) {
        if(outptr != nullptr) delete[] outptr;
        fbw.Close();
      }
    }
  public:
    //CoaddMPI(mpi::environment _env, mpi::communicator _comm) :
    //env(_env), world(_comm) {
    //}
    void Work(CoaddMPI_Params& param) {
      root = 0;
      work_one_group(param);
    }
};
#endif // MPI
#endif
