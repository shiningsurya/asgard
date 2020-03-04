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
	// DADA
	key_t       hkey;
	key_t       dkey;
	dada_hdu_t *dhdu;
	dada_hdu_t *hhdu;
	// error state
	bool dada_error;
	// states
	bool read_lock;
	bool write_lock;
	static uint64_t h_readtimes, h_writetimes;
	static uint64_t d_readtimes, d_writetimes;
	// logging
	multilog_t * log;
 bool Connect() {
   dada_error = dada_hdu_connect (hhdu) < 0 || dada_hdu_connect (dhdu) < 0;
   return dada_error;
 }
 // assume hhdu and dhdu are ALWAYS in the same state
 bool Disconnect() {
   if(hhdu == nullptr && dhdu == nullptr) return true;
   dada_error = dada_hdu_disconnect (hhdu)  < 1 || dada_hdu_disconnect (dhdu) < 1;
   return dada_error;
 }
 public:
  TrigDADA () {
    // chill
    log = nullptr;
    hhdu = nullptr;
    dhdu = nullptr;
    read_lock = false;
    write_lock = false;
  }
  TrigDADA (key_t hkey_, key_t dkey_) : 
    hkey (hkey_),
    dkey (dkey_) {
    // logging
    log = multilog_open ("triggerdada",0);
    multilog_add (log, stdout);
    // construction
    multilog(log, LOG_INFO,  "TrigDADA::ctor hkey=%x dkey=%x\n", hkey, dkey);
    dada_error = false;
    // header dada
    hhdu = dada_hdu_create(log);
    dada_hdu_set_key(hhdu, hkey);
    // data dada
    dhdu = dada_hdu_create(log);
    dada_hdu_set_key(dhdu, dkey);
    // Connection
    if(Connect()) exit(1);
    // state initialize
    read_lock = false;
    write_lock = false;
  }
 ~TrigDADA() { 
   multilog(log, LOG_INFO,  "TrigDADA::dtor hkey=%x dkey=%x\n", hkey, dkey);
   // Disconnection
   Disconnect();
   // log close
   if(log != nullptr) multilog_close(log);
 }
 TrigDADA& operator=(TrigDADA&& other) {
   multilog(other.log, LOG_INFO,  "TrigDADA::move_assignment hkey=%x dkey=%x\n", other.hkey, other.dkey);
   // logging
   std::swap (log, other.log);
   // DADA
   std::swap (hhdu, other.hhdu);
   std::swap (dhdu, other.dhdu);
   std::swap (hkey, other.hkey);
   std::swap (dkey, other.dkey);
   // state initialize
   std::swap (read_lock, other.read_lock);
   std::swap (write_lock, other.write_lock);
   // error state too
   std::swap (dada_error, other.dada_error);
   // destroy other
   other.~TrigDADA();
   return *this;
 }
 // lock/unlock functions work on both at the same time
 bool ReadLock(bool x) {
   bool ret;
   multilog(log,LOG_INFO,"TrigDADA::ReadLock hkey=%x dkey=%x Before\n",hkey, dkey);
   if(x) {
     // Requested lock
     if(read_lock) ret =  true;
     //else ret = (dada_hdu_lock_read (hhdu) && dada_hdu_lock_read (dhdu));
     dada_hdu_lock_read (hhdu); dada_hdu_lock_read (dhdu);
     read_lock = true;
   }
   else {
     // Requested unlock
     if(read_lock) { dada_hdu_unlock_read(hhdu); dada_hdu_unlock_read (dhdu); }
     else ret = true;
     read_lock = false;
   }
   multilog(log,LOG_INFO,"TrigDADA::ReadLock hkey=%x dkey=%x After\n",hkey, dkey);
   return ret;
 }
 bool WriteLock(bool x) {
   bool ret;
   multilog(log,LOG_INFO,"TrigDADA::WriteLock hkey=%x dkey=%x Before\n",hkey, dkey);
   if(x) {
     // Requested lock
     if(write_lock) ret = true;
     //else ret = (dada_hdu_lock_write(hhdu) && dada_hdu_lock_write (dhdu));
     dada_hdu_lock_write(hhdu); dada_hdu_lock_write (dhdu);
     write_lock = true;
   }
   else {
     // Requested unlock
     if(write_lock) {
       dada_hdu_unlock_write (hhdu);dada_hdu_unlock_write (dhdu);
     }
     else ret = true;
     write_lock = false;
   }
   multilog(log,LOG_INFO,"TrigDADA::WriteLock hkey=%x dkey=%x After\n",hkey, dkey);
   return ret;
 }
 // --------------------
 // those functions
 // max possible array is initialized on the heap
 // array is reused
 bool ReadHeader (Header_t& h, trigger_t& t) {
   auto hp = &h;
   auto tp = &t;
   if(ipcbuf_eod((ipcbuf_t*)hhdu->data_block)) {
     return false;
   }
   // This has to BLOCK
   timeslice readh = ipcio_read (hhdu->data_block, reinterpret_cast<char*>(hp), sizeof(Header_t));
   if(readh == -1) {
     multilog(log,LOG_INFO,"TrigDADA::ReadHeader key=%x Header read failed\n",hkey);
     dada_error = true;
     return false;
   }
   timeslice readt = ipcio_read (hhdu->data_block, reinterpret_cast<char*>(tp), sizeof(trigger_t));
   if(readt == -1) {
     multilog(log,LOG_INFO,"TrigDADA::ReadHeader key=%x Trigger read failed\n",hkey);
     dada_error = true;
     return false;
   }
   // read trim trigHead_t --> h and t
   multilog(log,LOG_INFO,"TrigDADA::ReadHeader key=%x h_readtimes=%" PRIu64 "\n",hkey,h_readtimes++);
   return true;
 }
 timeslice ReadData(PtrByte packin, timeslice bytes_chunk) {
   if(ipcbuf_eod((ipcbuf_t*)dhdu->data_block)) {
     return -1;
   }
   timeslice chunk_read = ipcio_read(dhdu->data_block, reinterpret_cast<char*>(packin), bytes_chunk);
   if(chunk_read == -1) {
     multilog(log,LOG_INFO,"TrigDADA::ReadData key=%x ipcio_read failed\n", dkey);
   }
   if(chunk_read != bytes_chunk)
    multilog (log, LOG_ERR, "TrigDADA::ReadData read %lu bytes while expected %lu bytes.\n", chunk_read, bytes_chunk); 
   multilog(log,LOG_INFO,"TrigDADA::ReadData key=%x d_readtimes=%" PRIu64 "\n",dkey,d_readtimes++);
   return chunk_read;
 }
 bool WriteHeader (Header_t& h, trigger_t& t) {
   auto hp = &h;
   auto tp = &t;
   // writes
   timeslice writeh = ipcio_write (hhdu->data_block, reinterpret_cast<char*>(hp), sizeof(Header_t));
   if(writeh < 0) {
     multilog(log,LOG_INFO,"TrigDADA::WriteHeader key=%x Header write failed\n",hkey);
     dada_error = true;
     return false;
   }
   timeslice writet = ipcio_write (hhdu->data_block, reinterpret_cast<char*>(tp), sizeof(trigger_t));
   if(writet < 0) {
     multilog(log,LOG_INFO,"TrigDADA::WriteHeader key=%x Trigger write failed\n",hkey);
     dada_error = true;
     return false;
   }
   multilog(log,LOG_INFO,"TrigDADA::WriteHeader key=%x h_writetimes=%" PRIu64 "\n",hkey,h_writetimes++);
   return true;
 }
 timeslice WriteData(PtrByte data, timeslice bytes_chunk) {
   timeslice bytes_written = ipcio_write(dhdu->data_block, reinterpret_cast<char*>(data), bytes_chunk);
   if(bytes_written < 0) {
     multilog(log,LOG_INFO,"TrigDADA::WriteData key=%x ipcio_write failed\n",dkey);
   }
   multilog(log,LOG_INFO,"TrigDADA::WriteData key=%x d_writetimes=%" PRIu64 "\n",dkey,d_writetimes++);
   return bytes_written;
 }
 timeslice WriteData(PtrByte data, timeslice start, timeslice bytes_chunk) {
   timeslice bytes_written = ipcio_write(dhdu->data_block, reinterpret_cast<char*>(data+start), bytes_chunk);
   if(bytes_written < 0) {
     multilog(log,LOG_INFO,"TrigDADA::WriteData key=%x ipcio_write failed\n",dkey);
   }
   multilog(log,LOG_INFO,"TrigDADA::WriteData key=%x d_writetimes=%" PRIu64 "\n",dkey,d_writetimes++);
   return bytes_written;
 }
};
// static variable initialization
uint64_t TrigDADA::d_readtimes  = 0;
uint64_t TrigDADA::h_readtimes  = 0;
uint64_t TrigDADA::d_writetimes = 0;
uint64_t TrigDADA::h_writetimes = 0;

