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
				int root;
				void work_one_group(const CoaddMPI_Params& param) {
						// name resolution
						PathList pl;
						std::string ifile;
						AnalyzeFB afb;	
						if(param.same_for_all) afb.Crawl(param.rootpath[0]);
						else afb.Crawl(param.rootpath[ world.rank() ]);
						// TODO process slave affinity
						if(param.kur) pl = afb.kfils[param.group_string];
						else pl = afb.fils[param.group_string];
						// assert(pl.size() == 1); // disabled during debug
						//ifile = pl[0].string();
						ifile = pl[world.rank()].string();
						std::cout << " I  " << world.rank() << " working  " << ifile << std::endl;
						// coadd variables
						double mintstart, tsamp, maxtstop, tstop, duration;
						DoubleVector tstarts, tstops;
						timeslice nsteps, numelems, boundcheck, i0, tstep;
						PtrFloat inptr = nullptr, outptr = nullptr;
						double timenow;
						bool inptr_all_zeros = false;
						// read filterbank into f
						Filterbank f;
						fbr.Read(f, ifile);
						// tstart is in MJD
						// duration is in seconds
						tstop = f.tstart + ( f.duration/86400.0f );
						// figure out start time and duration
						// open-mpi docs say that order is guaranteed
						// therefore no need to make pair
						mpi::all_reduce(world, f.tstart, mintstart, mpi::minimum<double>());
						mpi::all_reduce(world, tstop, maxtstop, mpi::maximum<double>());
						mpi::gather(world, f.tstart, tstarts, root);
						mpi::gather(world, tstop, tstops, root);
						// maxtstop - mintstart is in fractional mjd
						duration = 86400.0f * ( maxtstop - mintstart );
						// offset .. tsteps later when filterbank starts
						timeslice offset;
						offset = std::ceil( ( f.tstart - mintstart ) / param.loadsecs  );
						// write fb header logic
						if(world.rank() == root) {
								boundcheck = fbw.Initialize(param.outfile, f,  duration, mintstart);
						}
						// figure out tstep(width)
						tstep = param.loadsecs / f.tsamp;				
						numelems = tstep * f.nchans;
						nsteps = duration / param.loadsecs;
						// initialize stuff
						inptr = new float[numelems]();
						if(world.rank() == root) outptr = new float[numelems]();
						inptr_all_zeros = true;
						timenow = 0.0;
						i0 = 0;
						// coadd logic
						int numants = 0;
						timeslice i = 0;
						while (true) {
								/***
								 * Current version is not generic. 
								 * The granularity is fixed by loadsecs.
								 *
								 * **/
								mpi::broadcast(world, i, root);
								if(i == nsteps) break;
								numants = 0;
								i0 = i * tstep;
								timenow = mintstart + ( i0 * f.tsamp / 86400.0f );
								// read fbdata into inptr
								/***
								 * i0 is Bcasted.
								 * each process either sends fbdata or zero'array.
								 * this is bc MPI communicator is fixed
								 * ----------
								 *  The differential (offset) which remains is ignored bc current coadd logic 
								 *  is in loadsecs steps 
								 *  My rational is different tstarts is not something we expect 
								 *  in our pipeline very often. Infact it is bad behavior.
								***/
								if(timenow >= f.tstart && timenow < tstop) {
										f.Unpack(inptr, i0 - offset*tstep, tstep);
										inptr_all_zeros = false;
								}
								else{
										if(!inptr_all_zeros) {
												std::fill(inptr, inptr + numelems, 0);
												inptr_all_zeros = true;
										}
								}
								// actual MPI coadd call
								mpi::reduce(world, inptr, numelems, outptr, std::plus<float>(), root);
								// divide logic
								if(world.rank() == root) {
										// count numant
										for(int j = 0; j < world.size(); j++){
												if(timenow <= tstops[j] && timenow >= tstarts[j]) numants++;
										}
										// write fb data logic
										fbw.WriteFBdata(outptr, boundcheck, numelems, numants);
										// incrementing as in serial
										// NB see comments in serial code
										boundcheck += numelems/4;
										i++;
								}
						}
						delete[] inptr;
						if(world.rank() == root) {
								delete[] outptr;
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
