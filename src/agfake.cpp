#include "asgard.hpp"
#include "FilterbankFake.hpp"
#include "PsrDADA.hpp"
#include <thread>
#include <chrono>

// signal handling
#include <csignal>
void d_signal_handle (int sig) {
	std::cout << "received signal=" << sig << std::endl;
	// cause a core dump
	// core.pattern?
	exit (1);
}

void update_hepc (Header_t& h, time_t hep) {
	struct tm t;
	gmtime_r (&hep, &t);
  strftime (h.utc_start_str, sizeof(h.utc_start_str), "%Y-%m-%d-%H:%M:%S", &t);
}

// Boost
#include<boost/program_options/cmdline.hpp>
#include<boost/program_options/config.hpp>
#include<boost/program_options/environment_iterator.hpp>
#include<boost/program_options/option.hpp>
#include<boost/program_options/options_description.hpp>
#include<boost/program_options/parsers.hpp>
#include<boost/program_options/positional_options.hpp>
#include<boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

inline key_t dx(std::string in) {
	std::stringstream ss;
	key_t ret;
	ss << in;
	ss >> std::hex >> ret;
	return ret;
}

int main(int ac, char * av[]) {
	// signal bindings
	std::signal( SIGABRT, d_signal_handle );
	std::signal( SIGFPE,  d_signal_handle );
	std::signal( SIGILL,  d_signal_handle );
	std::signal( SIGINT,  d_signal_handle );
	std::signal( SIGTERM, d_signal_handle );
	std::signal( SIGSEGV, d_signal_handle );
	// actual start
	key_t dadakey;
	std::string s_dadakey, d_dadakey("20"), odir, d_odir("/mnt/ssd/dumps/");
	std::string pfile;
	float rms;
	float warm_time;
	int   nwarm;
	// po
	po::variables_map vm;
	po::options_description opt("Options");
	// adding options
	opt.add_options()
("help,h", "Prints help")
("file,f",  po::value<std::string>(&pfile), "S/N DM Width file")
("rms",  po::value<float>(&rms)->default_value(4.0f), "RMS value")
("wtime,w",  po::value<float>(&warm_time)->default_value(8.0f), "Warm up time.")
("nwtime,n",  po::value<int>(&nwarm)->default_value(0), "# of warm ups.")
("key,k",  po::value<std::string>(&s_dadakey)->default_value(d_dadakey), "Output DADA key[def=0x20]");
	// parsing
	try {
		po::store(po::command_line_parser(ac,av).options(opt).run(), vm);
		po::notify(vm);
		if(vm.count("help")) {
			std::cout << "Asgard::agtriggerhook Realtime processing of triggers." << std::endl;
			std::cout << std::endl;
			std::cout << opt << std::endl;
			std::cout << "Part of Asgard" << std::endl;
			return 0;
		}
	}
	catch(std::exception& e) {
		std::cerr << "Error in asgard main: " << e.what() << std::endl;
		return 1;
	}
	// key 0d to 0x
	dadakey = dx(s_dadakey);
	// header
	Header_t hfake;
	hfake.stationid    = 0;
	hfake.ra           = 0.0;
	hfake.dec          = 0.0;
	hfake.fch1         = 361.941;
	hfake.foff         = -0.0102384;
	hfake.cfreq        = 340.978403 ;
	hfake.bandwidth    = -41.936330;
	hfake.tsamp        = 781.25;
	hfake.tstart       = 54321.12;
	hfake.epoch        = time(NULL);
	hfake.nbits        = 8;
	hfake.nchans       = 4096;
	hfake.nifs         = 1;
	hfake.npol         = 1;
	strcpy (hfake.name, "FAKEWHITE");
	strcpy (hfake.sigproc_file, "/tmp/fakefakefakefake.fil");
	update_hepc (hfake, static_cast<time_t>(hfake.epoch));
	//strcpy (hfake.utc_start_str, "1900-01-01-00:00:00");
	// 
	FilterbankFake  ffake (hfake.tsamp/1E6, hfake.nchans, hfake.fch1, hfake.foff, rms);
	PsrDADA dada (dadakey, 0,0,0, "/home/vlite-master/surya/faketriggers/dadadada.log");
	// warm up loop
	for (int i = 0; i < 1; i++) {
    dada.WriteLock (true);
    dada.SetHeader (hfake);
    dada.WriteHeader ();
    auto fb = ffake.WhiteNoise (warm_time);
		dada.WriteData (fb.data(), fb.size());
		hfake.epoch += warm_time;
    update_hepc (hfake, static_cast<time_t>(hfake.epoch));
    dada.WriteLock (false);
	}
	// Create Write loop
	float sn, dm, wd;
	std::ifstream pf (pfile);
	dada.WriteLock (true);
	strcpy (hfake.name, "FAKESIGNAL");
  dada.SetHeader (hfake);
  dada.WriteHeader ();
  float atime = 0.0f;
  float ctime = 0.0f;
	while (pf >> sn >> dm >> wd) {
		// only 0.1s guards
		std::cout << "SN=" << sn << " DM=" << dm << " wd=" << wd*1e3 << std::endl;
		{
      auto fb = ffake.Signal (sn, dm, wd, 1.0, 1.0);
      dada.WriteData (fb.data(), fb.size());
      float tt = (fb.size()*hfake.tsamp/1E6/4096);
      std::cout << "Writing " << tt << "s bsize=" << fb.size() << std::endl;
      hfake.epoch += tt;
      atime  += tt;
      ctime  += tt;
		}
		if (ctime >= 24) {
      std::cout << ".. sleeping for .. " << 14 << "s" << std::endl;
      std::this_thread::sleep_for (std::chrono::seconds(14));
      ctime = 0.0f;
		}
    std::cout << "Written time=" << ctime << std::endl;
	}
	while (ctime < 24) {
		std::cout << "SN=" << sn << " DM=" << dm << " wd=" << wd*1e3 << std::endl;
		{
      auto fb = ffake.Signal (sn, dm, wd, 1.0, 1.0);
      dada.WriteData (fb.data(), fb.size());
      float tt = (fb.size()*hfake.tsamp/1E6/4096);
      std::cout << "Writing " << tt << "s bsize=" << fb.size() << std::endl;
      hfake.epoch += tt;
      atime  += tt;
      ctime  += tt;
		}
		if (ctime >= 24) {
      std::cout << ".. sleeping for .. " << 14 << "s" << std::endl;
      std::this_thread::sleep_for (std::chrono::seconds(14));
    }
	}
  dada.WriteLock (false);
  /// end of fakesignals
  update_hepc (hfake, static_cast<time_t>(hfake.epoch));
	strcpy (hfake.name, "FAKEWHITE");
	// for triggerhook to respond
	for (int i = 0; i < nwarm; i++) {
    dada.WriteLock (true);
    dada.SetHeader (hfake);
    dada.WriteHeader ();
    auto fb = ffake.WhiteNoise (warm_time);
		dada.WriteData (fb.data(), fb.size());
		hfake.epoch += warm_time;
    update_hepc (hfake, static_cast<time_t>(hfake.epoch));
    dada.WriteLock (false);
	}
	//
	return 0;
}
