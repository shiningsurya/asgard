#include "asgard.hpp"
#include "Filterbank.hpp"
#include "Candidate.hpp"
#include "Operations.hpp"
#include "FilterbankCandidate.hpp"
#include "Plotter.hpp"
class CandPlot : protected Plotter {
		protected:
						int imin, imax;
						float xlin[2], ylin[2];
						char txt[16];
		public:
				CandPlot(std::string fn) : Plotter(fn) {
						cpgpap (0.0,0.618); //10.0, width and aspect ratio
						fac = 1e-2f;
				}
				void Plot(PathList& fl, PathList& cl) {
						CandidateAntenna cant = CAFromPL(cl);
						StringVector ced, fed;
						fed = SVFromPL(fl); ced = SVFromPL(cl);
						std::sort(fed.begin(), fed.end(), SCompare);
						std::sort(ced.begin(), ced.end(),SCompare);
						int lmin = std::min(fed.size(), ced.size());
						for(int i = 0; i < lmin; i++) {
								FilterbankCandidate fbc(fed[i], ced[i]);
								Plot(fbc, cant);
						}
				}
				void Plot(FilterbankCandidate& fcl, CandidateAntenna& cant) {
						int chanout = 512;
						float * ffddw = NULL;
						std::string str;
						int twindow;
						FloatVector freqs, taxis;
						float min, max, xxmin, xxmax;
						/////////////////CANT//////////////////////////////
						nant = cant.size();
						w = (1*0.88/nant) - 0.02;
						xmin = 0.7;
						ymin = 0.07;
						xmax = 0.9;
						ymax = w + ymin;
						std::sort(cant.begin(), cant.end(), CLCompare );
						/////////////////CANT//////////////////////////////
						for(int cx = 0; cx < fcl.size(); cx++) {
								if(count != 0) {
										cpgpage();
										w = (1*0.88/nant) - 0.02;
										xmin = 0.7;
										ymin = 0.07;
										xmax = 0.9;
										ymax = w + ymin;
								}
								freqs = operations::FreqTable((float)fcl.fch1, (float)fcl.foff, fcl.nchans);
								// this function plots what is one page
								taxis = operations::TimeAxis((float)fcl.tsamp, fcl.istart, fcl.istop);
								twindow = fcl.nsamps - fcl.maxdelay;
								//////////////////////////////////////////////////
								tr[3] = freqs.front(); // fixed
								tr[0] = fcl.istart*(float)fcl.tsamp; // fixed
								tr[2] = (float)fcl.tsamp;
								tr[4] = 1.f*(float)fcl.foff; 
								//cpgsci(1); // color index
								cpgsvp(0.08, 0.33, 0.1, 0.45); // de-dispersed waterfall
								cpgswin(taxis[0], taxis[twindow-1], freqs.front(),  freqs.back());
								cpgbox("BCN",0.0,0,"BCNV",0.0,0);
								cpgsfs(1);
								cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
								// fsrunching
								ffddw = new float[twindow * chanout](); // uniform initialization, c++11
								operations::Fscrunch(fcl.dd_fb, fcl.nchans, twindow, chanout, ffddw);
								tr[4] *= (4096/chanout);
								cpgimag(ffddw, chanout, twindow, 1, chanout, 1, twindow, 0, 3, tr);
								//cpglab("Time (s)", "Freq (MHz)", "De-Dispersed Waterfall");
								cpgmtxt("B",2.5,.5,0.5,std::string("Time (s)").c_str()); // group at middle
								cpgmtxt("L",4,0.5,0.5,std::string("Freq (MHz)").c_str());
								cpgmtxt("T",.3,.5,0.5, std::string("De-Dispersed Waterfall").c_str());
								// start line
								cpgsci(6);
								cpgsls(2);
								xlin[0] = fcl.tsamp*(fcl.i0 - 3.5 * fcl.filterwidth);
								xlin[1] = fcl.tsamp*(fcl.i0 - 3.5 * fcl.filterwidth);
								ylin[0] = freqs.back();
								ylin[1] = freqs.front();
								cpgline(2,xlin, ylin);
								// end line
								xlin[0] = fcl.tsamp*(fcl.i1 + 3.5 * fcl.filterwidth);
								xlin[1] = fcl.tsamp*(fcl.i1 + 3.5 * fcl.filterwidth);
								cpgline(2,xlin, ylin);
								cpgsls(1);
								delete[] ffddw;
								//////////////////////////////////////////////////
								//chanout = 4096;
								//trf[4] = 1.f*(float)fcl.foff * fcl.nchans / chanout;
								cpgsci(1); // color index
								cpgsvp(0.42, 0.67, 0.1, 0.45); // dispersed waterfall
								cpgswin(taxis[0], taxis[fcl.nsamps-1], freqs.front(),  freqs.back());
								cpgbox("BCN",0.0,0,"BCNV",0.0,0);
								cpgsfs(1);
								cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
								ffddw = new float[fcl.nsamps*chanout]();
								operations::Fscrunch(fcl.d_fb, fcl.nchans, fcl.nsamps, chanout, ffddw);
								cpgimag(ffddw, chanout, fcl.nsamps, 1, chanout, 1, fcl.nsamps, 0, 3, tr);
								//trf[4] = 1.f*(float)fcl.foff; 
								//cpgimag(fcl.d_fb, fcl.nchans, fcl.nsamps, 1, fcl.nchans, 1, fcl.nsamps, 0, 3, trf);
								//cpglab("Time (s)", "", "Dispersed Waterfall");
								cpgmtxt("B",2.5,.5,0.5,std::string("Time (s)").c_str()); // group at middle
								cpgmtxt("T",.3,.5,0.5, std::string("Dispersed Waterfall").c_str());
								delete[] ffddw;
								//////////////////////////////////////////////////
								cpgsci(1); // color index
								cpgsvp(0.08, 0.33, 0.55, 0.9); // fscrunched profile 
								//cpgswin(76, 78.5, 0..0, 0.2 );
								min = *std::min_element(fcl.dd_tim, fcl.dd_tim + twindow);
								max = *std::max_element(fcl.dd_tim, fcl.dd_tim + twindow);
								xxmin = min - .1 * min;
								xxmax = max + .1 * min;
								//std::cout << "Plotter limits: " << xlin[0] << " " << xlin[1] << std::endl;
								cpgswin(taxis[0],taxis[twindow -1], xxmin, xxmax );
								cpgbox("BCN",0.0,0,"BCNV",0.0,0);
								cpgline(twindow, taxis.data(), fcl.dd_tim); 
								//cpglab("", "Intensity (a.u)", "De-dispersed Integrated Profile");
								cpgmtxt("L",4,0.5,0.5,std::string("Intensity (a.u.)").c_str());
								cpgmtxt("T",.3,.5,0.5, std::string("De-Dispersed Integrated Profile").c_str());
								str = std::string("S/N: ") + std::to_string(fcl.sn);
								cpgmtxt("T",-1.5*charh, .99, 1.0, str.c_str());
								cpgmtxt("T",2.0,1.0,0.3,fcl.group.c_str());
								// start line
								cpgsci(6);
								cpgsls(2);
								xlin[0] = fcl.tsamp*(fcl.i0 - 3.5 * fcl.filterwidth);
								xlin[1] = fcl.tsamp*(fcl.i0 - 3.5 * fcl.filterwidth);
								ylin[0] = xxmin;
								ylin[1] = xxmax;
								cpgline(2,xlin, ylin);
								// end line
								xlin[0] = fcl.tsamp*(fcl.i1 + 3.5 * fcl.filterwidth);
								xlin[1] = fcl.tsamp*(fcl.i1 + 3.5 * fcl.filterwidth);
								cpgline(2,xlin, ylin);
								cpgsls(1);
								cpgsci(1);
								//////////////////////////////////////////////////
								cpgsci(1); // color index
								cpgsvp(0.42, 0.67, 0.55, 0.9); // Scatter 
								cpgswin(0.,2., 1, 3);
								for(int i = 0; i < fcl.size(); i++) cpgcirc(log10(1e3f*fcl.clwd[i]), log10(fcl.cldm[i]), fac*log10(fcl.clsn[i]));
								// yline
								ylin[0] = log10((float)fcl.dm);
								ylin[1] = log10((float)fcl.dm);
								xlin[0] = 0.0f;
								xlin[1] = 2.0f;
								cpgline(2,xlin, ylin); 
								// xline
								xlin[0] = log10( (float)(fcl.filterwidth * fcl.tsamp * 1e3f) );
								xlin[1] = log10( (float)(fcl.filterwidth * fcl.tsamp * 1e3f) );
								ylin[0] = 1.0f;
								ylin[1] = 3.0f;
								cpgline(2,xlin, ylin); 
								cpgbox("BCLNTS",0.0,0,"BCLVNTS",0.0,0);
								//cpglab("Width (ms)", "DM (pc/cc)", "Scatter");
								cpgmtxt("B",2.5,.5,0.5,std::string("Width (ms)").c_str()); // group at middle
								cpgmtxt("L",4,0.5,0.5,std::string("DM (pc/cc)").c_str());
								cpgmtxt("T",.3,.5,0.5, std::string("Scatter").c_str());
								str = std::string("DM: ") + std::to_string(fcl.dm); 
								cpgmtxt("T",-1.5*charh, 0.99, 1.0, str.c_str());
								str = std::string("Width: ") + std::to_string(fcl.tsamp * fcl.filterwidth * 1e3f);
								cpgmtxt("T",-3*charh, 0.99, 1.0, str.c_str());
								//cpgmtxt("T",1.5,1.0,0.0,fcl.antenna.c_str());
								/////////////////CANT//////////////////////////////
								ylin[0] = 0.0f;
								ylin[1] = 1.0f;
								for(CandidateList& xcl : cant) {
										cpgsci(1); // color index
										if(xcl[0].antenna == fcl.antenna) {
												// change color
												cpgsci(6);
												//cpgsls(2);
										}
										cpgsvp(xmin, xmax, ymin, ymax);
										cpgswin(fcl.istart*fcl.tsamp, fcl.istop*fcl.tsamp, ylin[0], ylin[1]);
										// ONLY first time
										if(ymin == 0.07f)  { 
												cpgbox("BCN",0.0,0,"BC",0.0,0);
												cpgmtxt("B",2.5,.5,0.5,std::string("Time (s)").c_str()); 
										}
										else  cpgbox("BC",0.0,0,"BC",0.0,0);
										cpgmtxt("RV",2,.5,0.5,xcl[0].antenna.c_str());
										for(Candidate& cx : xcl) {
												cpgsci(1);
												xlin[1] = cx.i0*fcl.tsamp; 
												xlin[0] = cx.i0*fcl.tsamp;
												if(cx.i0 == fcl.i0) {
														cpgsci(5);
														//cpgsls(3);
												}
												cpgline(2, xlin, ylin);
										}
										xlin[1] = fcl.i0*fcl.tsamp;
										xlin[0] = fcl.i0*fcl.tsamp;
										cpgsci(5); cpgsls(2);
										cpgline(2,xlin, ylin);
										cpgsci(1); cpgsls(1);
										ymin = ymax + 0.02;
										ymax += w   + 0.02 ;
								}
								/////////////////CANT//////////////////////////////
								count++;
								//break;
								if( ! fcl.Next() ) break;
						}
				}
				void Plot(FilterbankCandidate& fcl) {
						int chanout = 512;
						PtrFloat ffddw = nullptr;
						std::string str;
						int twindow;
						FloatVector freqs, taxis;
						float min, max, xxmin, xxmax, dd_range;
						while(fcl.Next()) {
								// put selection logic here
								if(fcl.sn < 10) continue;
								// end selection logic
								if(count != 0) {
										cpgpage();
								}
								freqs = operations::FreqTable((float)fcl.fch1, (float)fcl.foff, fcl.nchans);
								// this function plots what is one page
								taxis = operations::TimeAxis((float)fcl.tsamp, fcl.istart, fcl.istop);
								twindow = fcl.nsamps - fcl.maxdelay;
								//////////////////////////////////////////////////
								tr[3] = freqs.front(); // fixed
								tr[0] = fcl.istart*(float)fcl.tsamp; // fixed
								tr[2] = (float)fcl.tsamp;
								tr[4] = 1.f*(float)fcl.foff; 
								//cpgsci(1); // color index
								cpgsvp(0.1, 0.45, 0.1, 0.5); // de-dispersed waterfall
								cpgswin(taxis[0], taxis[twindow-1], freqs.front(),  freqs.back());
								cpgbox("BCN",0.0,0,"BCNV",0.0,0);
								cpgsfs(1);
								cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
								// fsrunching
								ffddw = new float[twindow * chanout](); // uniform initialization, c++11
								operations::Fscrunch(fcl.dd_fb, fcl.nchans, twindow, chanout, ffddw);
								// array ops
								//
								// array ops
								tr[4] *= (4096/chanout);
								cpgimag(ffddw, chanout, twindow, 1, chanout, 1, twindow, fcl.bmin, fcl.bmax, tr);
								//cpglab("Time (s)", "Freq (MHz)", "De-Dispersed Waterfall");
								cpgmtxt("B",2.5,.5,0.5,"Time (s)"); // group at middle
								cpgmtxt("L",4,0.5,0.5,"Freq (MHz)");
								//cpgmtxt("T",.3,.5,0.5, "De-Dispersed Waterfall");
								// start line
								cpgsci(6);
								cpgsls(2);
								xlin[0] = fcl.tsamp*(fcl.i0 - 3.5 * fcl.filterwidth);
								xlin[1] = fcl.tsamp*(fcl.i0 - 3.5 * fcl.filterwidth);
								ylin[0] = freqs.back();
								ylin[1] = freqs.front();
								cpgline(2,xlin, ylin);
								// end line
								xlin[0] = fcl.tsamp*(fcl.i1 + 3.5 * fcl.filterwidth);
								xlin[1] = fcl.tsamp*(fcl.i1 + 3.5 * fcl.filterwidth);
								cpgline(2,xlin, ylin);
								cpgsls(1);
								delete[] ffddw;
								//////////////////////////////////////////////////
								//chanout = 4096;
								//trf[4] = 1.f*(float)fcl.foff * fcl.nchans / chanout;
								cpgsci(1); // color index
								cpgsvp(0.55, 0.95, 0.1, 0.45); // dispersed waterfall
								cpgswin(taxis[0], taxis[fcl.nsamps-1], freqs.front(),  freqs.back());
								cpgbox("BCN",0.0,0,"BCNV",0.0,0);
								cpgsfs(1);
								cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
								ffddw = new float[fcl.nsamps*chanout]();
								operations::Fscrunch(fcl.d_fb, fcl.nchans, fcl.nsamps, chanout, ffddw);
								cpgimag(ffddw, chanout, fcl.nsamps, 1, chanout, 1, fcl.nsamps, fcl.bmin, fcl.bmax, tr);
								//trf[4] = 1.f*(float)fcl.foff; 
								//cpgimag(fcl.d_fb, fcl.nchans, fcl.nsamps, 1, fcl.nchans, 1, fcl.nsamps, 0, 3, trf);
								//cpglab("Time (s)", "", "Dispersed Waterfall");
								cpgmtxt("B",2.5,.5,0.5,"Time (s)"); // group at middle
								cpgmtxt("T",.3,.5,0.5, "Dispersed Waterfall");
								delete[] ffddw;
								//////////////////////////////////////////////////
								cpgsci(1); // color index
								cpgsvp(0.1, 0.45, 0.5, 0.9); // fscrunched profile 
								//cpgswin(76, 78.5, 0..0, 0.2 );
								min = *std::min_element(fcl.dd_tim, fcl.dd_tim + twindow);
								max = *std::max_element(fcl.dd_tim, fcl.dd_tim + twindow);
								dd_range = max - min;
								xxmin = min - .1 * dd_range * min;
								xxmax = max + .1 * dd_range * min;
								//std::cout << "Plotter limits: " << xlin[0] << " " << xlin[1] << std::endl;
								cpgswin(taxis[0],taxis[twindow -1], xxmin, xxmax );
								cpgbox("BC",0.0,0,"BCNV",0.0,0);
								cpgline(twindow, taxis.data(), fcl.dd_tim); 
								//cpglab("", "Intensity (a.u)", "De-dispersed Integrated Profile");
								cpgmtxt("R",0,0.5,0.5,"Intensity (a.u.)");
								cpgmtxt("T",.3,.5,0.5, "De-Dispersed Integrated Profile and Waterfall");
								snprintf(txt, 16, "S/N: %3.2f", fcl.sn);
								cpgmtxt("T",-1.5*charh, .99, 1.0, txt);
								cpgmtxt("T",2.0,1.0,0.3,fcl.group.c_str());
								// start line
								cpgsci(6);
								cpgsls(2);
								xlin[0] = fcl.tsamp*(fcl.i0 - 3.5 * fcl.filterwidth);
								xlin[1] = fcl.tsamp*(fcl.i0 - 3.5 * fcl.filterwidth);
								ylin[0] = xxmin;
								ylin[1] = xxmax;
								cpgline(2,xlin, ylin);
								// end line
								xlin[0] = fcl.tsamp*(fcl.i1 + 3.5 * fcl.filterwidth);
								xlin[1] = fcl.tsamp*(fcl.i1 + 3.5 * fcl.filterwidth);
								cpgline(2,xlin, ylin);
								cpgsls(1);
								cpgsci(1);
								//////////////////////////////////////////////////
								cpgsci(1); // color index
								cpgsvp(0.55, 0.95, 0.55, 0.90); // Scatter 
								cpgswin(0.,1., 2, 3);
								for(int i = 0; i < fcl.size(); i++) cpgcirc(log10(1e3f*fcl.clwd[i]), log10(fcl.cldm[i]), fac*log10(fcl.clsn[i]));
								// yline
								ylin[0] = log10((float)fcl.dm);
								ylin[1] = log10((float)fcl.dm);
								xlin[0] = 0.0f;
								xlin[1] = 2.0f;
								cpgline(2,xlin, ylin); 
								// xline
								xlin[0] = log10( (float)(fcl.filterwidth * fcl.tsamp * 1e3f) );
								xlin[1] = log10( (float)(fcl.filterwidth * fcl.tsamp * 1e3f) );
								ylin[0] = 1.0f;
								ylin[1] = 3.0f;
								cpgline(2,xlin, ylin); 
								cpgbox("BCLNTS",0.0,0,"BCLVNTS",0.0,0);
								//cpglab("Width (ms)", "DM (pc/cc)", "Scatter");
								cpgmtxt("B",2.5,.5,0.5,"Width (ms)"); // group at middle
								cpgmtxt("L",4,0.5,0.5,"DM (pc/cc)");
								cpgmtxt("T",.3,.5,0.5, "Scatter");
								snprintf(txt, 16, "DM: %3.2f", fcl.dm);
								cpgmtxt("T",-1.5*charh, 0.99, 1.0, txt);
								snprintf(txt, 16, "Width: %3.2f", fcl.tsamp*fcl.filterwidth*1e3f);
								cpgmtxt("T",-3*charh, 0.99, 1.0, txt);
								cpgmtxt("T",1.5,1.0,0.0,fcl.antenna.c_str());
								count++;
								//break;
								if( ! fcl.Next() ) break;
						}
				}
};
