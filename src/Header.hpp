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
