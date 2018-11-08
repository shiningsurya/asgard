#include "asgard.hpp"
#include "Operations.hpp"
#ifndef PLOTTER_H
#define PLOTTER_H
#include "cpgplot.h"

class Waterfall {
		private:
				std::string filename;
				std::array<float,5> heat_l;
				std::array<float,5> heat_r;
				std::array<float,5> heat_g;
				std::array<float,5> heat_b;
				float contrast,brightness;
				float tr[6];
				bool interactive;
				float charh, timestep;
				int count, nant, hant;
		public:
				Waterfall(std::string fn) {
						Waterfall(fn, 2.0);
				}
				Waterfall(std::string fn, float ts){
						if(fn == std::string("?")) interactive = true;
						else interactive = false;
						timestep = ts;
						filename = fn; // output plot file name
						count = 0;
						//
						charh = .65; // pgplot char height
						heat_l = {0.0, 0.2, 0.4, 0.6, 1.0};
						heat_r = {0.0, 0.5, 1.0, 1.0, 1.0};
						heat_g = {0.0, 0.0, 0.5, 1.0, 1.0};
	 					heat_b = {0.0, 0.0, 0.0, 0.3, 1.0};
						contrast = 1;
						brightness = 0.5;
						cpgbeg(0,filename.c_str(),1,1); // beginning of another journey
						cpgsch(charh); // character height
						if(interactive) cpgask(1); // go manual mode
						else cpgask(0);
						cpgpap (0.0,0.618); //10.0, width and aspect ratio
				}
				~Waterfall() {
						cpgend();
				}
				void Plot(FilterbankList fl) {
						std::sort(fl.begin(), fl.end(), [](Filterbank x, Filterbank y) { return x.antenna > y.antenna; });
						std::string idx;
						int iant;
						timeslice i0, wid;
						float axis[] = {0.0, 2.0, 320., 360.};
						float trf[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
						float xmin, xmax, ymin, ymax, w;
						nant = fl.size();
						hant = nant/2;
						FloatVector duration_list;
						std::for_each(fl.begin(), fl.end(), [&duration_list](Filterbank bf) { duration_list.push_back( (float)bf.duration ); } ); 
						int nsteps = *(std::max_element(duration_list.begin(), duration_list.end()));
						nsteps = (int) nsteps/timestep + 1;
						// assuming all the filterbanks have the same frequency
						FloatVector ft = operations::FreqTable(fl[0]);
						float * juice;
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
								for(Filterbank f: fl) {
										wid = timestep / f.tsamp;
										i0 = axis[0] / f.tsamp;
										trf[3] = ft.back(); // fixed
										trf[0] = i0*(float)f.tsamp; // fixed
										trf[2] = (float)f.tsamp;
										trf[4] = -1.f*(float)f.foff; 
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
										f.Unpack(juice, i0, wid);
										cpgsfs(1);
										cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
										cpgimag(juice, wid,  f.nchans, 1, wid, 1, f.nchans, 0, 3, trf);
										//cpgimag(juice.data(), f.nchans, wid, 1, f.nchans, 1, wid, 0, 3, trf);
										cpgmtxt("RV",2,.5,0.5,f.antenna.c_str());
										ymin = ymax + 0.02;
										ymax += w   + 0.02 ;
										iant++;
								}
								count++;
						}
				}
};
class CandPlot {
		private:
				std::string filename;
				std::array<float,5> heat_l;
				std::array<float,5> heat_r;
				std::array<float,5> heat_g;
				std::array<float,5> heat_b;
				float contrast,brightness, charh;
				float tr[6];
				// vector of candidates needs to be here for
				// scatter plot
				int count;
		public:
				void Plot(FilterbankCandidate& fcl, CandidateAntenna& cant) {
						int chanout = 512;
						float * ffddw = NULL;
						std::string str;
						int twindow;
						float trf[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
						FloatVector freqs, taxis;
						float fac = 1e-2f, min, max, xxmin, xxmax;
						float xmin, xmax, ymin, ymax, w;
						float xlin[2], ylin[2];
						/////////////////CANT//////////////////////////////
						int nant = cant.size();
						w = (1*0.88/nant) - 0.02;
						xmin = 0.7;
						ymin = 0.07;
						xmax = 0.9;
						ymax = w + ymin;
						int imin, imax;
						 std::sort(cant.begin(), cant.end(), [](CandidateList x, CandidateList y) { return x[0].antenna > y[0].antenna; } );
						/*
						 *imin = INT_MAX;
						 *imax = INT_MIN;
						 *for(CandidateList& x : cant) {
						 *        std::sort(x.begin(), x.end(), [](Candidate x, Candidate y) { return x.i0 < y.i0; } );
						 *        imin = imin < x.front().i0 ? imin : x.front().i0;
						 *        imax = imax > x.back().i0 ? imax : x.back().i0;
						 *}
						 */
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
								trf[3] = freqs.front(); // fixed
								trf[0] = fcl.istart*(float)fcl.tsamp; // fixed
								trf[2] = (float)fcl.tsamp;
								trf[4] = 1.f*(float)fcl.foff; 
								//cpgsci(1); // color index
								cpgsvp(0.08, 0.33, 0.1, 0.45); // de-dispersed waterfall
								cpgswin(taxis[0], taxis[twindow-1], freqs.front(),  freqs.back());
								cpgbox("BCN",0.0,0,"BCNV",0.0,0);
								cpgsfs(1);
								cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
								// fsrunching
								ffddw = new float[twindow * chanout](); // uniform initialization, c++11
								operations::Fscrunch(fcl.dd_fb, fcl.nchans, twindow, chanout, ffddw);
								trf[4] *= (4096/chanout);
								cpgimag(ffddw, chanout, twindow, 1, chanout, 1, twindow, 0, 3, trf);
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
								cpgimag(ffddw, chanout, fcl.nsamps, 1, chanout, 1, fcl.nsamps, 0, 3, trf);
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
						float * ffddw = NULL;
						std::string str;
						int twindow;
						float trf[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
						FloatVector freqs, taxis;
						float fac = 1e-2f, min, max, xmin, xmax;
						float xlin[2], ylin[2];
						for(int cx = 0; cx < fcl.size(); cx++) {
								if(count != 0) cpgpage();
								freqs = operations::FreqTable((float)fcl.fch1, (float)fcl.foff, fcl.nchans);
								// this function plots what is one page
								taxis = operations::TimeAxis((float)fcl.tsamp, fcl.istart, fcl.istop);
								twindow = fcl.nsamps - fcl.maxdelay;
								//////////////////////////////////////////////////
								trf[3] = freqs.back(); // fixed
								trf[0] = fcl.istart*(float)fcl.tsamp; // fixed
								trf[2] = (float)fcl.tsamp;
								trf[4] = -1.f*(float)fcl.foff; 
								cpgsci(1); // color indecx
								cpgsvp(0.1, 0.45, 0.1, 0.45); // de-dispersed waterfall
								cpgswin(taxis[0], taxis[twindow-1], freqs.back(),  freqs.front());
								cpgbox("BCN",0.0,0,"BCNV",0.0,0);
								cpgsfs(1);
								cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
								// fsrunching
								ffddw = new float[twindow * chanout](); // uniform initialization, c++11
								operations::Fscrunch(fcl.dd_fb, fcl.nchans, twindow, chanout, ffddw);
								//for(int i = 0; i < fcl.nsamps*32; i++) std::cout << ffddw[i]; std::cout << std::endl;
								trf[4] *= (4096/chanout);
								cpgimag(ffddw, chanout, twindow, 1, chanout, 1, twindow, 0, 3, trf);
								//cpglab("Time (s)", "Freq (MHz)", "De-Dispersed Waterfall");
								cpgmtxt("B",2.5,.5,0.5,std::string("Time (s)").c_str()); // group at middle
								cpgmtxt("L",4,0.5,0.5,std::string("Freq (MHz)").c_str());
								cpgmtxt("T",.3,.5,0.5, std::string("De-Dispersed Waterfall").c_str());
								// start line
								cpgsci(4);
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
								cpgsci(1); // color index
								cpgsvp(0.55, 0.9, 0.1, 0.45); // dispersed waterfall
								cpgswin(taxis[0], taxis[fcl.nsamps-1], freqs.back(),  freqs.front());
								cpgbox("BCN",0.0,0,"BCNV",0.0,0);
								cpgsfs(1);
								cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
								ffddw = new float[fcl.nsamps*chanout]();
								operations::Fscrunch(fcl.d_fb, fcl.nchans, fcl.nsamps, chanout, ffddw);
								cpgimag(ffddw, chanout, fcl.nsamps, 1, chanout, 1, fcl.nsamps, 0, 3, trf);
								//cpglab("Time (s)", "", "Dispersed Waterfall");
								cpgmtxt("B",2.5,.5,0.5,std::string("Time (s)").c_str()); // group at middle
								cpgmtxt("T",.3,.5,0.5, std::string("Dispersed Waterfall").c_str());
								delete[] ffddw;
								//////////////////////////////////////////////////
								cpgsci(1); // color index
								cpgsvp(0.1, 0.45, 0.55, 0.9); // fscrunched profile 
								//cpgswin(76, 78.5, 0..0, 0.2 );
								min = *std::min_element(fcl.dd_tim, fcl.dd_tim + twindow);
								max = *std::max_element(fcl.dd_tim, fcl.dd_tim + twindow);
								xmin = min - .1 * min;
								xmax = max + .1 * min;
								//std::cout << "Plotter limits: " << xlin[0] << " " << xlin[1] << std::endl;
								cpgswin(taxis[0],taxis[twindow -1], xmin, xmax );
								cpgbox("BCN",0.0,0,"BCNV",0.0,0);
								cpgline(twindow, taxis.data(), fcl.dd_tim); 
								//cpglab("", "Intensity (a.u)", "De-dispersed Integrated Profile");
								cpgmtxt("L",4,0.5,0.5,std::string("Intensity (a.u.)").c_str());
								cpgmtxt("T",.3,.5,0.5, std::string("De-Dispersed Integrated Profile").c_str());
								str = std::string("S/N: ") + std::to_string(fcl.sn);
								cpgmtxt("T",-1.5*charh, .99, 1.0, str.c_str());
								cpgmtxt("T",2.0,1.0,0.0,fcl.group.c_str());
								// start line
								cpgsci(4);
								cpgsls(2);
								xlin[0] = fcl.tsamp*(fcl.i0 - 3.5 * fcl.filterwidth);
								xlin[1] = fcl.tsamp*(fcl.i0 - 3.5 * fcl.filterwidth);
								ylin[0] = xmin;
								ylin[1] = xmax;
								cpgline(2,xlin, ylin);
								// end line
								xlin[0] = fcl.tsamp*(fcl.i1 + 3.5 * fcl.filterwidth);
								xlin[1] = fcl.tsamp*(fcl.i1 + 3.5 * fcl.filterwidth);
								cpgline(2,xlin, ylin);
								cpgsls(1);
								//////////////////////////////////////////////////
								cpgsci(1); // color index
								cpgsvp(0.55, 0.9, 0.55, 0.9); // Scatter 
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
								cpgmtxt("T",1.5,1.0,0.0,fcl.antenna.c_str());
								//////////////////////////////////////////////////
								count++;
								if( ! fcl.Next() ) break;
						}
				}
				CandPlot(std::string fn) {
						heat_l = {0.0, 0.2, 0.4, 0.6, 1.0};
						heat_r = {0.0, 0.5, 1.0, 1.0, 1.0};
						heat_g = {0.0, 0.0, 0.5, 1.0, 1.0};
	 					heat_b = {0.0, 0.0, 0.0, 0.3, 1.0};
						contrast = 1;
						brightness = 0.5;
						count = 0;
						filename = fn;	
						cpgbeg(0,filename.c_str(),1,1); // beginning of another journey
						charh = 0.65;
						cpgsch(charh); // character height
						cpgask(0); // go manual mode
						cpgpap (0.0,0.618); //10.0, width and aspect ratio
				}
				~CandPlot() {
						cpgend();
				}
};
class CandSummary{ 
		private:
				int count;
				int iant;
				int numants;
				timeslice imin, imax;
				std::string filename, group, antenna;
				float charh;
				float xline[2], yline[2];
		public:
				CandSummary(std::string fn) {
						count = 0;
						filename = fn;	
						cpgbeg(0,fn.c_str(),1,1); // beginning of another journey
						charh = 0.65;
						cpgsch(charh); // character height
						cpgask(0); // go manual mode
						cpgpap (0.0,0.618); //10.0, width and aspect ratio
						imin = INT_MAX;
						imax = INT_MIN;
						yline[0] = 0.0;
						yline[1] = 1.0;
				}
				~CandSummary() {
						cpgend();
				}
				void Plot(CandidateAntenna& cant) {
						if(count > 0) cpgpage();
						// CandidateAntenna is vector of CandidateList
						// CandidateList is vector of Candidate
						// sort cant
						std::sort(cant.begin(), cant.end(), [](CandidateList x, CandidateList y) { return x[0].antenna > y[0].antenna; } );
						for(CandidateList& x : cant) {
								std::sort(x.begin(), x.end(), [](Candidate x, Candidate y) { return x.i0 < y.i0; } );
								imin = imin < x.front().i0 ? imin : x.front().i0;
								imax = imax > x.back().i0 ? imax : x.back().i0;
						}
						numants = cant.size();
						int hants = numants/2;
						// Some variables I will need
						float xmin, xmax, ymin, ymax, w;
						iant = 0;
						w = (1*0.88/numants) - 0.02;
						xmin = 0.05;
						ymin = 0.05;
						xmax = 0.9;
						ymax = w + ymin;
						// the main loop
						for(CandidateList& x : cant) {
								cpgsci(1); // color index
								cpgsvp(xmin, xmax, ymin, ymax);
								//cpgswin((float)imin,(float)imax, yline[0], yline[1]);
								cpgswin(0, 50000, yline[0], yline[1]);
								cpgbox("BC",0.0,0,"BC",0.0,0);
								cpgmtxt("RV",2,.5,0.5,x[0].antenna.c_str());
								cpgmtxt("LV",2,.5,0.5,std::to_string(x.size()).c_str());
								if(iant == 0) cpgmtxt("B",1.5, .5, .5, std::string("-- Candidate Timestamp -->").c_str());
								if(iant == numants - 1) {
										// last one
										cpgmtxt("T",1.3,.5,0.5,x[0].group.c_str()); // group at middle
										cpgmtxt("T",.3,.5,0.5, std::string("Candidate Summary").c_str());
								}
								////////////////////////////////////////
								//std::sort(x.begin(), x.end(), [](Candidate x, Candidate y) { return x.i0 < y.i0; } );
								for(Candidate& cx : x) {
										xline[1] = cx.i0;
										xline[0] = cx.i0;
										std::cerr << cx.i0 << std::endl;
										cpgline(2, xline, yline);
								}
								////////////////////////////////////////
								ymin = ymax + 0.02;
								ymax += w   + 0.02 ;
								iant++;
						}
						count++;
				}
};
#endif
