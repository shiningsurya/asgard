#pragma once
#include <asgard.hpp>
#include <bitset>
// psrdada stuff
#include "dada_def.h"
#include "dada_hdu.h"
#include "ipcio.h"
#include "ascii_header.h"
#include "tmutil.h"
#include "multilog.h"
// digitization stuff
#include <Redigitizer.hpp>

class PsrDADA {
		private:   
				multilog_t * log;
				// states
				bool isConnected;
				// key
				key_t dada_key;
				// hdu
				dada_hdu_t *hdu;
				timeslice it;
				// error state?
				bool dada_error;
				// Read header 
				char * header;
				timeslice header_size;
				// header params
				double fch1, foff, tsamp, cfreq, bandwidth;
				int nchans, nbits, nifs, npol;
				double ra, dec;
				timeslice stride;
				char utc_start_str[64], source_str[10];
				time_t utc_start;
				//
				float alpha, beta, gamma, delta;
				void pack(PtrFloat in, timeslice nsamps, unsigned char * dd) {
						timeslice ii = 0;
						if(nbits == 2) {
								for(it = 0; it < nsamps;) {
										alpha = in[it++];
										beta  = in[it++];
										gamma = in[it++];
										delta = in[it++];
										dd[ii++] = dig2bit(alpha, beta, gamma, delta,0);
								}
						}
						else if(nbits == 4) {
								for(it = 0; it < nsamps;) {
										alpha = in[it++];
										beta  = in[it++];
										dd[ii++] = dig4bit(alpha, beta,0);
								}
						}
						else if(nbits == 8) {
								for(it = 0; it < nsamps;) {
										alpha = in[it++];
										dd[ii++] = dig8bit(alpha,0);
								}
						}
				}
				void unpack(unsigned char * dd, timeslice nsamps, PtrFloat fbf) {
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
		public:
				PsrDADA() {
						dada_error = false;
						hdu = nullptr;
						isConnected = false;
				}
				PsrDADA(key_t idada) : dada_key(idada) {
						utc_start_str[63] = '\0';
						dada_error = false;
						log = multilog_open ("process_baseband",0);
						multilog_add (log, stdout);
						// create
						hdu = dada_hdu_create(log);
						// set shmkey
						dada_hdu_set_key(hdu, dada_key);
						// states
						isConnected = false;
						// DEBUG
						//nsamps = 32;
						nchans = 4;
						nbits = 4;
				}
				~PsrDADA() {
						if(hdu) {
								dada_hdu_unlock_write(hdu);
								dada_hdu_disconnect(hdu);
						}
						hdu = nullptr;
				}
				bool Connect() {
						dada_error = dada_hdu_connect(hdu) < 0;
						if(dada_error) return false;
						isConnected = true;
						return true;
				}
				bool Disconnect() {
						if(not isConnected) return true;
						dada_error = dada_hdu_disconnect(hdu)  < 0;
						if(dada_error) return false;
						return true;
				}
				bool ReadHeader() {
						dada_hdu_lock_read(hdu);
						if(!isConnected) return false;
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
						else utc_start = str2utctime(utc_start_str);// tmutil.h
						// read params
						stride = (nchans * nbits) / 8;
						tsamp /= 1e6f;
						// exit if no error
						if(dada_error) return false;
						// mark buffer clear
						ipcbuf_mark_cleared(hdu->header_block);	
						dada_hdu_unlock_read(hdu);
						return true;
				}
				bool ReadData(timeslice nsamps, PtrFloat data) {
						dada_hdu_lock_read(hdu);
						uint64_t bytes_to_read = (nsamps * nchans * nbits) / 8;
						uint64_t bytes_read = 0;
						if(ipcbuf_eod((ipcbuf_t*)hdu->data_block)) return false;
						if(!isConnected) return false;
						// actual read call
						unsigned char * packin = (unsigned char*) new char[bytes_to_read];
						bytes_read = ipcio_read(hdu->data_block, (char*)packin, bytes_to_read);
						if(bytes_read < 0) {
								std::cerr << "PsrDADA::ReadData ipcio_read fail." << std::endl;
								return false;
						}
						size_t nsamps_read = 8 * bytes_read / (nchans * nbits);
						if(nsamps_read != nsamps)
								std::cerr << "PsrDADA::ReadData read " 
												<< nsamps_read 
												<< " while expected "
												<< nsamps
												<< std::endl;
						// unpack to floats
						if(data == nullptr) data = new float[nsamps * nchans];
						unpack(packin, bytes_read, data);
						delete[] packin;
						dada_hdu_unlock_read(hdu);
						return true;
				}
				bool WriteHeader() {
						if(!isConnected) return false;
						dada_hdu_lock_write(hdu);
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
						if(ascii_header_set(header, "BANDWIDTH", "%lf", bandwidth) < 0) {
								if(ascii_header_set(header, "BW", "%lf", bandwidth) < 0) {
										std::cerr << "PsrDADA::WriteHeader BANDWIDTH write fail" << std::endl;
										dada_error = true;
								}
						}
						if(ascii_header_set(header, "CFREQ", "%lf", cfreq) < 0) {
								if(ascii_header_set(header, "FREQ", "%lf", cfreq) < 0) {
										std::cerr << "PsrDADA::WriteHeader FREQUENCY write fail" << std::endl;
										dada_error = true;
								}
						}
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
						ipcbuf_mark_filled (hdu->header_block, 4096);
						dada_hdu_unlock_write(hdu);
				}
				bool WriteData(timeslice nsamps, PtrFloat data) {
						dada_hdu_lock_write(hdu);
						uint64_t bytes_to_write = (nsamps * nchans * nbits)/8;
						uint64_t bytes_written  = 0;
						if(ipcbuf_eod((ipcbuf_t*)hdu->data_block)) return false;
						if(!isConnected) return false;
						// actual write call
						unsigned char * packout = (unsigned char*) new char[bytes_to_write];
						pack(data, nsamps*nchans, packout);
						bytes_written = ipcio_write(hdu->data_block, (char*)packout, bytes_to_write);
						if(bytes_written < 0) {
								std::cerr << "PsrDADA::WriteData ipcio_write fail." << std::endl;
								return false;
						}
						dada_hdu_unlock_write(hdu);
				}
};
