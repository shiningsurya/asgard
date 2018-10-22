#include "cpgplot.h"
#include "Operations.hpp"

class Waterfall {
		private:
				std::string filename;
				std::array<float,5> heat_l;
				std::array<float,5> heat_r;
				std::array<float,5> heat_g;
				std::array<float,5> heat_b;
				float contrast,brightness;
				float tr[6];
				float charh, timestep;
				int count, nant, hant;
		public:
				Waterfall(std::string fn) {
						Waterfall(fn, 2.0);
				}
				Waterfall(std::string fn, float ts){
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
						cpgask(0); // go manual mode
						cpgpap (0.0,0.618); //10.0, width and aspect ratio
				}
				~Waterfall() {
						cpgend();
				}
				void Plot(FilterbankList fl) {
						std::string idx;
						int iant;
						float axis[] = {0.0, 2.0, 320., 360.};
						float xmin, xmax, ymin, ymax, w;
						nant = fl.size();
						hant = nant/2;
						FloatVector duration_list;
						std::for_each(fl.begin(), fl.end(), [&duration_list](Filterbank bf) { duration_list.push_back( (float)bf.duration ); } ); 
						int nsteps = *(std::max(duration_list.begin(), duration_list.end())) / timestep + 1;
						// assuming all the filterbanks have the same frequency
						FloatVector ft = operations::FreqTable(fl[0]);
						axis[2] = ft.back();
						axis[3] = ft.front();
						for(int i = 0; i < nsteps; i++) {
								if(count != 0) cpgpage();
								w = (1*0.88/nant) - 0.02;
								xmin = 0.2;
								ymin = 0.05;
								xmax = 0.9;
								ymax = w + ymin;
								axis[0] = i * timestep;
								axis[1] = axis[0] + timestep;
								iant = 0;	
								for(Filterbank f: fl) {
										cpgsci(1); // color index
										cpgsvp(xmin, xmax, ymin, ymax);
										// xmin, xmax remain the same
										// ymin ymax change
										cpgswin(axis[0],axis[1],axis[2],axis[3]);
										if(iant == 0) {
												cpgmtxt("B",2,.5,0.5,std::string("Time (s)").c_str()); // group at middle
												cpgbox("BCN",0.0,0,"BC",40.0,0);
												cpgmtxt("LV",3,0.2,0.0,std::to_string(axis[2]).c_str());
												cpgmtxt("LV",3,0.8,0.0,std::to_string(axis[3]).c_str());
										}
										else if( iant == hant) {
												cpgbox("BC",0.0,0,"BC",40.0,0);
												cpgmtxt("L",4,0.,0.5,std::string("Freq (MHz)").c_str());
												cpgmtxt("LV",3,0.2,0.0,std::to_string(axis[2]).c_str());
												cpgmtxt("LV",3,0.8,0.0,std::to_string(axis[3]).c_str());
										}
										else if(iant == nant - 1) {
												cpgmtxt("T",1,.5,0.5,f.group.c_str()); // group at middle
												idx = std::string("Slice:") + std::to_string(i) + std::string("/") + std::to_string(nsteps);
												cpgmtxt("T",.5,.0,0.0,idx.c_str());   // slice index
												cpgmtxt("RV",2,.5,0.5,f.antenna.c_str());
										}
										else {
												cpgbox("BC",0.0,0,"BC",40.0,0);
												cpgmtxt("LV",3,0.2,0.0,std::to_string(axis[2]).c_str());
												cpgmtxt("LV",3,0.8,0.0,std::to_string(axis[3]).c_str());
										}
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
				float contrast,brightness;
				float tr[6];
				// vector of candidates needs to be here for
				// scatter plot
				int count;
		public:
				void CP(Filterbank& f, CandidateList& cl) {
						float *fd=NULL, *fdd=NULL, *fbd=NULL;
						float * taxis;
						float trf[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
						std::vector<timeslice> r;
						FloatVector freqs;
						float fac = 1e-2f;
						float xlin[2], ylin[2];
						/////////////////////////////////////////////////
						/*
						 *if(f.antenna != c.antenna) 	std::cerr << "Filterbank antenna and Candidate antenna not same\n";
						 *if(f.group != c.group) std::cerr << "Filterbank group and Candidate group not same\n";
						 */
						//
						//////////////////////////////////////////////////
						for(Candidate c : cl) {
								if(count != 0) cpgpage();
								r = operations::Dedisperse(fd, fdd, fbd, f, c);
								timeslice wid = r[0];
								timeslice ni0 = r[1];
								timeslice ni1 = r[2];
								timeslice maxD = r[3];
								timeslice wmd = wid-maxD;
								freqs = operations::FreqTable(f);
								// this function plots what is one page
								operations::TimeAxis(taxis, (float)f.tsamp, ni0, ni1);
								//////////////////////////////////////////////////
								trf[0] = ni0*(float)f.tsamp;
								trf[1] = (float)f.tsamp;
								trf[3] = freqs.back();
								trf[5] = -1.f*(float)f.foff; 
								//cpgsci(1); // color index
								cpgsvp(0.1, 0.45, 0.1, 0.45); // de-dispersed waterfall
								cpgswin(taxis[0], taxis[wid-1], freqs.back(),  freqs.front());
								cpgbox("BCN",0.0,0,"BCNV",0.0,0);
								cpgsfs(1);
								cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
								cpgimag(fbd, wid,  f.nchans, 1, wid, 1, f.nchans, 0, 3, trf);
								cpglab("Time (s)", "Freq (MHz)", "De-Dispersed Waterfall");
								//////////////////////////////////////////////////
								//cpgsci(1); // color index
								cpgsvp(0.55, 0.9, 0.1, 0.45); // dispersed waterfall
								cpgswin(taxis[0], taxis[wid-1], freqs.back(),  freqs.front());
								cpgbox("BCN",0.0,0,"BCNV",0.0,0);
								cpgsfs(1);
								cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
								cpgimag(fd, wid,  f.nchans, 1, wid, 1, f.nchans, 0, 3, trf);
								cpglab("Time (s)", "", "Dispersed Waterfall");
								//////////////////////////////////////////////////
								cpgsci(1); // color index
								cpgsvp(0.1, 0.45, 0.55, 0.9); // fscrunched profile 
								//cpgswin(76, 78.5, 0..0, 0.2 );
								cpgswin(taxis[0],taxis[wmd-1], 0.1, 0.2);
								cpgbox("BCN",0.0,0,"BCNV",0.0,0);
								cpgline(wmd, taxis, fdd); 
								cpglab("", "Intensity (a.u)", "De-dispersed Integrated Profile");
								cpgmtxt("T",1.5,0.0,0.0,std::to_string(f.tstart).c_str());
								cpgmtxt("T",2.0,1.0,0.0,f.group.c_str());
								//////////////////////////////////////////////////
								cpgsci(1); // color index
								cpgsvp(0.55, 0.9, 0.55, 0.9); // Scatter 
								cpgswin(0.,2., 1, 3);
								for(int i = 0; i < cl.size(); i++) cpgcirc(log10(1e3f*f.tsamp*cl[i].filterwidth), log10((float)cl[i].dm), fac*log10(cl[i].sn));
								xlin[0] = (float)c.dm;
								xlin[1] = (float)c.dm;
								ylin[0] = 1.0f;
								ylin[1] = 1.0f;
								cpgline(2,xlin, ylin); 
								cpgbox("BCLNTS",0.0,0,"BCLVNTS",0.0,0);
								cpglab("Width (ms)", "DM (pc/cc)", "Scatter");
								cpgmtxt("T",1.5,1.0,0.0,f.antenna.c_str());
								//////////////////////////////////////////////////
								count++;
								delete[] fd;
								delete[] fdd;
								delete[] fbd;
								delete[] taxis;
								taxis = NULL;
								fd=NULL; fdd=NULL; fbd=NULL;
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
						cpgsch(.65); // character height
						cpgask(0); // go manual mode
						cpgpap (0.0,0.618); //10.0, width and aspect ratio
				}
				~CandPlot() {
						cpgend();
				}
};

