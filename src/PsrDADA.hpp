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

//#define AG_RUNNING

constexpr char LOGDIR[] = "/home/vlite-master/surya/logs";

class PsrDADA {
 private:   
	// logging
	FILE * log_fp;
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
	long lalpha;
 void pack(PtrFloat ret, int numants, timeslice nsamps, PtrByte dd) {
   timeslice ii = 0;
   float rmean = 0.0f, rstd = 0.0f;
   float bmax = pow(2,nbits) -1; 
   float snant = sqrt(numants);
   //rmean = 0.5 * bmax * numants;
   //rstd  = sqrt(numants);
   rmean = 0.5 * bmax;
   rstd  = 33.318;
   //if (nbits == 8)      rstd *= 33.818;
   //else if (nbits == 4) rstd *= 3.137;
#ifdef AG_RUNNING
   // the running mean idea
   // two pass mean, std estimate
   rmean = 0.0f;
   rstd = 0.0f;
   std::for_each(ret, ret + nsamps, [&rmean](const float& xx) { rmean += xx; });
   rmean /= nsamps;
   std::for_each(ret, ret + nsamps, [&rstd, &rmean](const float& xx) { rstd += pow(xx - rmean, 2); });
   rstd = sqrt(rstd / (nsamps - 1)); // Bessel correction
   std::cout << " rmean=" << rmean << " rstd=" << rstd << std::endl;
   multilog (log, LOG_INFO, "PsrDADA::Running redigitization mean=%f std.dev.=%f\n", rmean, rstd);
#endif
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
      // the division by numants is required here for
      // the mean computation
       alpha = snant * ((ret[it++]/numants) - rmean) / rstd;
       //dd[ii++] = dig8bit(alpha);
       // ^ produces weird ridges in the distribution
       // going for linear-like coding
       lalpha = std::lround((32*alpha) + 128);
       if (lalpha < 0)    lalpha = 0;
       if (lalpha >= 255) lalpha = 255;
       dd[ii++] = (unsigned char) lalpha;
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
 PsrDADA() {
   // chill
   log_fp = NULL;
   log = NULL;
 }
 PsrDADA(key_t dada_key_, timeslice nsamps_, int nchans_, int nbits_, const char * logfile) : 
   dada_key(dada_key_), 
   nsamps(nsamps_),
   nchans(nchans_), 
   nbits(nbits_)
  {
    // logging
    log = multilog_open ("dadadada",0);
    multilog_add (log, stdout);
    log_fp = fopen(logfile, "w+");
    multilog_add(log, log_fp);
    // construction
    multilog(log, LOG_INFO,  "PsrDADA::ctor key=%x\n", dada_key);
    dada_error = false;
    sample_chunk = nsamps * nchans;
    bytes_chunk = nsamps * nchans * nbits / 8;
    bytes_stride = (nchans * nbits) / 8;
    // DADA
    hdu = dada_hdu_create(log);
    dada_hdu_set_key(hdu, dada_key);
    // Connection
    if(Connect()) exit(1);
    // state initialize
    read_lock = false;
    write_lock = false;
  }
 PsrDADA& operator=(PsrDADA&& other) {
   multilog(other.log, LOG_INFO,  "PsrDADA::move_assignment key=%x\n", other.dada_key);
   // ctor args
   nsamps = other.nsamps;
   nchans = other.nchans;
   nbits  = other.nbits;
   // sharing
   sample_chunk = other.sample_chunk;
   bytes_chunk  = other.bytes_chunk;
   bytes_stride = other.bytes_stride;
   // logging
   if(other.log_fp != NULL) log_fp = other.log_fp;
   other.log_fp = NULL;
   log = other.log;
   other.log = NULL;
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
   // destroy other
   other.~PsrDADA();
   return *this;
 }
 ~PsrDADA() { 
   multilog(log, LOG_INFO,  "PsrDADA::dtor key=%x\n", dada_key);
   // Disconnection
   Disconnect();
   // log close
   if(log != NULL) multilog_close(log);
   // file close
   if(log_fp != NULL) fclose(log_fp);
 }
 bool ReadLock(bool x) {
   bool ret;
   multilog(log,LOG_INFO,"PsrDADA::ReadLock key=%x Before\n",dada_key);
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
   multilog(log,LOG_INFO,"PsrDADA::ReadLock key=%x After\n",dada_key);
   return ret;
 }
 bool WriteLock(bool x) {
   bool ret;
   multilog(log,LOG_INFO,"PsrDADA::WriteLock key=%x Before\n",dada_key);
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
   multilog(log,LOG_INFO,"PsrDADA::WriteLock key=%x After\n",dada_key);
   return ret;
 }
 const timeslice GetByteChunkSize() const {
   return bytes_chunk;
 }
 const timeslice GetStride() const {
   return bytes_stride;
 }
 bool ReadHeader() {
   // blocking read
   header = ipcbuf_get_next_read(hdu->header_block, &header_size);
   if(!header) {
     multilog(log,LOG_INFO,"PsrDADA::ReadHeader key=%x GetNextRead failed\n",dada_key);
     dada_error = true;
     return false;
   }
   // get params
   if(ascii_header_get(header, "STATIONID", "%d", &stationid) < 0) {
     std::cerr << "PsrDADA::ReadHeader STATIONID write fail" << std::endl;
     dada_error = true;
   }
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
   if(ascii_header_get(header, "SCANSTART", "%lf", &tstart) < 0) {
     std::cerr << "PsrDADA::ReadHeader SCANSTART read fail" << std::endl;
     dada_error = true;
   }
   // unix epoch
   if(ascii_header_get(header, "UNIXEPOCH", "%lf", &epoch) < 0) {
     std::cerr << "PsrDADA::ReadHeader UNIXEPOCH read fail" << std::endl;
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
   if(ascii_header_get(header, "NAME", "%s", name) < 0) {
     std::cerr << "PsrDADA::ReadHeader NAME read fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_get(header, "UTC_START", "%s", utc_start_str) < 0) {
     std::cerr << "PsrDADA::ReadHeader UTC_START read fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_get(header, "SIGPROC_FILE", "%s", sigproc_file) < 0) {
     std::cerr << "PsrDADA::ReadHeader SIGPROC_FILE read fail" << std::endl;
     dada_error = true;
   }
   // mark buffer clear
   ipcbuf_mark_cleared(hdu->header_block);	
   multilog(log,LOG_INFO,"PsrDADA::ReadHeader key=%x h_readtimes=%" PRIu64 "\n",dada_key,h_readtimes++);
   return true;
 }
 timeslice ReadData(PtrFloat data, PtrByte packin) {
   if(ipcbuf_eod((ipcbuf_t*)hdu->data_block)) {
     return -1;
   }
   // Initialization
   if(packin == nullptr) packin = ( PtrByte ) new unsigned char[bytes_chunk];
   // READ -- this blocks
   timeslice chunk_read = ipcio_read(hdu->data_block, (char*)packin, bytes_chunk);
   if(chunk_read == -1) {
     multilog(log,LOG_INFO,"PsrDADA::ReadData key=%x ipcio_read failed\n", dada_key);
     return -1;
   }
   timeslice nsamps_read = chunk_read / bytes_stride;
   // Initialization
   if(data == nullptr) data = new float[nsamps_read * nchans];
   if(chunk_read != bytes_chunk)
     std::cerr << "PsrDADA::ReadData read " 
       << std::dec << chunk_read 
       << " bytes while expected "
       << bytes_chunk
       << " bytes."
       << std::endl;
   if(nsamps_read != nsamps)
     std::cerr << "PsrDADA::ReadData read " 
       << std::dec << nsamps_read 
       << " samples while expected "
       << nsamps
       << " samples."
       << std::endl;
   // unpack to floats
   unpack(packin, chunk_read, data);
   multilog(log,LOG_INFO,"PsrDADA::ReadData key=%x d_readtimes=%" PRIu64 "\n",dada_key,d_readtimes++);
   return chunk_read;
 }
 bool WriteHeader() {
   header = ipcbuf_get_next_write(hdu->header_block);
   if(!header) {
     multilog(log,LOG_INFO,"PsrDADA::WriteHeader key=%x GetNextWrite failed\n",dada_key);
     dada_error = true;
     return false;
   }
   // set params
   if(ascii_header_set(header, "STATIONID", "%d", stationid) < 0) {
     std::cerr << "PsrDADA::WriteHeader STATIONID write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_set(header, "NCHAN", "%d", nchans) < 0) {
     std::cerr << "PsrDADA::WriteHeader NCHAN write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_set(header, "BANDWIDTH", "%lf", bandwidth) < 0) {
     if(ascii_header_set(header, "BW", "%f", bandwidth) < 0) {
       std::cerr << "PsrDADA::WriteHeader BANDWIDTH write fail" << std::endl;
       dada_error = true;
     }
   }
   if(ascii_header_set(header, "CFREQ", "%lf", cfreq) < 0) {
     if(ascii_header_set(header, "FREQ", "%f", cfreq) < 0) {
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
   if(ascii_header_set(header, "SCANSTART", "%lf", tstart) < 0) {
     std::cerr << "PsrDADA::WriteHeader SCANSTART write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_set(header, "UNIXEPOCH", "%lf", epoch) < 0) {
     std::cerr << "PsrDADA::WriteHeader UNIXEPOCH write fail" << std::endl;
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
   if(ascii_header_set(header, "NAME", "%s", name) < 0) {
     std::cerr << "PsrDADA::WriteHeader NAME write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_set(header, "SIGPROC_FILE", "%s", sigproc_file) < 0) {
     std::cerr << "PsrDADA::WriteHeader SIGPROC_FILE write fail" << std::endl;
     dada_error = true;
   }
   if(ascii_header_set(header, "UTC_START", "%s", utc_start_str) < 0) {
     std::cerr << "PsrDADA::WriteHeader UTC_START write fail" << std::endl;
     dada_error = true;
   }
   ipcbuf_mark_filled (hdu->header_block, header_size);
   multilog(log,LOG_INFO,"PsrDADA::WriteHeader key=%x h_writetimes=%" PRIu64 "\n",dada_key,h_writetimes++);
   return true;
 }
 timeslice WriteData(PtrFloat data, PtrByte packout, int numants) {
   if(packout == nullptr) packout = ( PtrByte ) new unsigned char[bytes_chunk];
   // NOT SURE ABOUT EOD while WRITING?
   //if(ipcbuf_eod((ipcbuf_t*)hdu->data_block)) return -1;
   // PACK and WRITE
   pack(data, numants, sample_chunk,  packout);
   timeslice bytes_written = ipcio_write(hdu->data_block, (char*)packout, bytes_chunk);
   if(bytes_written < 0) {
     multilog(log,LOG_INFO,"PsrDADA::WriteData key=%x ipcio_write failed\n",dada_key);
     return false;
   }
   multilog(log,LOG_INFO,"PsrDADA::WriteData key=%x d_writetimes=%" PRIu64 "nant=%d\n",dada_key,d_writetimes++, numants);
   return bytes_written;
 }
 timeslice WriteData(PtrByte packout, timeslice bchunk) {
   // NOT SURE ABOUT EOD while WRITING?
   //if(ipcbuf_eod((ipcbuf_t*)hdu->data_block)) return -1;
   // PACK and WRITE
   timeslice bytes_written = ipcio_write(hdu->data_block, (char*)packout, bchunk);
   if(bytes_written < 0) {
     multilog(log,LOG_INFO,"PsrDADA::WriteData key=%x ipcio_write failed\n",dada_key);
     return false;
   }
   multilog(log,LOG_INFO,"PsrDADA::WriteData key=%x d_writetimes=%" PRIu64 "\n",dada_key,d_writetimes++);
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
 bool Scrub() {
   timeslice chunk_read;
   PtrByte  b = new unsigned char[bytes_chunk];
   multilog(log,LOG_INFO,"PsrDADA::Scrub key=%x called\n",dada_key);
   // second attack on headerbuff
   while(UsedHeadBuf()) {
     header = ipcbuf_get_next_read(hdu->header_block, &header_size);
     ipcbuf_mark_cleared( (ipcbuf_t*) hdu->header_block );
   }
   // first attack on databuff
   while(UsedDataBuf()) {
     do{
       chunk_read = ipcio_read(hdu->data_block, (char*)b, bytes_chunk);
     } while( !ipcbuf_eod( (ipcbuf_t*)hdu->data_block ) );
   }
   multilog(log,LOG_INFO,"PsrDADA::Scrub key=%x buffers cleared\n",dada_key);
   delete[] b;
   return true;
 }
 uint64_t TotalDataBuf() const {
   auto x = ipcbuf_get_nbufs ( (ipcbuf_t*)hdu->data_block );
   return x;
 }
 uint64_t UsedDataBuf() const {
   auto x = ipcbuf_get_nfull( (ipcbuf_t*)hdu->data_block );
   // multilog(log,LOG_INFO,"PsrDADA::UsedData key=%x buffers=%" PRIu64  "\n",dada_key, x);
   return x;
 }
 uint64_t UsedHeadBuf() const {
   auto x = ipcbuf_get_nfull( (ipcbuf_t*)hdu->header_block );
   multilog(log,LOG_INFO,"PsrDADA::UsedHead key=%x buffers=%" PRIu64  "\n",dada_key, x);
   return x;
 }
 uint64_t TellRead() const {
   ipcio_t * ipc = hdu->data_block;
   int64_t current = -1;

   if(ipc -> rdwrt == 'R' || ipc -> rdwrt == 'r') {
     current = ipcbuf_tell_read( (ipcbuf_t*)ipc ); 
   }

   if(current < 0) {
     multilog(log, LOG_ERR, "PsrDADA::TellRead key=%x ipcbuf_tell failed\n", dada_key);
     return -1;
   }

   return current + ipc->bytes;
 }
 uint64_t TellWrite() const {
   ipcio_t * ipc = hdu->data_block;
   int64_t current = -1;

   if(ipc -> rdwrt == 'w' || ipc -> rdwrt == 'W') {
     current = ipcbuf_tell_write( (ipcbuf_t*)ipc ); 
   }

   if(current < 0) {
     multilog(log, LOG_ERR, "PsrDADA::TellWrite key=%x ipcbuf_tell failed\n", dada_key);
     return -1;
   }

   return current + ipc->bytes;
 }
 bool Redigitize(PtrFloat data, PtrByte packout, int out_nbits, int nants) {
  auto old_nbits = nbits;
  nbits = out_nbits;
  pack(data, nants, sample_chunk, packout);
  nbits = old_nbits;
  return true;
 }
 timeslice ZeroReadData() {
   if(ipcbuf_eod((ipcbuf_t*)hdu->data_block)) {
     return -1;
   }
   // READ -- this blocks
   timeslice chunk_read = ipcio_read(hdu->data_block, NULL, bytes_chunk);
   if(chunk_read == -1) {
     multilog(log,LOG_INFO,"PsrDADA::ZeroReadData key=%x ipcio_read failed\n", dada_key);
   }
   multilog(log,LOG_INFO,"PsrDADA::ZeroReadData key=%x d_readtimes=%" PRIu64 "\n",dada_key,d_readtimes++);
   return chunk_read; 
 }
 char* GetCurrDataBuff () {
  return hdu->data_block->curbuf;  
 }
 char* GetBufPtr() {
  auto buf      = hdu->data_block->buf;
  auto ptr_sync = buf.sync;
  auto nbufs    = ptr_sync->nbufs;
  //auto iread    = 0;
  auto iread    = buf.iread;
  auto xfer     = buf.xfer;
  // this is modulo subtract
  auto g = (ptr_sync->r_bufs[iread] + nbufs - 1) % nbufs;
  return buf.buffer[g];
 }
 char* GetBufPtr(unsigned int ibuf) {
  return hdu->data_block->buf.buffer[ibuf];  
 }
 timeslice GetIndex () const {
  // modulo subtract 1
  auto buf = hdu->data_block->buf;
  auto ss = buf.sync;
  auto ii = buf.iread;
  return (ss->r_bufs[ii] + ss->nbufs - 1) % ss->nbufs;
 }
};
// static variable initialization
uint64_t PsrDADA::d_readtimes = 0;
uint64_t PsrDADA::h_readtimes = 0;
uint64_t PsrDADA::d_writetimes = 0;
uint64_t PsrDADA::h_writetimes = 0;
