#pragma once
#include <asgard.hpp>
// psrdada stuff
#include "dada_def.h"
#include "dada_hdu.h"
#include "ipcio.h"
#include "ascii_header.h"
#include "tmutil.h"
#include "multilog.h"
// digitization stuff
#include <Redigitizer.hpp>

struct DADAHeader {
		// positions
		double ra, dec;
		// frequency
		double fch1, foff, cfreq, bandwidth;
		// time
		double tsamp;
		// memory
		int nbits, nchans, nifs, npol;
		// strings
		char utc_start_str[64], source_str[10];
};

class PsrDADA {
		private:   
				// logging
				multilog_t * log;
				// DADA
				key_t dada_key;
				dada_hdu_t *hdu;
				timeslice it;
				// error state
				bool dada_error;
				// states
				bool read_lock;
				bool write_lock;
				// Read header 
				char * header;
				timeslice header_size;
				// header params
				timeslice nsamps;
				double fch1, foff, tsamp, cfreq, bandwidth;
				int nchans, nbits, nifs, npol;
				char utc_start_str[64], source_str[10];
				double ra, dec;
				// sizes
				timeslice stride, bytes_chunk, bytes_stride;
				timeslice sample_chunk;
				// PACK/UNPACK
				float alpha, beta, gamma, delta;
				void pack(PtrFloat in, int numants, timeslice nsamps, unsigned char * dd) {
						timeslice ii = 0;
						if(nbits == 2) {
								for(it = 0; it < nsamps;) {
										alpha = in[it++] / numants;
										beta  = in[it++] / numants;
										gamma = in[it++] / numants;
										delta = in[it++] / numants;
										dd[ii++] = dig2bit(alpha, beta, gamma, delta,0);
								}
						}
						else if(nbits == 4) {
								for(it = 0; it < nsamps;) {
										alpha = in[it++] / numants;
										beta  = in[it++] / numants;
										dd[ii++] = dig4bit(alpha, beta,0);
								}
						}
						else if(nbits == 8) {
								for(it = 0; it < nsamps;) {
										alpha = in[it++] / numants;
										dd[ii++] = dig8bit(alpha,0);
								}
						}
				}
				void unpack(PtrByte dd, timeslice nsamps, PtrFloat fbf) {
						timeslice ii = 0;
						unsigned char dc;
						if(nbits == 2) {
								for(it = 0; it < nsamps; it++) {
										dc = dd[it]; // read one character
										// one character has 4 samples
										fbf[ii++] = (float) ((dc & HI2BITS) >> 6); 
										fbf[ii++] = (float) ((dc & UPMED2BITS) >> 4); 
										fbf[ii++] = (float) ((dc & LOMED2BITS) >> 2); 
										fbf[ii++] = (float) (dc & LO2BITS); 
								}
						}
						else if(nbits == 4) {
								for(it = 0; it < nsamps; it++) {
										dc = dd[it]; // read one character
										// one character has 2 samples
										fbf[ii++] = (float) ((dc & UP4BITS) >> 4); 
										fbf[ii++] = (float) ((dc & LO4BITS) >> 0); 
								}
						} else if(nbits == 8) {
								for(it = 0; it < nsamps; it++) {
										dc = dd[it]; // read one character
										// one character has 1 samples
										fbf[ii++] = (float) dc;
								}
						}
				}
				bool Connect() {
						dada_error = dada_hdu_connect(hdu) < 0;
						if(dada_error) return false;
						return true;
				}
				bool Disconnect() {
						if(hdu == nullptr) return true;
						dada_error = dada_hdu_disconnect(hdu)  < 0;
						if(dada_error) return false;
						return true;
				}
				/********************************************
				 * Rationale for making Lock functions private.
				 * All locking/unlocking happens internally now.
				 * lock at the start of function and 
				 * unlock at the end.
				 * ******************************************/
				bool ReadLock(bool x) {
						bool ret;
						std::cerr << "PsrDADA::ReadLock before key=" << dada_key << std::endl;
						if(x) {
						   // Requested lock
						   if(read_lock) ret =  true;
						   else ret = dada_hdu_lock_read(hdu);
						   read_lock = true;
						}
						else {
						   // Requested unlock
						   if(read_lock) ret = dada_hdu_unlock_read(hdu);
						   else ret = true;
						   read_lock = false;
						}
						std::cerr << "PsrDADA::ReadLock after key=" << dada_key << std::endl;
						return ret;
				}
				bool WriteLock(bool x) {
						bool ret;
						std::cerr << "PsrDADA::WriteLock before key=" << dada_key << std::endl;
						if(x) {
						   // Requested lock
						   if(write_lock) ret = true;
						   else ret = dada_hdu_lock_write(hdu);
						   write_lock = true;
						}
						else {
						   // Requested unlock
						   if(write_lock) ret = dada_hdu_unlock_write(hdu);
						   else ret = true;
						   write_lock = false;
						}
						std::cerr << "PsrDADA::WriteLock after key=" << dada_key << std::endl;
						return ret;
				}
		public:
				PsrDADA() {
						// chill
				}
				PsrDADA(key_t dada_key_, timeslice nsamps_, int nchans_, int nbits_) : 
						dada_key(dada_key_), 
						nsamps(nsamps_),
						nchans(nchans_), 
						nbits(nbits_)					
		{
						std::cerr << "PsrDADA::ctor key=" << dada_key << std::endl;
						dada_error = false;
						sample_chunk = nsamps * nchans;
						bytes_chunk = nsamps * nchans * nbits / 8;
						bytes_stride = (nchans * nbits) / 8;
						// logging
						log = multilog_open ("dadadada",0);
						multilog_add (log, stdout);
						// DADA
						hdu = dada_hdu_create(log);
						dada_hdu_set_key(hdu, dada_key);
						// Connection
						Connect();
						// state initialize
						read_lock = false;
						write_lock = false;
				}
				PsrDADA& operator=(PsrDADA&& other) {
						std::cerr << "PsrDADA::move_assignment key=" << other.dada_key << std::endl;
						// ctor args
						nsamps = other.nsamps;
						nchans = other.nchans;
						nbits  = other.nbits;
						// sharing
						sample_chunk = other.sample_chunk;
						bytes_chunk  = other.bytes_chunk;
						bytes_stride = other.bytes_stride;
						// logging
						log = multilog_open ("dadadadada",0);
						multilog_add (log, stdout);
						// DADA
						hdu = other.hdu;
						other.hdu = nullptr;
						dada_key = other.dada_key;
						other.dada_key = 0;
						// state initialize
						read_lock = other.read_lock;
						write_lock = other.write_lock;
						other.read_lock = false;
						other.write_lock = false;
						return *this;
				}
				~PsrDADA() { 
						std::cerr << "PsrDADA::dtor key=" << dada_key << std::endl;
						// unlocks
						ReadLock(false);
						WriteLock(false);
						// Disconnection
						Disconnect();
						// log close
						multilog_close(log);
				}
				const timeslice GetByteChunkSize() const {
						return bytes_chunk;
				}
				const timeslice GetStride() const {
						return bytes_stride;
				}
				bool ReadHeader() {
						ReadLock(true);
						std::cerr << "PsrDADA::ReadHeader key=" << dada_key << std::endl;
						// blocking read
						header = ipcbuf_get_next_read(hdu->header_block, &header_size);
						if(!header) {
								std::cerr << "PsrDADA::ReadHeader fail" << std::endl;
								dada_error = true;
								return false;
						}
						// get params
						if(ascii_header_get(header, "NCHAN", "%d", &nchans) < 0) {
								std::cerr << "PsrDADA::ReadHeader NCHAN read fail" << std::endl;
								dada_error = true;
						}
						if(ascii_header_get(header, "BANDWIDTH", "%lf", &bandwidth) < 0) {
								if(ascii_header_get(header, "BW", "%f", &bandwidth) < 0) {
										std::cerr << "PsrDADA::ReadHeader BANDWIDTH read fail" << std::endl;
										dada_error = true;
								}
						}
						if(ascii_header_get(header, "CFREQ", "%lf", &cfreq) < 0) {
								if(ascii_header_get(header, "FREQ", "%f", &cfreq) < 0) {
										std::cerr << "PsrDADA::ReadHeader FREQUENCY read fail" << std::endl;
										dada_error = true;
								}
						}
						if (!dada_error) {
								double start_freq = cfreq - (bandwidth / 2);
								double chan_width = bandwidth / nchans;
								double first_chan_cfreq = start_freq + (chan_width / 2);

								fch1 = first_chan_cfreq;
								foff = chan_width;
						}
						if(ascii_header_get(header, "NPOL", "%d", &npol) < 0) {
								std::cerr << "PsrDADA::ReadHeader NPOL read fail" << std::endl;
								dada_error = true;
						}
						if(ascii_header_get(header, "NBIT", "%d", &nbits) < 0) {
								std::cerr << "PsrDADA::ReadHeader NBIT read fail" << std::endl;
								dada_error = true;
						}
						if(ascii_header_get(header, "TSAMP", "%lf", &tsamp) < 0) {
								std::cerr << "PsrDADA::ReadHeader TSAMP read fail" << std::endl;
								dada_error = true;
						}
						if(ascii_header_get(header, "RA", "%lf", &ra) < 0) {
								std::cerr << "PsrDADA::ReadHeader RA read fail" << std::endl;
								dada_error = true;
						}
						if(ascii_header_get(header, "DEC", "%lf", &dec) < 0) {
								std::cerr << "PsrDADA::ReadHeader DEC read fail" << std::endl;
								dada_error = true;
						}
						if(ascii_header_get(header, "UTC_START", "%s", utc_start_str) < 0) {
								std::cerr << "PsrDADA::ReadHeader UTC_START read fail" << std::endl;
								dada_error = true;
						}
						// read params
						tsamp /= 1e6f;
						// exit if no error
						if(dada_error) return false;
						// mark buffer clear
						ipcbuf_mark_cleared(hdu->header_block);	
						ReadLock(false);
						return true;
				}
				timeslice ReadData(PtrFloat data, PtrByte packin) {
						ReadLock(true);
						std::cerr << "PsrDADA::ReadData key=" << dada_key << std::endl;
						if(ipcbuf_eod((ipcbuf_t*)hdu->data_block)) return -1;
						// Initialization
						if(packin == nullptr) packin = ( PtrByte ) new unsigned char[bytes_chunk];
						// READ -- this blocks
						timeslice chunk_read = ipcio_read(hdu->data_block, (char*)packin, bytes_chunk);
						if(chunk_read == -1) {
								std::cerr << "PsrDADA::ReadData ipcio_read fail." << std::endl;
								return -1;
						}
						timeslice nsamps_read = chunk_read / bytes_stride;
						// Initialization
						if(data == nullptr) data = new float[nsamps_read * nchans];
						if(chunk_read != bytes_chunk)
								std::cerr << "PsrDADA::ReadData read " 
												<< chunk_read 
												<< " bytes while expected "
												<< bytes_chunk
												<< " bytes."
												<< std::endl;
						if(nsamps_read != nsamps)
								std::cerr << "PsrDADA::ReadData read " 
												<< nsamps_read 
												<< " samples while expected "
												<< nsamps
												<< " samples."
												<< std::endl;
						// unpack to floats
						unpack(packin, chunk_read, data);
						ReadLock(false);
						return chunk_read;
				}
				bool WriteHeader() {
						WriteLock(true);
						std::cerr << "PsrDADA::WriteHeader key=" << dada_key << std::endl;
						header = ipcbuf_get_next_write(hdu->header_block);
						if(!header) {
								std::cerr << "PsrDADA::WriteHeader fail" << std::endl;
								dada_error = true;
								return false;
						}
						// set params
						if(ascii_header_set(header, "NCHAN", "%d", nchans) < 0) {
								std::cerr << "PsrDADA::WriteHeader NCHAN write fail" << std::endl;
								dada_error = true;
						}
						std::cerr << "PsrDADA::WriteHeader nchan"  << std::endl;
						if(ascii_header_set(header, "BANDWIDTH", "%lf", bandwidth) < 0) {
								if(ascii_header_set(header, "BW", "%lf", bandwidth) < 0) {
										std::cerr << "PsrDADA::WriteHeader BANDWIDTH write fail" << std::endl;
										dada_error = true;
								}
						}
						std::cerr << "PsrDADA::WriteHeader bandwidth"  << std::endl;
						if(ascii_header_set(header, "CFREQ", "%lf", cfreq) < 0) {
								if(ascii_header_set(header, "FREQ", "%lf", cfreq) < 0) {
										std::cerr << "PsrDADA::WriteHeader FREQUENCY write fail" << std::endl;
										dada_error = true;
								}
						}
						std::cerr << "PsrDADA::WriteHeader cfreq"  << std::endl;
						if(ascii_header_set(header, "NPOL", "%d", npol) < 0) {
								std::cerr << "PsrDADA::WriteHeader NPOL write fail" << std::endl;
								dada_error = true;
						}
						if(ascii_header_set(header, "NBIT", "%d", nbits) < 0) {
								std::cerr << "PsrDADA::WriteHeader NBIT write fail" << std::endl;
								dada_error = true;
						}
						if(ascii_header_set(header, "TSAMP", "%lf", tsamp) < 0) {
								std::cerr << "PsrDADA::WriteHeader TSAMP write fail" << std::endl;
								dada_error = true;
						}
						if(ascii_header_set(header, "RA", "%lf", ra) < 0) {
								std::cerr << "PsrDADA::WriteHeader RA write fail" << std::endl;
								dada_error = true;
						}
						if(ascii_header_set(header, "DEC", "%lf", dec) < 0) {
								std::cerr << "PsrDADA::WriteHeader DEC write fail" << std::endl;
								dada_error = true;
						}
						if(ascii_header_set(header, "UTC_START", "%s", utc_start_str) < 0) {
								std::cerr << "PsrDADA::WriteHeader UTC_START write fail" << std::endl;
								dada_error = true;
						}
						std::cerr << "PsrDADA::WriteHeader UTC"  << std::endl;
						ipcbuf_mark_filled (hdu->header_block, header_size);
						WriteLock(false);
						return true;
				}
				timeslice WriteData(PtrFloat data, PtrByte packout, int numants) {
						WriteLock(true);
						std::cerr << "PsrDADA::WriteData key=" << dada_key << std::endl;
						if(packout == nullptr) packout = ( PtrByte ) new unsigned char[bytes_chunk];
						// NOT SURE ABOUT EOD while WRITING?
						//if(ipcbuf_eod((ipcbuf_t*)hdu->data_block)) return -1;
						// PACK and WRITE
						pack(data, numants, sample_chunk,  packout);
						timeslice bytes_written = ipcio_write(hdu->data_block, (char*)packout, bytes_chunk);
						if(bytes_written < 0) {
								std::cerr << "PsrDADA::WriteData ipcio_write fail." << std::endl;
								return false;
						}
						WriteLock(false);
						return bytes_written;
				}
				void PrintHeader() {
						// positions
						std::cout << "RA     " << ra  << std::endl;
						std::cout << "DEC    " << dec << std::endl;
						// frequency
						std::cout << "FCH1   " << fch1 << std::endl;
						std::cout << "FOFF   " << foff << std::endl;
						std::cout << "CFREQ  " << cfreq << std::endl;
						std::cout << "BANDW  " << bandwidth<< std::endl;
						// time
						std::cout << "TSAMP  " << tsamp << std::endl;
						// memory
						std::cout << "NCHANS " << nchans<< std::endl;
						std::cout << "NBITS  " << nbits << std::endl;
						std::cout << "NIFS   " << nifs<< std::endl;
						std::cout << "NPOL   " << npol << std::endl;
						// strings
						std::cout << "UTC    " << utc_start_str << std::endl;
						std::cout << "SOURCE " << source_str << std::endl;
				}
				struct DADAHeader GetHeader() {
						struct DADAHeader ret;
						ret.ra       = ra;
						ret.dec      = dec;
						ret.tsamp    = tsamp;
						ret.nchans   = nchans;
						ret.nbits    = nbits;
						ret.nifs     = nifs;
						ret.npol     = npol;
						ret.fch1     = fch1;
						ret.foff     = foff;
						ret.cfreq    = cfreq;
						ret.bandwidth = bandwidth;
						strcpy(ret.utc_start_str, utc_start_str);
						strcpy(ret.source_str, source_str);
						return ret;
				}
				bool SetHeader(const struct DADAHeader& in) {
						ra       = in.ra;
						dec      = in.dec;
						tsamp    = in.tsamp;
						nchans   = in.nchans;
						nbits    = in.nbits;
						nifs     = in.nifs;
						npol     = in.npol;
						fch1     = in.fch1;
						foff     = in.foff;
						cfreq    = in.cfreq;
						bandwidth = in.bandwidth;
						strcpy(utc_start_str, in.utc_start_str);
						strcpy(source_str, in.source_str);
						return true;
				}
				bool Scrub() {
						return true;
				}
};
