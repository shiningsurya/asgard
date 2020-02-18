#pragma once
#include "asgard.hpp"
using std::cout;
using std::endl;

#include "TriggerDADA.hpp"
#include "FilterbankJSON.hpp"


class TriggerSink {
private:
  key_t dkey;
  TrigDADA tbuff;
	multilog_t * log;
  // trigger 
  Header_t      header;
  trigger_t     triggg;
  PtrByte       pdata;
  PtrByte       bt;
  PtrFloat      tsn;
  PtrFloat      dsn;
  void Print (const struct Header& h, const trigger_t& t) {
    char str[32];
    // epoch
    // source name
    // utc
    // sn,dm,width
  }
  // fbson dumper
  FilterbankJSON fbson;
  void dumpfbson () {
    fbson.DumpHead (header, triggg);
    fbson.DumpData (pdata, 0, bytesindump);
    try {
      fbson.WritePayload (bytesindump);
    }
    catch (...) {
      multilog (log, LOG_ERR, "TriggerSink::DumpFBSON caught exception\n");
      Print (header, triggg);
    }
  }
  // mler
  void mler () {
    // TODO here is where I 
    // put ML logic
  }
public:
  TriggerSink ( key_t key_ ) : dkey(key_), tbuff (dkey), log(nullptr) {
    log = multilog_open ("triggersink",0);
    multilog_add (log, stdout);
  }
  ~TriggerSink () {
    tbuff.~TrigDADA();
    multilog(log, LOG_INFO,  "TriggerSink::dtor key=%x\n", dada_key);
    // log close
    if(log != nullptr) multilog_close(log);
  }
  // MAIN method
  void FollowTrig () {
    tbuff.ReadLock (true);
    tbuff.ReadHeader ();
    header = tbuff.GetHeader ();
    triggg = tbuff.GetTrigger();
    bytesindump = triggg.nsamps * header.nchans * header.nbits / 8;
    timeslilce bytes_read = tbuff.ReadData (pdata, bytesindump);
    if (bytes_read != bytesindump) {
      multilog (log, LOG_ERR, "TriggerSink::FollowTrig incomplete data.\n");
      bytesindump = bytes_read;
    }
    // --- actions
    // TODO should I have some logic?
    // TODO make them multi-threaded?
    // dump fbson
    // make candplot
    dumpfbson ();
    plotter ();
    mler ();
    tbuff.ReadLock (false);
  }


};

