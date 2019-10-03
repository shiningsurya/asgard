#include "asgard.hpp"
#include "PsrDADA.hpp"
#include <thread>
#include <chrono>

using std::cout;
using std::endl;

int main() {
  PtrFloat f = nullptr;
  PtrByte  b = nullptr;
  key_t kk = 0x20;
  PsrDADA my(kk, 10240, 4096, 2, "/home/vlite-master/surya/logs/dadaout.log");  
  std::cout << " sizes=" << my.GetByteChunkSize() << std::endl;
  std::cout << " stride=" << my.GetStride() << std::endl;
  // initialize memory
  f = new float[10240*4096];
  b = new unsigned char[10240*4096*2/8];
  // Header
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
  // write Header
  my.SetHeader(dHead);
  int obs = 0;
  while(++obs <= 3) {
    my.WriteLock(true);
    my.WriteHeader();
    int xx = 0;
    while(++xx <= 4) {
      // write Data
      my.WriteData(f, b, 1);
      // sleep
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    my.WriteLock(false);
  }
  return 0;
}
