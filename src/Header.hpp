#pragma once
#include <asgard.hpp>

// One Header to rule them all

struct Header {
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
};

typedef struct Header Header_t;

typedef struct {
  double i0;
  double i1;
  float sn;
  float dm;
  float width;
  float peak_time;
  char meta[128];
} trigger_t;

typedef struct {
  double i0;
  double i1;
  float sn;
  float dm;
  float width;
  float peak_time;
  float dur;
  // station
  int stationid;
  // positions
  double ra, dec;
  // frequency
  double fch1, foff;
  // time
  double tsamp, tstart, epoch;
  // dm
  double dm1, dmoff;
  // memorys
  unsigned nbits, nchans, nsamps, ndm;
	// strs
  char name[16];
  char sigproc_file[64];
} trigHead_t;

// packed trigger header struct
// for trigDADA ease
typedef struct {
  // trigger stuff
  double i0;
  double i1;
  float sn;
  float dm;
  float width;
  float peak_time;
  // header stuff
  // station
  int stationid;
  // positions
  double ra, dec;
  // frequency
  double fch1, foff;
  // time
  double tsamp, tstart, epoch;
  // memory
  unsigned nbits, nchans;
} packth_t;

// cout
std::ostream& operator<< (std::ostream& os, const Header_t& h) {
  // positions
  std::cout << "RA     " << h.ra  << std::endl;
  std::cout << "DEC    " << h.dec << std::endl;
  // frequency
  std::cout << "FCH1   " << h.fch1 << std::endl;
  std::cout << "FOFF   " << h.foff << std::endl;
  std::cout << "CFREQ  " << h.cfreq << std::endl;
  std::cout << "BANDW  " << h.bandwidth<< std::endl;
  // time
  std::cout << "TSAMP  " << h.tsamp << std::endl;
  std::cout << "TSTART " << h.tstart << std::endl;
  std::cout << "EPOCH  " << h.epoch << std::endl;
  // memory
  std::cout << "NCHANS " << h.nchans<< std::endl;
  std::cout << "NBITS  " << h.nbits << std::endl;
  std::cout << "NPOL   " << h.npol << std::endl;
  // strings
  std::cout << "UTC    " << h.utc_start_str << std::endl;
  std::cout << "SOURCE " << h.name << std::endl;
  std::cout << "SIGFIL " << h.sigproc_file << std::endl;
  std::cout << "STATID " << h.stationid << std::endl;
  return os;
}
std::ostream& operator<< (std::ostream& os, const trigger_t& t) {
  std::cout << "I0     " << t.i0 << std::endl;
  std::cout << "I1     " << t.i1 << std::endl;
  std::cout << "SN     " << t.sn << std::endl;
  std::cout << "DM     " << t.dm << std::endl;
  std::cout << "WD     " << t.width*1e3 << std::endl;
  std::cout << "PT     " << t.peak_time << std::endl;
  return os;
}
