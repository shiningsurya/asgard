/***
 * Trigger DADA buffer
 *
 *
 * **/
#pragma once
#define __STDC_FORMAT_MACROS 1
#include <asgard.hpp>
#include <exception>
// psrdada stuff
#include "dada_def.h"
#include "dada_hdu.h"
#include "ipcio.h"
#include "ascii_header.h"
#include "tmutil.h"
#include "multilog.h"
// digitization stuff
#include <Redigitizer.hpp>
// Header stuff
#include <Header.hpp>
#include <inttypes.h>

constexpr char TRIGLOGDIR[] = "/home/vlite-master/surya/logs";

class TrigDADA {
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
	static uint64_t h_readtimes, h_writetimes;
	static uint64_t d_readtimes, d_writetimes;
	// Read header 
	char * header;
	timeslice header_size;
	// header params
	timeslice nsamps;
	// station
	int stationid;
	// strings
	char utc_start_str[64];
	char name[16];
	char sigproc_file[256];
	// positions
	double ra, dec;
	// frequency
	double fch1, foff, cfreq, bandwidth;
	// time
	double tsamp, tstart, epoch;
	// memory
	int nbits, nchans, nifs, npol;
	// sizes
	timeslice stride, bytes_chunk, bytes_stride;
	timeslice sample_chunk;
	// PACK/UNPACK
	float alpha, beta, gamma, delta;
 void pack(PtrFloat ret, int numants, timeslice nsamps, PtrByte dd) {
   timeslice ii = 0;
   float rmean = 0.0f, rstd = 0.0f;
   float bmax = pow(2,nbits) -1; 
   rmean = 0.5 * bmax * numants;
   rstd  = sqrt(numants);
   if (nbits == 8)      rstd *= 33.818;
   else if (nbits == 4) rstd *= 3.137;
   if(nbits == 2) {
     for(it = 0; it < nsamps;) {
       alpha = (ret[it++] - rmean) / rstd;
       beta  = (ret[it++] - rmean) / rstd;
       gamma = (ret[it++] - rmean) / rstd;
       delta = (ret[it++] - rmean) / rstd;
       dd[ii++] = dig2bit(alpha, beta, gamma, delta);
     }
   }
   else if(nbits == 4) {
     for(it = 0; it < nsamps;) {
       alpha = (ret[it++] - rmean) / rstd;
       beta  = (ret[it++] - rmean) / rstd;
       dd[ii++] = dig4bit(alpha, beta);
     }
   }
   else if(nbits == 8) {
     for(it = 0; it < nsamps;) {
       alpha = (ret[it++] - rmean) / rstd;
       dd[ii++] = dig8bit(alpha);
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
   return dada_error;
 }
 bool Disconnect() {
   if(hdu == nullptr) return true;
   dada_error = dada_hdu_disconnect(hdu)  < 1;
   if(dada_error) return false;
   return true;
 }
 public:
  TrigDADA () {
    // chill
    log = nullptr;
    hdu = nullptr;
    dada_key = 0;
    read_lock = false;
    write_lock = false;
  }
  TrigDADA (key_t dkey_) : 
    dada_key (dkey_) {
    // logging
    log = multilog_open ("triggerdada",0);
    multilog_add (log, stdout);
    // construction
    multilog(log, LOG_INFO,  "TrigDADA::ctor key=%x\n", dada_key);
    dada_error = false;
    // DADA
    hdu = dada_hdu_create(log);
    dada_hdu_set_key(hdu, dada_key);
    // Connection
    if(Connect()) exit(1);
    // state initialize
    read_lock = false;
    write_lock = false;
  }
 ~TrigDADA() { 
   multilog(log, LOG_INFO,  "TrigDADA::dtor key=%x\n", dada_key);
   // Disconnection
   Disconnect();
   // log close
   if(log != NULL) multilog_close(log);
 }
 TrigDADA& operator=(TrigDADA&& other) {
   multilog(other.log, LOG_INFO,  "TrigDADA::move_assignment key=%x\n", other.dada_key);
   // logging
   std::swap (log, other.log);
   // DADA
   std::swap (hdu, other.hdu);
   std::swap (dada_key, other.dada_key);
   // state initialize
   std::swap (read_lock, other.read_lock);
   std::swap (write_lock, other.write_lock);
   // destroy other
   other.~TrigDADA();
   return *this;
 }
 bool ReadLock(bool x) {
   bool ret;
   multilog(log,LOG_INFO,"TrigDADA::ReadLock key=%x Before\n",dada_key);
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
   multilog(log,LOG_INFO,"TrigDADA::ReadLock key=%x After\n",dada_key);
   return ret;
 }
 bool WriteLock(bool x) {
   bool ret;
   multilog(log,LOG_INFO,"TrigDADA::WriteLock key=%x Before\n",dada_key);
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
   multilog(log,LOG_INFO,"TrigDADA::WriteLock key=%x After\n",dada_key);
   return ret;
 }
 // --------------------
 // those functions
 // max possible array is initialized on the heap
 // array is reused
 bool ReadHeader() {
   // blocking read
   header = ipcbuf_get_next_read(hdu->header_block, &header_size);
   if(!header) {
     multilog(log,LOG_INFO,"TrigDADA::ReadHeader key=%x GetNextRead failed\n",dada_key);
     dada_error = true;
     return false;
   }
   // get params
   if(ascii_header_get(header, "STATIONID", "%d", &stationid) < 0) {
     std::cerr << "TrigDADA::ReadHeader STATIONID write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_get(header, "NCHAN", "%d", &nchans) < 0) {
     std::cerr << "TrigDADA::ReadHeader NCHAN read fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_get(header, "BANDWIDTH", "%lf", &bandwidth) < 0) {
     if(ascii_header_get(header, "BW", "%f", &bandwidth) < 0) {
       std::cerr << "TrigDADA::ReadHeader BANDWIDTH read fail" << std::endl;
       dada_error = true;
     }
   }
   if(ascii_header_get(header, "CFREQ", "%lf", &cfreq) < 0) {
     if(ascii_header_get(header, "FREQ", "%f", &cfreq) < 0) {
       std::cerr << "TrigDADA::ReadHeader FREQUENCY read fail" << std::endl;
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
     std::cerr << "TrigDADA::ReadHeader NPOL read fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_get(header, "NBIT", "%d", &nbits) < 0) {
     std::cerr << "TrigDADA::ReadHeader NBIT read fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_get(header, "TSAMP", "%lf", &tsamp) < 0) {
     std::cerr << "TrigDADA::ReadHeader TSAMP read fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_get(header, "SCANSTART", "%lf", &tstart) < 0) {
     std::cerr << "TrigDADA::ReadHeader SCANSTART read fail" << std::endl;
     dada_error = true;
   }
   // unix epoch
   if(ascii_header_get(header, "UNIXEPOCH", "%lf", &epoch) < 0) {
     std::cerr << "TrigDADA::ReadHeader UNIXEPOCH read fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_get(header, "RA", "%lf", &ra) < 0) {
     std::cerr << "TrigDADA::ReadHeader RA read fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_get(header, "DEC", "%lf", &dec) < 0) {
     std::cerr << "TrigDADA::ReadHeader DEC read fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_get(header, "NAME", "%s", name) < 0) {
     std::cerr << "TrigDADA::ReadHeader NAME read fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_get(header, "UTC_START", "%s", utc_start_str) < 0) {
     std::cerr << "TrigDADA::ReadHeader UTC_START read fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_get(header, "SIGPROC_FILE", "%s", sigproc_file) < 0) {
     std::cerr << "TrigDADA::ReadHeader SIGPROC_FILE read fail" << std::endl;
     dada_error = true;
   }
   // mark buffer clear
   ipcbuf_mark_cleared(hdu->header_block);	
   multilog(log,LOG_INFO,"TrigDADA::ReadHeader key=%x h_readtimes=%" PRIu64 "\n",dada_key,h_readtimes++);
   return true;
 }
 timeslice ReadData(PtrByte packin, timeslice bytes_chunk) {
   if(ipcbuf_eod((ipcbuf_t*)hdu->data_block)) {
     return -1;
   }
   timeslice chunk_read = ipcio_read(hdu->data_block, (char*)packin, bytes_chunk);
   if(chunk_read == -1) {
     multilog(log,LOG_INFO,"TrigDADA::ReadData key=%x ipcio_read failed\n", dada_key);
   }
   if(chunk_read != bytes_chunk)
     std::cerr << "TrigDADA::ReadData read " 
       << std::dec << chunk_read 
       << " bytes while expected "
       << bytes_chunk
       << " bytes."
       << std::endl;
   multilog(log,LOG_INFO,"TrigDADA::ReadData key=%x d_readtimes=%" PRIu64 "\n",dada_key,d_readtimes++);
   return chunk_read;
 }
 bool WriteHeader() {
   header = ipcbuf_get_next_write(hdu->header_block);
   if(!header) {
     multilog(log,LOG_INFO,"TrigDADA::WriteHeader key=%x GetNextWrite failed\n",dada_key);
     dada_error = true;
     return false;
   }
   // set params
   if(ascii_header_set(header, "STATIONID", "%d", stationid) < 0) {
     std::cerr << "TrigDADA::WriteHeader STATIONID write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_set(header, "NCHAN", "%d", nchans) < 0) {
     std::cerr << "TrigDADA::WriteHeader NCHAN write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_set(header, "BANDWIDTH", "%lf", bandwidth) < 0) {
     if(ascii_header_set(header, "BW", "%f", bandwidth) < 0) {
       std::cerr << "TrigDADA::WriteHeader BANDWIDTH write fail" << std::endl;
       dada_error = true;
     }
   }
   if(ascii_header_set(header, "CFREQ", "%lf", cfreq) < 0) {
     if(ascii_header_set(header, "FREQ", "%f", cfreq) < 0) {
       std::cerr << "TrigDADA::WriteHeader FREQUENCY write fail" << std::endl;
       dada_error = true;
     }
   }
   if(ascii_header_set(header, "NPOL", "%d", npol) < 0) {
     std::cerr << "TrigDADA::WriteHeader NPOL write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_set(header, "NBIT", "%d", nbits) < 0) {
     std::cerr << "TrigDADA::WriteHeader NBIT write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_set(header, "TSAMP", "%lf", tsamp) < 0) {
     std::cerr << "TrigDADA::WriteHeader TSAMP write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_set(header, "SCANSTART", "%lf", tstart) < 0) {
     std::cerr << "TrigDADA::WriteHeader SCANSTART write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_set(header, "UNIXEPOCH", "%lf", epoch) < 0) {
     std::cerr << "TrigDADA::WriteHeader UNIXEPOCH write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_set(header, "RA", "%lf", ra) < 0) {
     std::cerr << "TrigDADA::WriteHeader RA write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_set(header, "DEC", "%lf", dec) < 0) {
     std::cerr << "TrigDADA::WriteHeader DEC write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_set(header, "NAME", "%s", name) < 0) {
     std::cerr << "TrigDADA::WriteHeader NAME write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_set(header, "SIGPROC_FILE", "%s", sigproc_file) < 0) {
     std::cerr << "TrigDADA::WriteHeader SIGPROC_FILE write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_set(header, "UTC_START", "%s", utc_start_str) < 0) {
     std::cerr << "TrigDADA::WriteHeader UTC_START write fail" << std::endl;
     dada_error = true;
   }
   ipcbuf_mark_filled (hdu->header_block, header_size);
   multilog(log,LOG_INFO,"TrigDADA::WriteHeader key=%x h_writetimes=%" PRIu64 "\n",dada_key,h_writetimes++);
   return true;
 }
 timeslice WriteData(PtrFloat data, timeslice bytes_chunk) {
   timeslice bytes_written = ipcio_write(hdu->data_block, (char*)packout, bytes_chunk);
   if(bytes_written < 0) {
     multilog(log,LOG_INFO,"TrigDADA::WriteData key=%x ipcio_write failed\n",dada_key);
   }
   multilog(log,LOG_INFO,"TrigDADA::WriteData key=%x d_writetimes=%" PRIu64 "\n",dada_key,d_writetimes++);
   return bytes_written;
 }
 void PrintHeader() {
   char str[32];
   // positions
   snprintf (str, sizeof(str), "RA     %2.2f", ra);
   snprintf (str, sizeof(str), "DEC    %2.2f", dec);
   // frequency
   snprintf (str, sizeof(str), "FCH1   %4.1f", fch1);
   snprintf (str, sizeof(str), "FOFF   %4.2f", foff);
   snprintf (str, sizeof(str), "CFREQ  %4.1f", cfreq);
   snprintf (str, sizeof(str), "BANDW  %4.1f", bandwidth);
   // time
   std::cout << "TSAMP  " << tsamp << std::endl;
   std::cout << "TSTART " << tstart << std::endl;
   std::cout << "EPOCH  " << epoch << std::endl;
   // memory
   std::cout << "NCHANS " << nchans<< std::endl;
   std::cout << "NBITS  " << nbits << std::endl;
   std::cout << "NPOL   " << npol << std::endl;
   // strings
   std::cout << "UTC    " << utc_start_str << std::endl;
   std::cout << "SOURCE " << name << std::endl;
   std::cout << "SIGFIL " << sigproc_file << std::endl;
   std::cout << "STATID " << stationid << std::endl;
 }
 struct Header GetHeader() const {
   struct Header ret;
   // positions
   ret.ra       = ra;
   ret.dec      = dec;
   // time
   ret.tsamp    = tsamp;
   ret.tstart   = tstart;
   ret.epoch    = epoch;
   // memory
   ret.nchans   = nchans;
   ret.nbits    = nbits;
   ret.nifs     = nifs;
   ret.npol     = npol;
   // freq
   ret.fch1     = fch1;
   ret.foff     = foff;
   ret.cfreq    = cfreq;
   ret.bandwidth = bandwidth;
   // station
   ret.stationid = stationid;
   // strings
   strcpy(ret.utc_start_str, utc_start_str);
   strcpy(ret.name, name);
   strcpy(ret.sigproc_file, sigproc_file);
   return ret;
 }
 bool SetHeader(const struct Header& in) {
   // positions
   ra       = in.ra;
   dec      = in.dec;
   // time
   tsamp    = in.tsamp;
   tstart   = in.tstart;
   epoch    = in.epoch;
   // memory
   nchans   = in.nchans;
   nbits    = in.nbits;
   nifs     = in.nifs;
   npol     = in.npol;
   // freq
   fch1     = in.fch1;
   foff     = in.foff;
   cfreq    = in.cfreq;
   bandwidth = in.bandwidth;
   // station
   stationid = in.stationid;
   // strings
   strcpy(utc_start_str, in.utc_start_str);
   strcpy(name, in.name);
   strcpy(sigproc_file, in.sigproc_file);
   return true;
 }
};
// static variable initialization
uint64_t TrigDADA::d_readtimes  = 0;
uint64_t TrigDADA::h_readtimes  = 0;
uint64_t TrigDADA::d_writetimes = 0;
uint64_t TrigDADA::h_writetimes = 0;

