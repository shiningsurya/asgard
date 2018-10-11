#include <iostream>
#include "cpgplot.h"

typedef unsigned long int slicetype;


class Waterfall {
		private:
				std::string filename,group;
				int nant,hnant;
				unsigned long int numslice;
				float charh;
				
		public:
				Waterfall(std::string fn, int NANT, slicetype nslice,std::string basename){
						numslice = nslice; // number of pgplot pages
						nant = NANT; // Number of antennas
						filename = fn; // output plot file name
						group = basename; // group
						hnant  = NANT/2;
						//
						charh = .65; // pgplot char height
				}
				void Plot(slicetype slice) {
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
				// vector of candidates needs to be here for
				// scatter plot
		public:
				CandPlot(std::string fn) {
						filename = fn;	
						cpgbeg(0,filename.c_str(),1,1); // beginning of another journey
						cpgsch(.65); // character height
						cpgask(0); // go manual mode
						cpgpap (0.0,0.618); //10.0, width and aspect ratio
				}
				~CandPlot() {
						cpgend();
				}
				void CP() {
						// this function plots what is one page
						//////////////////////////////////////////////////
						//////////////////////////////////////////////////
						cpgsci(1); // color index
						cpgsvp(0.1, 0.45, 0.1, 0.45); // de-dispersed waterfall
						cpgswin(20.,24., 320, 340);
						cpgbox("BCN",0.0,0,"BCNV",0.0,0);
						cpglab("Time (s)", "Freq (MHz)", "De-Dispersed Waterfall");
						//////////////////////////////////////////////////
						cpgsci(1); // color index
						cpgsvp(0.55, 0.9, 0.1, 0.45); // dispersed waterfall
						cpgswin(20.,24., 320, 340);
						cpgbox("BCN",0.0,0,"BCNV",0.0,0);
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
				}
};



int main() {
		//Waterfall wf(std::string("wut.ps/cps"),16/1,10,std::string("base"));
		//wf.Plot(1);
		CandPlot cd(std::string("cacaca.ps/vps"));
		cd.CP();
		return 0;
}
