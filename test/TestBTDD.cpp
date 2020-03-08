#include "asgard.hpp"
#include "FilterbankJSON.hpp"
#define TIMING
#include "BTDD.hpp"
#ifdef TIMING
#include "Timer.hpp"
#endif

#define WRITE 1

#define PFBSON "/home/shining/work/vlite/searches/b1933+16/fbsons/20191216_222018_muos_ea99_dm158.65_sn8.95.fbson"
#define FFBSON "/home/shining/work/vlite/searches/soi/20200303_175836_muos_ea99_dm882.06_sn08.65_wd93.75.fbson"
#define DFBSON "/home/shining/work/vlite/searches/soi/20200129_060053_muos_ea00_dm56.75_sn58.81_wd4.69.fbson"
#define HFBSON "/home/shining/work/vlite/searches/snhighlow/20200112_010829_muos_ea99_dm56.75_sn8.50.fbson"
int main () {
	std::cout << std::endl;
	std::string tpath(DFBSON);
#ifdef TIMING
	Timer read("ReadFBDump");
	read.Start ();
#endif
	FBDump x (tpath);
#ifdef TIMING
	read.StopPrint (std::cout);
#endif
	std::cout << x << std::endl;
	unsigned nsamps = x.nsamps / x.nchans / x.nbits * 8;
	std::cout << "nsamps=" << nsamps << std::endl;
	std::vector<float> bt;
	std::vector<unsigned char> dd;
	/////
	BTDD<unsigned char> btdd (x.tsamp, x.nchans, x.fch1, x.foff);
	btdd.SetDM (x.dm);
	btdd.Execute (x.fb, nsamps, bt, dd);
	/////
	if (WRITE) {
		std::ofstream ofs("bt.dat");
		std::copy (bt.begin(), bt.end(), std::ostream_iterator<float>(ofs,"\n"));
	}
	if (WRITE) {
		std::ofstream ofs("dd.dat");
		std::copy (dd.begin(), dd.end(), std::ostream_iterator<float>(ofs,"\n"));
	}
	return 0;
}
