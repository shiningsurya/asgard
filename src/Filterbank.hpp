//#include "asgard.hpp"
#pragma once
#ifndef ASGARD_H
#include "asgard.hpp"
#endif
#ifndef FILTERBANK_H
#define FILTERBANK_H
#include <boost/iostreams/device/mapped_file.hpp>
#define HI2BITS 192
#define UPMED2BITS 48
#define LOMED2BITS 12
#define LO2BITS 3
#define LO4BITS 15 
#define UP4BITS 240
#ifdef _DEBUG
#include<bitset>
#include<fstream>
#endif
#include <Redigitizer.hpp>
#include <numeric>
namespace bios = boost::iostreams;
//#include <fstream>
//typedef long unsigned int timeslice;
class Filterbank {
		public:
				Filterbank() {
						mmap = false;
						//OneTimeMMap();
				}
				~Filterbank() {
						if(mmap) fbdata.close();
				}
				std::string filename, group; // <-- this is mine
				bool isKur;
				std::string source_name, antenna;
				int telescope_id, data_type, nchans, nbits, nifs, barycentric; /* these two added Aug 20, 2004 DRL */
				int b_per_spectrum;
				double duration, tstart,tsamp,fch1,foff,src_raj,src_dej, tstop;
				int headersize;
				long int datasize, totalsamp;
				unsigned int bmin, bmax;
#ifdef AG_RUNNING
				FloatVector last;
#endif 
				// printing
				friend std::ostream& operator<< (std::ostream& os, const Filterbank& fb);
				// Read data
				/*
					*void Unpack(float** fbf, double ttstart, double tduration) {
					*        timeslice tts = ttstart/tsamp;
					*        timeslice ttd = tduration/tsamp;
					*        Unpack(fbf, tts, ttd);
					*}
					*/
				void Unpack(float* fbf, timeslice nstart, timeslice nsamp) {
						if(!mmap) OneTimeMMap(); 
						// unpack td slice starting from ts
						if(nstart + nsamp > totalsamp) {
								std::cerr << "Asking to unpack more than what is there\n";
								nsamp = totalsamp - nstart;
								std::cerr << "Changing nsamp accordingly\n";
						}
						int ichan = 0;
						int ii = 0;
						timeslice b0 = (nstart * b_per_spectrum) + headersize;
						timeslice b1 = (nsamp * b_per_spectrum) + b0;
						dd = fbdata.data(); // <-- this is data
						if(nbits == 2) {
								for(timeslice it = b0; it < b1;it++) {
										dc = (unsigned char) dd[it];
										// one character has 4 samples
										fbf[ii++] = (float) (dc & LO2BITS); 
										fbf[ii++] = (float) ((dc & LOMED2BITS) >> 2); 
										fbf[ii++] = (float) ((dc & UPMED2BITS) >> 4); 
										fbf[ii++] = (float) ((dc & HI2BITS) >> 6); 
								}
						}
						else if(nbits == 4) {
								for(timeslice it = b0; it < b1;it++) {
										dc = (unsigned char) dd[it];
										// one character has 2 samples
										fbf[ii++] = (float) ((dc & LO4BITS) >> 0); 
										fbf[ii++] = (float) ((dc & UP4BITS) >> 4); 
								}
						}
						else if(nbits == 8) {
								for(timeslice it = b0; it < b1;it++) {
										dc = (unsigned char) dd[it];
										// one character has 1 samples
										fbf[ii++] = (float) dc;
								}
						}
						// return is 2d array of size td * nchans
#ifdef AG_RUNNING
      PtrFloat ret = fbf;
      timeslice datasamps = nsamp * nchans;
      timeslice lastsamps = 0;
						// the running mean idea
						// two pass mean, std estimate
						float _rmean = 0.0f;
						float _rstd = 0.0f;
						std::for_each(ret, ret + datasamps, [&_rmean](const float& xx) { _rmean += xx; });
//						_rmean += std::accumulate(last.begin(), last.end(), 0);
						_rmean /= (datasamps + last.size());
						std::for_each(ret, ret + datasamps, [&_rstd, &_rmean](const float& xx) { _rstd += pow(xx - _rmean, 2); });
//						std::for_each(last.begin(), last.end(), [&_rstd, &_rmean](const float& xx) { _rstd += pow(xx - _rmean, 2); });
						_rstd = sqrt(_rstd / (datasamps - 1)); // Bessel correction
						std::cout << " reading _rmean=" <<_rmean << " _rstd=" << _rstd << std::endl;
//						last.clear(); last.resize(lastsamps);
//						std::copy(ret + (datasamps - lastsamps), ret + datasamps, std::back_inserter(last));
#endif
				}
		private:
				bool mmap;
				const char *dd; 
				unsigned char dc;
				bios::mapped_file_source fbdata;
				void OneTimeMMap() {
						fbdata.open(filename);		
						mmap = true;
				}
};
std::ostream& operator<< (std::ostream& os, const Filterbank& fb) {
		os << "Filename: " << fb.filename << std::endl;
		os << "Group: " << fb.group << std::endl;
		os << "Source Name: " << fb.source_name << std::endl;
		os << "RA (HH:MM:SS)" << fb.src_raj << std::endl;
		os << "DEC (HH:MM:SS)" << fb.src_dej << std::endl;
		os << "Header size: " << fb.headersize << std::endl;
		os << "Num-chan: " << fb.nchans << std::endl;
		os << "Tstart: " << fb.tstart << std::endl;
		os << "Tsamp: " << fb.tsamp << std::endl;
		os << "Nbits: " << fb.nbits << std::endl;
		os << "BMin:" << fb.bmin << std::endl;
		os << "BMax:" << fb.bmax << std::endl;
		os << "Nifs: " << fb.nifs << std::endl;
		os << "Nsamples: " << fb.totalsamp << std::endl;
		os << "Duration:(s) " << fb.duration << std::endl;
		os << "Antenna:" << fb.antenna << std::endl;
		if(fb.isKur) os << "Kurtosis-ed: " << std::string("Yes") << std::endl;
		else os << "Kurtosis-ed: " << std::string("No") << std::endl;
		return os;
}


class FilterbankReader {
		public:
				FilterbankReader() {
						r = new char[80];
				}
				~FilterbankReader() {
						delete[] r;
				}
				int Read(Filterbank& fb, std::string ifile) {
						totalbytes = 0;
						inputfile = fopen(ifile.c_str(),"rb");
						fb.filename = ifile;
						filename = ifile;
						if(!inputfile) {
								std::cerr << "Unable to open file" << std::endl;
								return 1;				
						}
						fb.headersize = read_header(fb);
						if(!fb.headersize) {
								std::cerr << "Unable to read header of FIL: " << ifile << std::endl;
								return 1;
						}
						fb.b_per_spectrum = (fb.nbits * fb.nchans * fb.nifs)/8;
						fb.totalsamp = fb.datasize/(long int)fb.b_per_spectrum;
						fb.duration = fb.totalsamp * fb.tsamp;
						fb.antenna = GetAntenna(ifile);
						fb.isKur = QueryKurtosis(ifile);
						fb.group = GetGroup(ifile);
						fb.tstop = fb.tstart + ( fb.duration / 86400.0f );
						fb.bmin = 0;
						fb.bmax = pow(2, fb.nbits) - 1;
						fclose(inputfile);
						return 0;
				}
		private:
				////////////////// lvalue rvalue shit
				//std::string filename; // <-- this is mine
				std::string source_name;
				int telescope_id, data_type, nchan, nbits, nifs, barycentric; /* these two added Aug 20, 2004 DRL */
				double tstart,tsamp,fch1,foff,src_raj,src_dej;
				////////////////// lvalue rvalue shit
				FILE* inputfile;
				std::string filename;
				char *r;
				int totalbytes;
				int ifread(int size) {
						int ret;
						fread(&ret, sizeof(int), 1, inputfile);
						//inputfile.read(&r[0], size);
						//std::cout << "In ifread :: r: " << std::to_string(ret) << std::endl;
						//return std::atoi(r);
						return ret;
				}
				std::string sfread(int size) {
						fread(r, size, 1, inputfile);
						return std::string(r,r+size);
				}
				double dfread(int size) {
						double ret;
						fread(&ret, sizeof(double), 1, inputfile);
						return ret; 
				}
				std::string get_string() {
						int nchar;
						nchar = ifread(sizeof(int));
						totalbytes += sizeof(int);
						//std::cout << "In get_string :: nchar: " << std::to_string(nchar) << std::endl;
						if (feof(inputfile)) return std::string(""); 
						if (nchar > 80 || nchar < 1) return std::string("");
						//std::cout << "In get_string 2 :: nchar: " << std::to_string(nchar) << std::endl;
						totalbytes += nchar;
						return sfread(nchar); 
						//str[nchar]='\0';
				}
				// There are so many of strings_equal
				bool strings_equal(std::string one, const char * x) {
						std::string two(x);
						return one == two;
				}
				int read_header(Filterbank& f) {
						std::string str, message;
						int expecting_source_name=0; 
						/* try to read in the first line of the header */
						str = get_string();
						if (str != std::string("HEADER_START")) {
								/* the data file is not in standard format, rewind and return */
								//rewind(inputfile);
								//std::cout << "what you are coming here?\n" << str.length() << std::endl;
								rewind(inputfile);
								return 0;
						}
						/* loop over and read remaining header lines until HEADER_END reached */
						while (1) {
								str = get_string();
								if (strings_equal(str,"HEADER_END")) break;
								//totalbytes+=str.length();
								if (strings_equal(str,"source_name")) {
										expecting_source_name=1;
										//totalbytes+=str.length();
								} else if (strings_equal(str,"src_raj")) {
										f.src_raj = dfread(sizeof(f.src_raj));
										totalbytes+=sizeof(f.src_raj);
								} else if (strings_equal(str,"src_dej")) {
										f.src_dej = dfread(sizeof(f.src_dej));
										totalbytes+=sizeof(f.src_dej);
								} else if (strings_equal(str,"tstart")) {
										f.tstart = dfread(sizeof(f.tstart));
										totalbytes+=sizeof(f.tstart);
								} else if (strings_equal(str,"tsamp")) {
										f.tsamp = dfread(sizeof(f.tsamp));
										totalbytes+=sizeof(f.tsamp);
										/*
											*} else if (strings_equal(str,"period")) {
											*        fread(f.period,sizeof(period),1,inputfile); // <-- This is not there
											*        totalbytes+=sizeof(period);
											*/
								} else if (strings_equal(str,"fch1")) {
										f.fch1 = dfread(sizeof(f.fch1));
										totalbytes+=sizeof(f.fch1);
								} else if (strings_equal(str,"foff")) {
										f.foff = dfread(sizeof(f.foff));
										totalbytes+=sizeof(f.foff);
								} else if (strings_equal(str,"nchans")) {
										f.nchans = ifread(sizeof(f.nchans));
										totalbytes+=sizeof(f.nchans);
								} else if (strings_equal(str,"telescope_id")) {
										f.telescope_id = ifread(sizeof(f.telescope_id));
										totalbytes+=sizeof(f.telescope_id);
								} else if (strings_equal(str,"data_type")) {
										f.data_type = ifread(sizeof(f.data_type));
										totalbytes+=sizeof(f.data_type);
								} else if (strings_equal(str,"nbits")) {
										f.nbits = ifread(sizeof(f.nbits));
										totalbytes+=sizeof(f.nbits);
								} else if (strings_equal(str,"barycentric")) {
										f.barycentric = ifread(sizeof(f.barycentric));
										totalbytes+=sizeof(f.barycentric);
										/*
											*} else if (strings_equal(str,"nbins")) {
											*        fread(f.nbins,sizeof(nbins),1,inputfile); // <-- This is not there
											*        totalbytes+=sizeof(nbins);
											*} else if (strings_equal(str,"nsamples")) {
											*        [> read this one only for backwards compatibility <]
											*        fread(f.itmp,sizeof(itmp),1,inputfile);// <-- This is not there
											*        totalbytes+=sizeof(itmp);
											*/
								} else if (strings_equal(str,"nifs")) {
										f.nifs = ifread(sizeof(f.nifs));
										totalbytes+=sizeof(f.nifs);
										/*
											*} else if (strings_equal(str,"signed")) {
											*        fread(f.isign,sizeof(isign),1,inputfile); // This is not there
											*        totalbytes+=sizeof(isign);
											*/
								} else if (expecting_source_name) {
										//strcpy(source_name,str);
										f.source_name = str;
										expecting_source_name=0;
								} else {
										std::string message("read_header - unknown parameter: ");
										message += str;
										//fprintf(stderr,"ERROR: %s\n",message);
										std::cerr << "ERROR in FilterbankReader " << message << std::endl;
										std::cerr << "While reading " << filename << std::endl;  
										exit(1);
								} 
								if (totalbytes != ftell(inputfile)) {
										std::cerr << "ERROR: Header bytes does not equal file position while reading" << std::endl;
										std::cerr << "Totalbytes was: " << std::to_string(totalbytes) << std::endl;
										std::cerr << "ftell was: " << std::to_string(ftell(inputfile)) << std::endl;
										std::cerr << "String was: " + str << std::endl;
										//fprintf(stderr,"       header: %d file: %d\n",totalbytes,ftell(inputfile));
										exit(1);
								}
						} 
						/* add on last header string */
						if (totalbytes != ftell(inputfile)) {
								std::cerr << "ERROR: Header bytes does not equal file position after reading" << std::endl;
								std::cerr << "Totalbytes was: " << std::to_string(totalbytes) << std::endl;
								std::cerr << "ftell was: " << std::to_string(ftell(inputfile)) << std::endl;
								std::cerr << "String was: " + str << std::endl;
								//exit(1);
						}
						/*
							*find total size of the file
							*/
						fseek(inputfile, 0L, SEEK_END);
						f.datasize = ftell(inputfile) - totalbytes;
						/* return total number of bytes read */
						return totalbytes;
				}

};

struct FilterbankHeader {
		std::string source_name;
		double src_raj, src_dej;
		int telescope_id, data_type;
		double fch1, foff, tsamp;
		int nchans, nbits, nifs;
};
class FilterbankWriter {
		private:
#ifdef _DEBUG
				std::ofstream debug_file;
				long debnum;
				// debug 
#endif // _DEBUG
				unsigned char A, B, C, D, ucval;
				int ival;
				PtrByte dd;
				bios::mapped_file_sink fbdata;
				timeslice it;
				float alpha, beta, gamma, delta;
				int nbits;
#ifdef AG_RUNNING
    FloatVector last;
#endif
				void _copy_fbdata(PtrFloat ret, timeslice datasamps, int nant) {
						float rmean = 0.0f, rstd = 0.0f;
      rmean = 1.12f * nant;
      rstd  = 0.92f * sqrt(nant);
#ifdef AG_RUNNING
      std::transform(ret, ret + datasamps, ret, [&nant](float& xx) { return xx / nant; });
						// the running mean idea
						// two pass mean, std estimate
						timeslice lastsamps = 0;
						float _rmean = 0.0f;
						float _rstd = 0.0f;
						std::for_each(ret, ret + datasamps, [&_rmean](const float& xx) { _rmean += xx; });
//					_rmean += std::accumulate(last.begin(), last.end(), 0);
						_rmean /= (datasamps + last.size());
						std::for_each(ret, ret + datasamps, [&_rstd, &_rmean](const float& xx) { _rstd += pow(xx - _rmean, 2); });
//						std::for_each(last.begin(), last.end(), [&_rstd, &_rmean](const float& xx) { _rstd += pow(xx - _rmean, 2); });
						_rstd = sqrt(_rstd / (datasamps - 1)); // Bessel correction
						std::cout << " Writing _rmean=" <<_rmean << " _rstd=" << _rstd << std::endl;
						std::cout << " Writing  rmean=" << rmean << " rstd=" << rstd << std::endl;
//						last.clear(); last.resize(lastsamps);
//						std::copy(ret + (datasamps - lastsamps), ret + datasamps, std::back_inserter(last));
						rmean = _rmean;
						rstd = _rstd;
						// I am dumb
						// I forgot to divide by nant
#endif
						// writing
						if(nbits == 2) {
								for(timeslice i = 0; i < datasamps;) {
										alpha = (ret[i++] - rmean) / rstd;
										beta  = (ret[i++] - rmean) / rstd;
										gamma = (ret[i++] - rmean) / rstd;
										delta = (ret[i++] - rmean) / rstd;
										dd[it++] = dig2bit(alpha, beta, gamma, delta); 
								}
						}
						else if(nbits == 4) {
								for(timeslice i = 0; i < datasamps;) {
										alpha = (ret[i++] - rmean) / rstd;
										beta  = (ret[i++] - rmean) / rstd;
										dd[it++] = dig4bit(alpha, beta);
								}
						}
						else if(nbits == 8) {
								for(timeslice i = 0; i < datasamps;) {
										alpha = (ret[i++] - rmean) / rstd;
										dd[it++] = dig8bit(alpha);
								}
						}
				}
				void _copy_header(Filterbank& from, double tst, int nb) {
						it = 0;
						dd = (unsigned char*)fbdata.data();
						// ^ initializes 
						send_string("HEADER_START");
						send_string("source_name");
						send_string(from.source_name);
						send_double("src_raj",from.src_raj);
						send_double("src_dej",from.src_dej);
						send_int("telescope_id",from.telescope_id);
						send_int("data_type",from.data_type);
						send_double("fch1",from.fch1);
						send_double("foff",from.foff);
						send_int("nchans",from.nchans);
						send_int("nbits",nb);
						nbits = nb;
						send_double("tstart",tst);
						send_double("tsamp",from.tsamp);
						send_int("nifs",from.nifs);
						send_int("barycentric", 1);
						send_string("HEADER_END");
				}
				void _copy_header(struct FilterbankHeader& from, double tst) {
						it = 0;
						dd = (unsigned char*)fbdata.data();
						// ^ initializes 
						send_string("HEADER_START");
						send_string("source_name");
						send_string(from.source_name);
						send_double("src_raj",from.src_raj);
						send_double("src_dej",from.src_dej);
						send_int("telescope_id",from.telescope_id);
						send_int("data_type",from.data_type);
						send_double("fch1",from.fch1);
						send_double("foff",from.foff);
						send_int("nchans",from.nchans);
						send_int("nbits",from.nbits);
						nbits = from.nbits;
						send_double("tstart",tst);
						send_double("tsamp",from.tsamp);
						send_int("nifs",from.nifs);
						send_int("barycentric", 1);
						send_string("HEADER_END");
				}
				void fdwrite( const void* ddd, int size, int num,  unsigned char* idd ) {
						//std::copy(ddd, ddd + size*num, idd);
						std::size_t sn = size * num;
						std::memcpy(idd + it, ddd, sn);
						it += sn;
				}
				void send_string(const std::string str) {
						int len = str.length();
						fdwrite(&len, sizeof(int), 1, dd);
						fdwrite(str.c_str(), sizeof(char), len, dd);
				}
				void send_string(const char * str) {
						int len = strlen(str);
						fdwrite(&len, sizeof(int), 1, dd);
						fdwrite(str, sizeof(char), len, dd);
				}
				void send_double (const char *name, const double db) {
						send_string(name);
						fdwrite(&db,sizeof(double),1,dd);
				}
				void send_int(const char *name, const int integer) {
						send_string(name);
						fdwrite(&integer,sizeof(int),1,dd);
				}
		public:
				FilterbankWriter() {
						// chill empty initialization
#ifdef _DEBUG
						debug_file.open("debug.file");
						debnum = 2048L;
#endif // _DEBUG
				}
				timeslice Initialize(std::string fname, Filterbank& takeHeader, double dur, double tstrt, int nbits_) {
						// dur is the length
						// tstrt is the starting time
						// take headers from f and account for dur and tstrt
						//
						//outputfile = fopen(fname.c_str(),"wb");
						std::size_t flength = takeHeader.headersize + ( dur/takeHeader.tsamp *takeHeader.nchans * nbits_)/8;
						// by 4 because one byte yields 4 samples
						bios::mapped_file_params param;
						param.path = fname;
						param.new_file_size = flength;
						param.flags = bios::mapped_file::mapmode::readwrite;
						fbdata.open(param);	
						if(!fbdata.is_open()) {
								std::cout << " PRAY!!!!!! " << std::endl;
						}
						// copy header
						_copy_header(takeHeader, tstrt, nbits_);
						return it;
				}
				void WriteFBdata(PtrFloat da, timeslice ib, timeslice datasamps) {
						if(it != ib) std::cerr << "Writing non-aligned FB data\n";
						_copy_fbdata(da, datasamps, 1);
				}
				void WriteFBdata(PtrFloat da, timeslice ib, timeslice datasamps, int nant) {
						if(it != ib) std::cerr << "Writing non-aligned FB data\n";
						_copy_fbdata(da, datasamps, nant);
				}
				struct FilterbankHeader GetHeader(Filterbank& from) {
						struct FilterbankHeader ret;
						// copy values
						ret.src_raj = from.src_raj;
						ret.source_name = from.source_name;
						ret.src_raj = from.src_raj;
						ret.src_dej = from.src_dej;
						ret.telescope_id = from.telescope_id;
						ret.data_type = from.data_type;
						ret.fch1 = from.fch1;
						ret.foff = from.foff;
						ret.nchans = from.nchans;
						ret.nbits = from.nbits;
						nbits = from.nbits;
						ret.tsamp = from.tsamp;
						ret.nifs = from.nifs;
						return ret;
				}
				void Close() {
						// close the mmap file here
						fbdata.close();
#ifdef _DEBUG
						debug_file.close();
#endif  // _DEBUG
				}
};
#endif
