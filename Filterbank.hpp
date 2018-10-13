//#include "asgard.hpp"
//#include "dedisp.h" // cuda based dedispersion

/*
 * Forgive me Father, for I am about to sin. 
 * I am going to shamelessly copy from sigproc code 
 * and put it here after C++-izing the code
 */
#include <iostream>
#include <fstream>
class Filterbank {
		public:
				std::string filename; // <-- this is mine
				std::string rawdatafile, source_name;
				int machine_id, telescope_id, data_type, nchans, nbits, nifs, scan_number,
					barycentric,pulsarcentric; /* these two added Aug 20, 2004 DRL */
				double tstart,mjdobs,tsamp,fch1,foff,refdm,az_start,za_start,src_raj,src_dej;
				double gal_l,gal_b,header_tobs,raw_fch1,raw_foff;
				int nbeams, ibeam;
				/* added 20 December 2000    JMC */
				double srcl,srcb;
				double ast0, lst0;
				long wapp_scan_number;
				std::string project, culprits;
				double analog_power[2];
				/* added frequency table for use with non-contiguous data */
				double frequency_table[4096]; /* note limited number of channels */
				long int npuls; /* added for binary pulse profile format */
				// printing
				friend std::ostream& operator<< (std::ostream& os, const Filterbank& fb);
};
std::ostream& operator<< (std::ostream& os, const Filterbank& fb) {
		os << "Filename: " << fb.filename << std::endl;
		os << "Source Name: " << fb.source_name << std::endl;
		return os;
}


class FilterbankReader {
		public:
				FilterbankReader() {
						r = new char[36];
						nbytes = 0;
				}
				~FilterbankReader() {
						delete[] r;
				}
				int Read(Filterbank& fb, std::string ifile) {
						inputfile.open(ifile,std::ifstream::binary);
						fb.filename = ifile;
						if(!inputfile.is_open()) {
								std::cerr << "Unable to read file" << std::endl;
								return 1;				
						}
						if(!read_header(fb)) {
								std::cerr << "Unable to read header of FIL: " << ifile << std::endl;
								return 1;
						}
						inputfile.close();
						return 0;
				}
		private:
				////////////////// lvalue rvalue shit
				//std::string filename; // <-- this is mine
				std::string source_name;
				int telescope_id, data_type, nchan, nbits, nifs, barycentric; /* these two added Aug 20, 2004 DRL */
				double tstart,tsamp,fch1,foff,src_raj,src_dej;
				////////////////// lvalue rvalue shit
				std::ifstream inputfile;
				std::string filename;
				char *r;
				int nbytes;
				int ifread(int size) {
						inputfile.read(r, size);
						std::cout << "In ifread :: r: " << std::to_string(size) << std::endl;
						return std::atoi(r);
				}
				std::string sfread(int size) {
						inputfile.read(r, size);
						return std::string(r);
				}
				double dfread(int size) {
						inputfile.read(r, size);
						return std::stod(std::string(r));
				}
				std::string get_string() {
						int nchar;
						nchar = ifread(sizeof(int));
						nbytes = sizeof(int);
						std::cout << "In get_string :: nchar: " << std::to_string(nchar) << std::endl;
						if (inputfile.eof()) return std::string(""); 
						if (nchar > 80 || nchar < 1) return std::string("");
						nbytes += nchar;
						return sfread(nchar); 
						//str[nchar]='\0';
				}
				// There are so many of strings_equal
				bool strings_equal(std::string one, const char * x) {
						std::string two(x);
						return one == two;
				}
				int read_header(Filterbank& f) {
						inputfile.seekg(0, inputfile.beg);
						std::string str, message;
						int itmp,nbytes,totalbytes,expecting_rawdatafile=0,expecting_source_name=0; 
						int expecting_frequency_table=0,channel_index;
						/* try to read in the first line of the header */
						str = get_string();
						if (str != std::string("HEADER_START")) {
								/* the data file is not in standard format, rewind and return */
								//rewind(inputfile);
								std::cout << "what you are coming here?\n" << str.length() << std::endl;
								inputfile.seekg(0, inputfile.beg);
								return 0;
						}
						/* store total number of bytes read so far */
						totalbytes=nbytes;

						/* loop over and read remaining header lines until HEADER_END reached */
						while (1) {
								str = get_string();
								if (strings_equal(str,"HEADER_END")) break;
								totalbytes+=nbytes;
								if (strings_equal(str,"source_name")) {
										expecting_source_name=1;
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
										//exit(1);
								} 
								if (totalbytes != inputfile.gcount()) {
										std::cerr << "ERROR: Header bytes does not equal file position" << std::endl;
										std::cerr << "String was: " + str << std::endl;
										//fprintf(stderr,"       header: %d file: %d\n",totalbytes,ftell(inputfile));
										//exit(1);
								}
						} 
						/* add on last header string */
						totalbytes+=nbytes;
						if (totalbytes != inputfile.gcount()) {
								std::cerr << "ERROR: Header bytes does not equal file position" << std::endl;
								std::cerr << "String was: " + str << std::endl;
								//exit(1);
						}
						/* return total number of bytes read */
						return totalbytes;
				}

};


