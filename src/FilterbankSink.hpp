#pragma once
#include "asgard.hpp"
#include "Header.hpp"
#include <fstream>

class FilterbankSink{
 private:
	FILE * fb_fp;
	std::string filename;
	void open(std::string _filename) {

	}
	inline double sigproc_ra(double ra) {
	 float hh = (180.0f / M_PI) * (24.0f / 360.0f) * ra;
	 float mm = (hh - int(hh))  * 60.0f;
	 float ss = (mm - int(mm))  * 60.0f;
	 return ( int(hh) * 1e4 ) + ( int(mm) * 1e2 ) + ss;
	}
	inline double sigproc_dec(double dec) {
	 float dd = (180.0f / M_PI) * fabs(dec);
	 float mm = (dd - int(dd))  * 60.0f;
	 float ss = (mm - int(mm))  * 60.0f;
	 return ( int(dd) * 1e4 ) + ( int(mm) * 1e2 ) + ss;
	}
	template<typename T>
	 inline void send(const char *str, const T data) {
		// label
		send(str);
		// data
		fwrite(&data, sizeof(T), 1, fb_fp);
	 }
	inline void send(const char * str) {
	 int len = strlen(str);
	 // first goes length of string
	 fwrite(&len, sizeof(int), 1, fb_fp);
	 // writing str
	 fwrite(str, sizeof(char), len, fb_fp);
	}
	inline void send(const std::string& str) {
	 auto s = str;
	 int len = s.size();
	 // first goes length of string
	 fwrite(&len, sizeof(int), 1, fb_fp);
	 // writing str
	 fwrite(str.c_str(), sizeof(char), len, fb_fp);
	}
 public:
	FilterbankSink() {
	 fb_fp = NULL;
	}
	FilterbankSink(const std::string& _filename) :
	 filename(_filename) {
		std::cerr << "Filename: " << filename << std::endl;
		fb_fp = NULL;
	 }
	FilterbankSink(const struct Header& inhead) :
	 filename(inhead.sigproc_file) {
		std::cerr << "Filename: " << filename << std::endl;
		fb_fp = fopen(filename.c_str(), "wb+");
		Header(inhead);
	 }
	void Initialize(const struct Header& inhead) {
	 filename = std::string(inhead.sigproc_file);
	 fb_fp = fopen(filename.c_str(), "wb+");
	 Header(inhead);
	}
	void Header(const struct Header& inhead) {
	 if(fb_fp == NULL) {
		fb_fp = fopen(filename.c_str(), "wb+");
	 }
	 // dance of functions
	 send("HEADER_START");
	 // source_name
	 send("source_name"); 
	 send(inhead.name);
	 // barycentric
	 send("barycentric", 0);
	 // telescope ID
	 send("telescope_id", inhead.stationid);
	 // positions
	 send("src_raj", sigproc_ra(inhead.ra));
	 send("src_dej", sigproc_dec(inhead.dec));
	 // datatype
	 send("data_type", 1);
	 // freq
	 send("fch1", inhead.fch1);
	 send("foff", inhead.foff);
	 // data
	 send("nchans", inhead.nchans);
	 send("nbits", inhead.nbits);
	 send("nifs", 1);
	 // time
	 send("tstart",inhead.tstart);
	 send("tsamp", inhead.tsamp);
	 send("HEADER_END");
	}
	void Data(const PtrByte in, timeslice bytes_chunk) {
	 fwrite(in, 1, bytes_chunk, fb_fp);
	}
	void Close() {
	 fclose(fb_fp);
	 fb_fp = NULL;
	}
	~FilterbankSink() {
	 if(fb_fp != NULL) fclose(fb_fp);
	}
};
