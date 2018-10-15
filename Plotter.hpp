#include "cpgplot.h"
#include "Operations.hpp"

class Waterfall {
		private:
				std::string filename,group;
				int nant,hnant;
				unsigned long int numslice;
				float charh;
				
		public:
				Waterfall(std::string fn, int NANT, timeslice nslice,std::string basename){
						numslice = nslice; // number of pgplot pages
						nant = NANT; // Number of antennas
						filename = fn; // output plot file name
						group = basename; // group
						hnant  = NANT/2;
						//
						charh = .65; // pgplot char height
				}
				void Plot(timeslice slice) {
						//if(dat.size() != nant) {
								//std::cout << "Input data doesn't contain all the antennas?" <<std::endl;
						//}
						cpgbeg(0,filename.c_str(),1,1); // beginning of cpgplot
						cpgsch(.65); // character height
						cpgask(0); // we don't want manual mode
						cpgpap (0.0,0.618); //10.0, width and aspect ratio
						float axis[] = {0.0, 2.0, 320., 360.};
						float xmin, xmax, ymin, ymax, w;
						w = (1*0.88/nant) - 0.02;
						xmin = 0.2;
						ymin = 0.05;
						xmax = 0.9;
						ymax = w + ymin;
						int ha = (int)nant/2;
						//
						for(int i = 0; i < nant; i++) {
								//if(i == ha) {
										//cpgpage();	
										//xmin = 0.1;
										//ymin = 0.1;
										//xmax = 0.95;
										//ymax = w + 0.05;
								//}
								cpgsci(1); // color index
								cpgsvp(xmin, xmax, ymin, ymax);
								//cpgpage();
								//std::cout << w << " " << ymin << " " << ymax << " " << ymax - ymin << std::endl;
								// xmin, xmax remain the same
								// ymin ymax change
								cpgswin(axis[0],axis[1],axis[2],axis[3]);
								if(i == 0) {
										cpgmtxt("B",2,.5,0.5,std::string("Time (s)").c_str()); // group at middle
										cpgbox("BCN",0.0,0,"BC",40.0,0);
										cpgmtxt("LV",3,0.2,0.0,std::string("320").c_str());
										cpgmtxt("LV",3,0.8,0.0,std::string("360").c_str());
								}
								else if( i == ha) {
										cpgbox("BC",0.0,0,"BC",40.0,0);
										cpgmtxt("L",4,0.,0.5,std::string("Freq (MHz)").c_str());
										cpgmtxt("LV",3,0.2,0.0,std::string("320").c_str());
										cpgmtxt("LV",3,0.8,0.0,std::string("360").c_str());
								}
								else {
										cpgbox("BC",0.0,0,"BC",40.0,0);
										cpgmtxt("LV",3,0.2,0.0,std::string("320").c_str());
										cpgmtxt("LV",3,0.8,0.0,std::string("360").c_str());
								}
								//if(i == ha) cpglab("","Freq [MHz]","");
								//cpgmtxt("B",-.5*(ymin+ymax)/charh,.9999999,0.0,(std::string("  ea")+std::to_string(i+1)+std::string("  ")).c_str());
								cpgmtxt("RV",2,.5,0.5,(std::string("ea")+std::to_string(i+1)).c_str());
								if(i == nant - 1) {
										cpgmtxt("T",1,.5,0.5,group.c_str()); // group at middle
										std::string idx = std::string("Slice:") + std::to_string(slice) + std::string("/") + std::to_string(numslice);
										cpgmtxt("T",.5,.0,0.0,idx.c_str());   // slice index
								}
								//
								ymin = ymax + 0.02;
								ymax += w   + 0.02 ;
						}
						cpgend();
				}
};
class CandPlot {
		private:
				std::string filename;
				float tr[6];
				// vector of candidates needs to be here for
				// scatter plot
				int count;
		public:
				CandPlot(std::string fn) {
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
				void CP(Filterbank& f, Candidate& c) {
						if(count != 0) cpgpage();
						float *fd=NULL, *fdd=NULL;
						std::vector<timeslice> vt = operations::ExtractJuice(f, c, fd, 5);
						timeslice ni0 = vt[1];
						timeslice ni1 = vt[2];
						timeslice wid = vt[0];
						if(fd == NULL) std::cerr << "Why tf are you NULL?\n";
						fdd = new float[wid*f.nchans];
						operations::Dedisperse(fd, fdd, c.dm, f.nchans, f.tsamp, f.fch1, f.foff, wid*f.nchans);
						// this function plots what is one page
						//////////////////////////////////////////////////
						float trf[] = {ni0*(float)f.tsamp, (float)f.tsamp, 0.0, 320, 0.0, (360.f-320.f)/f.nchans}; 
						float heat_l[] = {0.0, 0.2, 0.4, 0.6, 1.0};
						float heat_r[] = {0.0, 0.5, 1.0, 1.0, 1.0};
						float heat_g[] = {0.0, 0.0, 0.5, 1.0, 1.0};
						float heat_b[] = {0.0, 0.0, 0.0, 0.3, 1.0};
						float contrast = 1,brightness = 0.5;
						//////////////////////////////////////////////////
						cpgsci(1); // color index
						cpgsvp(0.1, 0.45, 0.1, 0.45); // de-dispersed waterfall
						cpgswin(ni0*f.tsamp +.5*f.tsamp, ni1*f.tsamp - .5*f.tsamp, 320, 360);
						cpgbox("BCN",0.0,0,"BCNV",0.0,0);
						cpgsfs(1);
						cpgctab (heat_l, heat_r, heat_g, heat_b, 5, contrast, brightness);
						cpgimag(fdd, wid,  f.nchans, 1, wid, 1, f.nchans, 0, 3, trf);
						cpglab("Time (s)", "Freq (MHz)", "De-Dispersed Waterfall");
						//////////////////////////////////////////////////
						cpgsci(1); // color index
						cpgsvp(0.55, 0.9, 0.1, 0.45); // dispersed waterfall
						cpgswin(ni0*f.tsamp +.5*f.tsamp, ni1*f.tsamp - .5*f.tsamp, 320, 360);
						cpgbox("BCN",0.0,0,"BCNV",0.0,0);
						cpgsfs(1);
						cpgctab (heat_l, heat_r, heat_g, heat_b, 5, contrast, brightness);
						cpgimag(fd, wid,  f.nchans, 1, wid, 1, f.nchans, 0, 3, trf);
						cpglab("Time (s)", "", "Dispersed Waterfall");
						//////////////////////////////////////////////////
						cpgsci(1); // color index
						cpgsvp(0.1, 0.45, 0.55, 0.9); // fscrunched profile 
						cpgswin(20.,24., 320, 340);
						cpgbox("BCN",0.0,0,"BCNV",0.0,0);
						cpglab("", "Intensity (a.u)", "De-dispersed Integrated Profile");
						cpgmtxt("T",1.5,0.0,0.0,"UTC 2018-10-10 20:17:22");
						cpgmtxt("T",2.0,1.0,0.0,"GROUP");
						//////////////////////////////////////////////////
						cpgsci(1); // color index
						cpgsvp(0.55, 0.9, 0.55, 0.9); // Scatter 
						cpgswin(20.,24., 2, 3);
						cpgbox("BCN",0.0,0,"BCLVNTS",0.0,0);
						cpglab("", "DM (pc/cc)", "Scatter");
						cpgmtxt("T",1.5,1.0,0.0,"EAXX");
						//////////////////////////////////////////////////
						count++;
						delete[] fd;
						delete[] fdd;
				}
};

