#include "asgard.hpp"
#include "PsrDADA.hpp"

using std::cout;
using std::endl;

int main() {
  PtrFloat f = nullptr;
  PtrByte  b = nullptr;
  key_t kk = 0x56;
  PsrDADA my(kk, 10240, 4096, 2);  
  // initialize memory
  f = new float[my.GetByteChunkSize() / my.GetStride()];
  b = new unsigned char[my.GetByteChunkSize()];
  // Header
  struct DADAHeader dHead;
  dHead.ra = 0.0f;
  dHead.dec = 0.0f;
  dHead.tsamp = 123;
  dHead.nbits = 420;
  strcpy(dHead.utc_start_str, "060606060660606606");
  strcpy(dHead.source_str,  "J0420-0420");
  my.WriteLock(true);
  // write Header
  my.SetHeader(dHead);
  my.WriteHeader();
  // write Data
  my.WriteData(f, b, 1);
  // unlock write
  my.WriteLock(false);
  return 0;
}
