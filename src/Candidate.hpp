#include "asgard.hpp"
#ifndef CANDIDATE_H
#define CANDIDATE_H
class Candidate {
		public:
				std::string antenna, group;
				float sn, dm;
				long unsigned peak_index, dm_index;
				double peak_time;
				int filterwidth, ngiant;
				timeslice i0,i1;
				double tsamp, width;
				CandidateList matches;
		Candidate(std::string line, double ts, std::string gp, std::string ant) {
				antenna = ant;
				group = gp;
				std::vector<std::string> b;
				boost::split(b, line, boost::is_any_of(" \t"),boost::token_compress_on);
				if(b.size() ==  2) {
						sn = 0;
				}
				else {
						//
						if(b[0] == std::string("inf")) sn = 1e5;
						else sn = std::stof(b[0]);
						peak_index = std::stoul(b[1]);
						peak_time = std::stod(b[2]);
						filterwidth = std::stoi(b[3]);
						dm_index = std::stoul(b[4]);
						dm = std::stof(b[5]);
						ngiant = std::stoi(b[6]);
						i0 = std::stoul(b[7]);
						i1 = std::stoul(b[8]);
						ts = tsamp;
						width = tsamp * (i1 - i0);
						}
		}
		// printing
		friend std::ostream& operator<< (std::ostream& os, const Filterbank& fb);
		
		bool Overlap(Candidate other, float ddm, float dwi) {
				// collapses candidates with ddm and dwi
				float x = abs( (dm - other.dm) / (dm + other.dm) );	
				double y = abs( (width - other.width) / pow(width * other.width, .5)); 
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

std::ostream& operator<< (std::ostream& os, const Candidate& cd){
		os << "I0: " << cd.i0 << std::endl;
		os << "I1: " << cd.i1 << std::endl;
		os << "Wd: " << cd.filterwidth << std::endl;
		os << "W: " << cd.width << std::endl;
		os << "SN: " << cd.sn << std::endl;
		os << "DM: " << cd.dm << std::endl;
		return os;
}

CandidateList ReadCandidates(const std::string st, double ts) {
		CandidateList ret;
		std::string line;
		if(ts == 0.0) ts = TSAMP;
		fs::ifstream ifs(st);
		std::string group = GetGroup(st);
		std::string ant = GetAntenna(st);
		while(std::getline(ifs,line)) {
		  if (line.length() > )
      ret.push_back(Candidate(line, ts, group, ant));
  }
		ifs.close();
		return ret;
}

/*
 *CandidateList ReadCandidates(fs::directory_entry& cl, double& ts) {
 *        CandidateList ret;
 *        std::string line;
 *        if(ts == 0.0) ts = TSAMP;
 *        auto x = ReadCandidates(il.path().string(), ts);
 *        ret.insert(ret.end(), x.begin(), x.begin());
 *        return ret;
 *}
 */


void Coincidence(CandidateList& cg, float ddm, float dwi) {
		// Coincide all the candidates
		std::vector<timeslice> end_times;
		std::for_each(cg.begin(),cg.end(), [&end_times](Candidate x)
						{ end_times.push_back(x.i1); } );
		std::sort(end_times.begin(), end_times.end());
		std::sort(cg.begin(), cg.end(), [](const Candidate x, const Candidate y) 
						{ return x.i1 < y.i1; } ); 
		// Assuming all the Candidates have same timesamp
		double ts = cg[0].tsamp;
		for(timeslice i : end_times){ 
				i *= ts;
		}
		//
		CandidateList pcands, tcands;
		CandidateList::iterator it0, it1;
		float tslice = 1;
		timeslice nslice = (end_times[end_times.size()-1]/tslice) + 1;
		it0 = cg.begin(); // initilizing
		for(timeslice i = 0; i < nslice; i++) {
				it1 = std::lower_bound(cg.begin(), cg.end(),tslice*(i+1)/ts, [](Candidate x, float ts) { return x.i1 < ts; } );
				std::copy(it0, it1, tcands.begin());
				//
				for(Candidate cc : tcands) {
						for(Candidate dd : tcands) {
								if((cc).Overlap(dd, ddm, dwi))
										(cc).match_append(dd);
						}
						for(Candidate dd: pcands) {
								if((cc).Overlap(dd, ddm, dwi))
										(cc).match_append(dd);
						}
				}
				pcands = tcands;
				it0 = it1;
		}
}
#endif
