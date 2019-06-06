#include "asgard.hpp"
#include "Filterbank.hpp"
#include "Candidate.hpp"
#include "Operations.hpp"
#include "xRFI.hpp"
#include "Plotter.hpp"
class CandSummary : protected Plotter {
		protected:
				double tsamp;
				double imin, imax;
				std::string group, antenna;
				float xxx[1], yyy[1];
				int iant;
				float fac;
		public:
				CandSummary(std::string fn, double ts) : Plotter(fn) {
						cpgpap (0.0,0.618); //10.0, width and aspect ratio
						imin = 1e16;
						imax = -1e16; 
						tsamp = ts;
						fac = 2*1e-2f;
				}	
				void Plot(CandidateAntenna& cant) {
						if(count > 0) cpgpage();
						// CandidateAntenna is vector of CandidateList
						// CandidateList is vector of Candidate
						// sort cant
						std::sort(cant.begin(), cant.end(), CLCompare );
						for(CandidateList& x : cant) {
								if(x.size() == 0) return;
								std::sort(x.begin(), x.end(), iCompare );
								imin = imin < x.front().peak_time ? imin : x.front().peak_time;
								imax = imax > x.back().peak_time ? imax : x.back().peak_time;
						}
						nant = cant.size();
						int hants = nant/2;
						//////////DM////////////////////////////////////// 
						cpgsvp(0.05, 0.9, 0.05, 0.35);
						cpgswin(imin, imax, 1.0, 3.0);
						cpgsci(1); // color index
						cpgsls(1);
						cpgtbox("BCNTS",0.0,0,"BCLTSN",0.0,0);
						cpgmtxt("L", 2, .5, .5, std::string("DM (pc/cc)").c_str());
						cpgmtxt("B", 3*charh, .5, .5, std::string("Time (s)").c_str());
						// the main loop
						for(CandidateList& xx : cant) for(Candidate& x : xx) {
								iant = (AntennaIndex(x.antenna) % 16) + 1;
								cpgsci((int)(iant / 4) + 1);
								cpgsls((int)(iant % 4) + 1);
								xxx[0] =  x.peak_time;
								yyy[0] = log10(x.dm);
								cpgcirc(xxx[0],yyy[0],fac);
						}
						//////////width/////////////////////////////////// 
						cpgsvp(0.05, 0.9, 0.35, 0.65);
						cpgsci(1); // color index
						cpgsls(1);
						cpgswin(imin, imax, 0.0, 1.0);
						cpgtbox("BCTS",0.0,0,"BCLMTS",0.0,0);
						cpgmtxt("R", 2.5, .5, .5, std::string("Width (ms)").c_str());
						// the main loop
						for(CandidateList& xx : cant) for(Candidate& x : xx) {
								iant = (AntennaIndex(x.antenna) % 16) + 1;
								cpgsci((int)(iant / 4) + 1);
								cpgsls((int)(iant % 4) + 1);
								xxx[0] =  x.peak_time;
								yyy[0] = log10(x.filterwidth);
								cpgcirc(xxx[0],yyy[0],fac);
								//cpgline(1,xxx,yyy);
						}
						//////////sn////////////////////////////////////// 
						cpgsvp(0.05, 0.9, 0.65, .95);
						cpgsci(1); // color index
						cpgsls(1); // color index
						cpgswin(imin, imax, 1.0, 3.0);
						cpgtbox("BCTS",0.0,0,"BCLNTS",0.0,0);
						cpgmtxt("L", 2, .5, .5, std::string("S/N").c_str());
						// the main loop
						for(CandidateList& xx : cant) for(Candidate& x : xx) {
								iant = (AntennaIndex(x.antenna) % 16) + 1;
								cpgsci((int)(iant / 4) + 1);
								cpgsls((int)(iant % 4) + 1);
								xxx[0] =  x.peak_time;
								yyy[0] = log10(x.sn);
								cpgcirc(xxx[0],yyy[0],fac);
						}
						////////////////////////////////////////////////// 
						cpgsci(1); // color index
						cpgmtxt("T",1, .5, .5, cant[0][0].group.c_str());
						count++;
						iant++;
				}
				void Plot(DEList& x) {
						CandidateAntenna cx = CAFromDE(x);
						Plot( cx );
				}
};
class Waterfall : protected Plotter {
		private:
				int chanout;
				float timestep;
				float ctimestep;
				bool coarse;
		public:
				Waterfall(std::string fn, float ts, int chout) : chanout(chout), timestep(ts), Plotter(fn) {
						coarse = false;
						ctimestep = 0.0f;
						cpgpap (0.0,0.618); //10.0, width and aspect ratio
				}
				Waterfall(std::string fn, float ts, float cts, int chout) : chanout(chout), timestep(ts), ctimestep(cts), Plotter(fn) {
						coarse = true;
						cpgpap (0.0,0.618); //10.0, width and aspect ratio
				}
				void Plot(DEList& x) {
						FilterbankList fl; 
						fl = FLFromDE(x);
						Plot( fl );
				}
				void Plot(PathList& x) {
						FilterbankList fl; 
						fl = FLFromPL(x);
						Plot( fl );
				}
				void Plot(FilterbankList& fl) {
						std::sort(fl.begin(), fl.end(), FCompare); 
						std::string idx;
						int iant, nsteps;
						timeslice i0, wid, widout;
						float axis[] = {0.0, 2.0, 320., 360.};
						float maxdur;
						nant = fl.size();
						int hant = nant/2;
						FloatVector duration_list;
						std::for_each(fl.begin(), fl.end(), [&duration_list](Filterbank& bf) { duration_list.push_back( (float)bf.duration ); } ); 
						maxdur = *(std::max_element(duration_list.begin(), duration_list.end()));
						//
						if(coarse && ctimestep == 0.0f) {
								timestep = maxdur;
								nsteps = 1;
						}
						else {
								nsteps = (int) maxdur/timestep + 1;
						}
						// assuming all the filterbanks have the same frequency
						FloatVector ft = operations::FreqTable(fl[0]);
						float * juice, * juice2;
						axis[2] = ft.back(); 
						axis[3] = ft.front(); 
						for(int i = 0; i < nsteps; i++) {
								if(count != 0) cpgpage();
								w = (1*0.88/nant) - 0.02;
								xmin = 0.1;
								ymin = 0.05;
								xmax = 0.9;
								ymax = w + ymin;
								axis[0] = i * timestep;
								axis[1] = axis[0] + timestep;
								iant = 0;	
								for(Filterbank& f: fl) {
										if(coarse && ctimestep == 0.0f) wid = maxdur / f.tsamp;
										else wid = timestep / f.tsamp;
										if(ctimestep == 0.0f) widout = wid; 
										else widout = wid / (ctimestep / f.tsamp);	
										i0 = axis[0] / f.tsamp;
										tr[3] = ft.front(); // fixed
										tr[0] = axis[0]; // fixed
										tr[2] = (float)f.tsamp * wid / widout;
										tr[0] -= 0.5*tr[2];
										//tr[2] = (float)f.tsamp * widout / wid;
										tr[4] = 1.f*(float)f.foff * (f.nchans/chanout); 
										tr[3] -= 0.5*tr[4];
										cpgsci(1); // color index
										cpgsvp(xmin, xmax, ymin, ymax);
										// xmin, xmax remain the same
										// ymin ymax change
										cpgswin(axis[0],axis[1],axis[2],axis[3]);
										if(iant == 0) {
												cpgmtxt("B",2,.5,0.5,std::string("Time (s)").c_str()); // group at middle
												cpgbox("BCN",0.0,0,"BC",40.0,0);
												cpgmtxt("LV",3,0.2,0.0,std::to_string((int)axis[2]).c_str());
												cpgmtxt("LV",3,0.8,0.0,std::to_string((int)axis[3]).c_str());
										}
										else if( iant == hant) {
												cpgbox("BC",0.0,0,"BC",40.0,0);
												cpgmtxt("L",4,0.,0.5,std::string("Freq (MHz)").c_str());
												cpgmtxt("LV",3,0.2,0.0,std::to_string((int)axis[2]).c_str());
												cpgmtxt("LV",3,0.8,0.0,std::to_string((int)axis[3]).c_str());
										}
										else if(iant == nant - 1) {
												cpgbox("BC",0.0,0,"BC",40.0,0);
												cpgmtxt("T",1,.5,0.5,f.group.c_str()); // group at middle
												if(f.isKur) cpgmtxt("T",1,.8,0.5,std::string("KUR").c_str()); // group at middle
												else cpgmtxt("T",1,.8,0.5,std::string("NOKUR").c_str());
												cpgmtxt("LV",3,0.2,0.0,std::to_string((int)axis[2]).c_str());
												cpgmtxt("LV",3,0.8,0.0,std::to_string((int)axis[3]).c_str());
												cpgbox("BC",0.0,0,"BC",40.0,0);
												idx = std::string("Slice:") + std::to_string(i+1) + std::string("/") + std::to_string(nsteps);
												cpgmtxt("T",.5,.0,0.0,idx.c_str());   // slice index
										}
										else {
												cpgbox("BC",0.0,0,"BC",40.0,0);
												cpgmtxt("LV",3,0.2,0.0,std::to_string((int)axis[2]).c_str());
												cpgmtxt("LV",3,0.8,0.0,std::to_string((int)axis[3]).c_str());
										}
										// extract juice and cpgimag
										juice = new float[f.nchans*wid]();
										f.Unpack(juice, i0, wid);
										juice2 = new float[widout*chanout];
										operations::Crunch(juice, f.nchans, wid, chanout, widout, juice2);
										cpgsfs(1);
										cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
										cpgimag(juice2, chanout,  widout, 1, chanout, 1, widout, f.bmin, f.bmax, tr);
										//cpgimag(juice.data(), f.nchans, wid, 1, f.nchans, 1, wid, 0, 3, trf);
										cpgmtxt("RV",2,.5,0.5,f.antenna.c_str());
										ymin = ymax + 0.02;
										ymax += w   + 0.02 ;
										iant++;
										delete[] juice2;
										delete[] juice;
								}
								count++;
						}
				}
};
class DPlot : protected Plotter {
		private:
				int nsteps, nchans;
				timeslice widin, wid, nsamps_out;
				bool ikur;
				int nchans_out;
				FilterbankReader fbr;
				float ffac, tfac;
				double timestep;
				std::string slice_str;
                FloatVector bins = {0.0f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f};
                float xlin[2] = {0.0f, 0.0f}, ylin[2] = {0.0f, 0.0f};
		public:
				DPlot(std::string fn, float ts, int chout, float ffc, float tfc) : nchans(chout), timestep(ts), ffac(ffc), tfac(tfc), 
						Plotter(fn) {
								cpgpap (0.0,0.618); //10.0, width and aspect ratio

						}
				void Plot(fs::directory_entry ff, int method) {
						Filterbank xx;
						fbr.Read(xx, ff.path().string()); 
						Plot(xx, method);
				}
				void Plot(fs::path ff, int method) {
						Filterbank xx;
						fbr.Read(xx, ff.string()); 
						Plot(xx, method);
				}
				void Plot(Filterbank& f, int method) {
						nchans = 1024;
						wid = 256;
						// variables
						float axis[4];
						double duration = f.duration;
						int nsteps;
						int nchansin = f.nchans;
						excision::xestimate estt, estf;
						if(timestep == 0.0f) {
								timestep = duration;
								nsteps = 1;
						}
						else {
								nsteps = duration/timestep + 1;
						}
						widin = timestep / f.tsamp;
						timeslice i0 = 0; // one time initialization
						// RAII
						// Resource acquisition is initialization
						float * dat = new float[wid*nchans];
						float * datin = new float[widin*nchansin];
						float * bandshape = new float[nchans];
						float * timeshape = new float[wid];
						char *  bandflags = new char[nchans];
						for(int i = 0; i < nchans; i++) bandflags[i] = 'c';
						char * timeflags = new char[wid];
						for(timeslice i = 0; i < wid; i++) timeflags[i] = 'c';
						float * timex = new float[wid];
						float * freqx = new float[nchans];
						float dt = timestep / wid;
						float ft = f.foff * nchansin / nchans;
						freqx[nchans-1] = f.fch1;
						for(int ii = nchans-2; ii >= 0; ii--) freqx[ii] = freqx[ii+1] + ft;
						tr[3] = freqx[nchans-1]; // fixed
						tr[2] = (float)f.tsamp * widin / wid;
						tr[4] = ft;
						// work loop
						for(int i = 0; i < nsteps; i++, i0+=widin, count++) {
								if(count != 0) cpgpage();
								// work part
								f.Unpack(datin, i0, widin);
								operations::Crunch(datin, nchansin, widin, nchans, wid, dat);
								operations::FreqShape(dat, wid, nchans, bandshape);
								operations::TimeShape(dat, wid, nchans, timeshape);
								// xrfi part
								if(method == 1) {
										estf = excision::MAD(bandshape, nchans, bandflags, ffac);
										estt = excision::MAD(timeshape, wid, timeflags, tfac);
								}
								else if(method == 2) {
										estf = excision::Histogram(bandshape, nchans, bandflags, bins, ffac);
										estt = excision::Histogram(timeshape, wid, timeflags, bins, tfac);
								}
								// plot part
								// filterbank
								axis[0] = i * timestep;
								axis[1] = axis[0] + timestep;
								axis[2] = freqx[0];
								axis[3] = freqx[nchans-1];
								cpgsci(1);
								cpgsvp(0.1, 0.7, 0.1, 0.7);
								cpgswin(axis[0],axis[1],axis[2],axis[3]);
								cpgbox("BCNT",0.0,0,"BCTN",20.0,4);
								cpgsfs(1);
								cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
								tr[0] = axis[0];
								cpgimag(dat, nchans, wid, 1, nchans, 1, wid, 0, 3, tr);
								cpgmtxt("B",2,0.5,0.5,"Time (s)");
								cpgmtxt("L",3, 0.5, 0.5, "Freq (Hz)");
								// bandshape
								cpgsci(1);
								cpgsvp(0.7, 0.9, 0.1, 0.7);
								axis[2] = freqx[0];
								axis[3] = freqx[nchans-1];
								axis[0] = 0.0f;
								axis[1] = 4.0f;
								cpgswin(axis[0],axis[1],axis[2],axis[3]);
								cpgbox("BCM",0.0,0,"BC",30.0,0);
								cpgline(nchans, bandshape, freqx);
								// lines in bandshape
								xlin[0] = axis[0];
								xlin[1] = axis[1];
								cpgsci(3);
								for(int ii = 0; ii < nchans; ii++) {
										if(bandflags[ii] == 'o') {
												ylin[0] = freqx[ii];
												ylin[1] = freqx[ii];
												cpgline(2,xlin,ylin);
										}
								}
								// timeshape
								axis[0] = i * timestep;
								axis[1] = axis[0] + timestep;
								axis[2] = 0.0f;
								axis[3] = 4.0f;
								cpgsci(1);
								cpgsvp(0.1, 0.7, 0.7, 0.9);
								cpgswin(axis[0],axis[1],axis[2],axis[3]);
								cpgbox("BC",0.0,0,"BCM",0.0,0);
								cpgmtxt("T",3, 0.5, 0.5, f.group.c_str());
								cpgmtxt("T",0.5, 0.5, 0.5, f.antenna.c_str());
								if(f.isKur) cpgmtxt("T",3, 0.8, 0.0, "KUR");
								else cpgmtxt("T",3, 0.8, 0.0, "NOKUR");
								slice_str = std::string("Slice:") + std::to_string(i+1) + std::string("/") + std::to_string(nsteps);
								cpgmtxt("T",3,.0,0.0,slice_str.c_str());   // slice index
								timex[0] = axis[0];
								for(int ii = 1; ii < wid; ii++) timex[ii] = timex[ii-1] + dt;
								cpgline(wid, timex, timeshape);
								// lines in timeshape
								ylin[0] = axis[2];
								ylin[1] = axis[3];
								cpgsci(3);
								for(int ii = 0; ii < wid; ii++) {
										if(timeflags[ii] == 'o') {
												xlin[0] = timex[ii];
												xlin[1] = timex[ii];
												cpgline(2,xlin,ylin);
										}
								}
						}
						delete[] dat;
						delete[] bandshape;
						delete[] timeshape;
						delete[] bandflags;
						delete[] timeflags;
						delete[] timex;
						delete[] freqx;
				}
};
