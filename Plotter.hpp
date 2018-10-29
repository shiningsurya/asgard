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
						float * juice;
						float xmin, xmax, ymin, ymax, w;
						nant = fl.size();
						hant = nant/2;
						FloatVector duration_list;
						std::for_each(fl.begin(), fl.end(), [&duration_list](Filterbank bf) { duration_list.push_back( (float)bf.duration ); } ); 
						int nsteps = *(std::max_element(duration_list.begin(), duration_list.end()));
						nsteps = (int) nsteps/timestep + 1;
						//nsteps += 1;
						// assuming all the filterbanks have the same frequency
						FloatVector ft = operations::FreqTable(fl[0]);
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
										/*
										 *trf[0] = i0*(float)f.tsamp;
										 *trf[1] = (float)f.tsamp;
										 *trf[3] = ft.back();
										 *trf[5] = -1.f*(float)f.foff; 
										 */
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
										juice = new float[wid*f.nchans];
										f.Unpack(juice, i0, wid);
										cpgsfs(1);
										cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
										//cpgimag(juice, wid,  f.nchans, 1, wid, 1, f.nchans, 0, 3, trf);
										cpgimag(juice, f.nchans, wid, 1, f.nchans, 1, wid, 0, 3, trf);
										delete[] juice;
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
								trf[3] = freqs.back(); // fixed
								trf[0] = ni0*(float)f.tsamp; // fixed
								trf[2] = (float)f.tsamp;
								trf[4] = -1.f*(float)f.foff; 
								//cpgsci(1); // color index
								cpgsvp(0.1, 0.45, 0.1, 0.45); // de-dispersed waterfall
								cpgswin(taxis[0], taxis[wid-1], freqs.back(),  freqs.front());
								cpgbox("BCN",0.0,0,"BCNV",0.0,0);
								cpgsfs(1);
								cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
								cpgimag(fbd, f.nchans, wid, 1, f.nchans, 1, wid, 0, 3, trf);
								cpglab("Time (s)", "Freq (MHz)", "De-Dispersed Waterfall");
								//////////////////////////////////////////////////
								//cpgsci(1); // color index
								cpgsvp(0.55, 0.9, 0.1, 0.45); // dispersed waterfall
								cpgswin(taxis[0], taxis[wid-1], freqs.back(),  freqs.front());
								cpgbox("BCN",0.0,0,"BCNV",0.0,0);
								cpgsfs(1);
								cpgctab (heat_l.data(), heat_r.data(), heat_g.data(), heat_b.data(), 5, contrast, brightness);
								cpgimag(fd, f.nchans, wid, 1, f.nchans, 1, wid, 0, 3, trf);
								cpglab("Time (s)", "", "Dispersed Waterfall");
								//////////////////////////////////////////////////
								cpgsci(1); // color index
								cpgsvp(0.1, 0.45, 0.55, 0.9); // fscrunched profile 
								//cpgswin(76, 78.5, 0..0, 0.2 );
								cpgswin(taxis[0],taxis[wmd-1], 0.1, 0.2);
								cpgbox("BCN",0.0,0,"BCNV",0.0,0);
								cpgline(wmd, taxis, fdd); 
								cpglab("", "Intensity (a.u)", "De-dispersed Integrated Profile");
								cpgmtxt("T",1.5,0.0,0.0,std::to_string(std::round((float)c.peak_time)).c_str());
								cpgmtxt("T",2.0,1.0,0.0,f.group.c_str());
								//////////////////////////////////////////////////
								cpgsci(1); // color index
								cpgsvp(0.55, 0.9, 0.55, 0.9); // Scatter 
								cpgswin(0.,2., 1, 3);
								for(int i = 0; i < cl.size(); i++) cpgcirc(log10(1e3f*f.tsamp*cl[i].filterwidth), log10((float)cl[i].dm), fac*log10(cl[i].sn));
								// yline
								ylin[0] = log10((float)c.dm);
								ylin[1] = log10((float)c.dm);
								xlin[0] = 0.0f;
								xlin[1] = 2.0f;
								cpgline(2,xlin, ylin); 
								// xline
								xlin[0] = log10( (float)(c.filterwidth * f.tsamp * 1e3f) );
								xlin[1] = log10( (float)(c.filterwidth * f.tsamp * 1e3f) );
								ylin[0] = 1.0f;
								ylin[1] = 3.0f;
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
/*
class CandSummary{ 
		private:
				int iant;
				std::string group;
				CandidateAntenna cant;
				int numGroups;
				StringVector groups;
		public:
				CandSummary(CandidateGroup cg) {
				}
				void Plot(CandidateGroup& cg) {
						// CandidateGroup has group & Candidate Antenna
						// CandidateAntenna is vector of CandidateList
						group = cg.first;
						cant = cg.second;	
						// sort cant
						std::sort(cant.begin(), cant.end(), [](CandidateList& x, CandidateList& y) { return x[0].antenna > y[0].antenna; } );
						// Some variables I will need
						float axis[] = {0.0, 2.0, 0., 1.};
						float xmin, xmax, ymin, ymax, w;
						int nant, hant;
						iant = 0;
						// the main loop
						for(const CandidateList& x : cant) {
								////////////////////////////////////////
								std::sort(x.begin(), x.end(), [](const Candidate& x, const Candidate& y) { return x.i0 < y.i0; } );

								////////////////////////////////////////
								nant = x.size();
								hant = nant/2;
								w = (1*0.88/nant) - 0.02;
								xmin = 0.2;
								ymin = 0.05;
								xmax = 0.9;
								ymax = w + ymin;
								cpgsci(1); // color index
								cpgsvp(xmin, xmax, ymin, ymax);
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
										if(f.isKur) cpgmtxt("T",1,.5,0.5,std::string("KUR").c_str()); // group at middle
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
								cpgmtxt("RV",2,.5,0.5,f.antenna.c_str());
								ymin = ymax + 0.02;
								ymax += w   + 0.02 ;
								iant++;



						}

				}


};
*/
