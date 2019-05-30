#ifndef ASGARD_H
#include "asgard.hpp"
#endif
#ifndef COADD_H
#define COADD_H
#include "Filterbank.hpp"
#include "xRFI.hpp"
#include "Group.hpp"

FilterbankList FLFromDE(DEList& x) {
		FilterbankList ret;
		FilterbankReader fbr;
		for(fs::directory_entry& de : x) {
				Filterbank xx;
				fbr.Read(xx, de.path().string()); 
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
				float mintstart, maxdur;
				timeslice retsize, i0, ddit;
				FilterbankWriter fbw;
				int method;
				float * bandshape, * timeshape;
				FloatVector bins;
				char * bandflags, * timeflags;
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
						bandflags = new char[nchans];
						timeflags = new char[wid];
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
						float currTime = 0.0f;
						for(timeslice i = 0; i < nsteps; i++) {
								NumAnt = 0;
								for(Filterbank& f : fl) {
										// i0 * f.tsamp <-- timestep
										currTime = i0 * f.tsamp;
										if(currTime < f.duration && currTime >= (f.tstart - mintstart) ) {  
												f.Unpack(fbdat, i0, tstep);
												// <--------------------->
												// this is where you put your xrfi logic
												// call here
												if(!xxinitialized) _init_xxer(tstep, f.nchans);
												if(!method)	_xxer(fbdat, tstep, f.nchans);
												// <--------------------->
												NumAnt++;
												for(timeslice j = 0; j < tdat_size; j++) {
														tdat[j] += fbdat[j]; 
														fbdat[j] = 0.0f;
												}
										}
								}
								for(timeslice j = 0; j < tdat_size; j++) {
										if(NumAnt == 0) tdat[j] = 0.0f;
										else tdat[j] = std::floor(tdat[j] / NumAnt);
										//else tdat[j] = std::floor(tdat[j]);
								}
								bw.WriteFBdata(tdat,ddit, tdat_size);
								for(timeslice j = 0; j < tdat_size; j++) tdat[j] = 0.0f;
								i0 += tstep;
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
								bandshape = timeshape =  NULL;
								bandflags = timeflags = NULL;
								method = 0;
						}
				void flagger(DEList& x) {
						FilterbankList fl; 
						fl = FLFromDE(x);
						flagger( fl );
				}
				void flagger(FilterbankList& fl) {
						// TODO
				}
				void coadd(DEList& x) {
						FilterbankList fl; 
						fl = FLFromDE(x);
						coadd( fl );
				}
				void coadd(FilterbankList& fl) {
						// compute max duration here
						FloatVector duration_l, tstart_l;
						std::for_each(fl.begin(), fl.end(), [&duration_l, &tstart_l](Filterbank& bf) { duration_l.push_back( (float)bf.duration ); tstart_l.push_back( (float) bf.tstart );  });
						maxdur = *(std::max_element(duration_l.begin(), duration_l.end()));
						mintstart = *(std::min_element(tstart_l.begin(), tstart_l.end()));
						ddit = fbw.Initialize(filename, fl[0], maxdur, mintstart);
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
};
namespace mpi = boost::mpi;
/****
 *	An assumption is at play here.
 *	I am assuming that MPI coaddition is run from any vlite-difx* nodes.
 *  and not from vlite-nrl.
 *  This is by design of MPI. "in_value".
 *  One workaround for future would be to use 
 *  zero'd array as inptr on vlite-nrl
 * */
class CoaddMPI {
		private:
				std::string filename;
				FilterbankWriter fbw;
				FilterbankReader fbr;
				mpi::environment env;
				mpi::communicator world;
				PathList filpathlist;
				StringVector filstringlist;
				std::vector<StringVector> All_filpathlist;
				int root;
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
						if(world.rank() == root) {
								for(auto& vSv : All_filpathlist) 
										for(auto& Sv : vSv)
												std::cout << " I " << Sv << std::endl;
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
								boundcheck = fbw.Initialize(param.outfile, f[0],  duration, mintstart);
						}
						// figure out tstep(width)
						tstep = param.loadsecs / f[0].tsamp;				
						numelems = tstep * f[0].nchans;
						nsteps = duration / param.loadsecs;
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
#pragma omp parallel for
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
										// write fb data logic
										fbw.WriteFBdata(outptr, boundcheck, numelems, numants);
										// incrementing as in serial
										// NB see comments in serial code
										boundcheck += numelems/4;
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
