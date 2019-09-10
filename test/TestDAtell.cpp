#include "asgard.hpp"
#include "PsrDADA.hpp"

using std::cout;
using std::endl;

int main() {
  key_t kk = 0x60;
  PtrFloat f = nullptr;
  PtrByte  b = nullptr;
  // initialize memory
  f = new float[10240*4096];
  b = new unsigned char[10240*4096*2/8];
  // header
  struct Header dHead;
  dHead.ra = 2.5465;
  dHead.dec = -0.145303;
  dHead.tsamp = 781.25;
  dHead.nbits = 2;
  dHead.fch1 = 361.941;
  dHead.foff = -0.0102;
  dHead.cfreq = 340.978;
  dHead.bandwidth = -41.9363;
  dHead.nchans = 4096;
  dHead.tstart = 58685;
  dHead.npol = 1;
  strcpy(dHead.sigproc_file,  "/mnt/ssd/fildata/20190720_555555_muos_ea99_kur.fil");
  strcpy(dHead.utc_start_str, "2019-07-11-15:43:20");
  strcpy(dHead.name,  "J0420-0420");
  // it's play time
  PsrDADA dadaout(kk, 10240, 4096, 2, "/tmp/dadaout.log");  
  dadaout.WriteLock(true);
  while(true) {
    PsrDADA dadain(kk, 10240, 4096, 2, "/tmp/dadain.log");  

  }
  return 0;
}
