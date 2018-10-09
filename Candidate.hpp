#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

fs = namespace boost::filesystem;

class Candidate {
		public:
				float sn, dm;
				long unsigned peak_index, dm_index;
				double peak_time;
				int filterwidth, ngiant;
				long unsigned i0,i1;
				double tsamp, width;
				std::vector<Candidate> matches;

		Candidate(std::string line, double ts) {
				std::vector<std::string> b;
				boost::split(b, line, boost::is_any_of(" ");
				if(b.size() != 9) throw BadCandidateException;
				//
				sn = std::atof(b[0]);
				if(sn == "inf") sn = 1e5;
				peak_index = std::stoul(b[1]);
				peak_time = std::atod(b[2]);
				filterwidth = std::atoi(b[3]);
				dm_index = std::stoul(b[4]);
				dm = std::atof(b[5]);
				ngiant = std::atoi(b[6]);
				i0 = std::stoul(b[7]);
				i1 = std::stoul(b[8]);
				ts = tsamp;
				width = tsamp * (i1 - i0);
		}
		std::string operator<< {
				std::string ret;
				ret << "I0: " << i0 << std::endl;
				ret << "I1: " << i1 << std::endl;
				ret << "Wd: " << filterwidth << std::endl;
				ret << "SN: " << sn << std::endl;
				ret << "DM: " << dm << std::endl;
		}
		
		bool Overlap(Candidate other, float ddm, float dwi) {
				// collapses candidates with ddm and dwi
				float x = abs( (dm - other.dm) / (dm + other.dm) );	
				double y = abs( (width - other.width) / (width * other.width)^.5); 
				//
				if(x > ddm) return false;
				if(y > dwi) return false;
				if(i0 < other.i0) return other.i0 < i1;
				else return i0 < other.i1;
		}
		void match_append(Candidate x) {
				matches.push_back(x);				
		}
};

class CandGroups {
		private:
				std::vector<Candidate> cg;
				double tsamp;
				bool coincided;
		public:
				CandGroups(std::vector<fs::directory_entry> list, double ts) {
						tsamp = ts;
						for(std::vector<fs::directory_entry>::iterator il : list) {
								std::string line;
								fs::ifstream ifs(il.path());	
								while(getline(ifs, line))	cg.push_back(Candidate(line),ts);
						}
						coincided = false;
				}
				

				void Coincidence(float ddm, float dwi) {
						// Coincide all the candidates
						//
						std::vector<long unsigned int> end_times;
						std::for_each(cg.begin(),cg.end(), [](Candidate x)
										{ end_times.push_back(x.i1) } );
						std::sort(end_times.begin(), end_times.end());
						std::sort(cg.begin(), cg.end(), [](const Candidate x, const Candidate y) 
										{ return x.i1 < y.i1 } ); 
						for(auto i : end_times){ 
								*i *= ts;
						}
						//
						std::vector<Candidate> pcands, tcands;
						std::vector<Candidate>::iterator it0, it1;
						float tslice = 1;
						long unsigned int nslice = (end_times[end_times.size()-1]/tslice) + 1;
						it0 = cg.begin(); // initilizing
						for(long unsigned int i = 0; i < nslice; i++) {
								it1 = std::lower_bound(cg.begin(), cg.end(),tslice*(i+1)/ts, [](Candidate x, float ts) { return x.i1 < ts } );
								std::copy(it0, it1, tcands.begin());
								//
								for(cc : tcands) {
										for(dd : tcands) {
												if((*cc).Overlap(*dd, ddm, dwi))
														(*cc).match_append(*dd);
										}
										for(dd: pcands) {
												if((*cc).Overlap(*dd, ddm, dwi))
														(*cc).match_append(*dd);
										}
								}
								pcands = tcands;
								it0 = it1;
						}
						coincided = true;
				}
				bool isCoincided() { return coincided; }
};
