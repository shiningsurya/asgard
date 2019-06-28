#include <asgard.hpp>
#include <Header.hpp>
#include <FilterbankSink.hpp>

using std::cout;
using std::endl;

constexpr timeslice N = 1024;
int main() {
 // prepare data
 unsigned char arr[N];
 for(timeslice i = 0; i < N;i++) arr[i] = i % 256;
 // prepare header
 struct Header ret;
 // positions
 ret.ra       = 1.23;
 ret.dec      = 0.456;
 // time
 ret.tsamp    = 786e-6f;
 ret.tstart   = 58888.888f;
 // memory
 ret.nchans   = 1024;
 ret.nbits    = 8;
 ret.nifs     = 1;
 ret.npol     = 1;
 // freq
 ret.fch1     = 1033.33f;
 ret.foff     = -42.00f;
 ret.cfreq    = 420.00f;
 ret.bandwidth = 42.00f;
 // station
 ret.stationid = 1;
 // strings
 strcpy(ret.utc_start_str, "2019-04-20-16:20:00");
 strcpy(ret.name, "3C147");
 strcpy(ret.sigproc_file, "/tmp/testtesttesttest.fil");
 	cout << "Creating FBSINK object" << endl;
 // Get to main stuff
 FilterbankSink fbsink(ret);
 PtrByte data = arr;
 	cout << "Writing DATA" << endl;
 fbsink.Data(data, N);
 return 0;
}


