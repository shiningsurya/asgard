#include "asgard.hpp"
#include "FilterbankFake.hpp"
#include "PsrDADA.hpp"

// signal handling
#include <csignal>
void d_signal_handle (int sig) {
	std::cout << "received signal=" << sig << std::endl;
	// cause a core dump
	// core.pattern?
	exit (1);
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
	// po
	po::variables_map vm;
	po::options_description opt("Options");
	// adding options
	opt.add_options()
("help,h", "Prints help")
("file,f",  po::value<std::string>(&pfile), "S/N DM Width file")
("rms,--rms",  po::value<float>(&rms)->default_value(4.0f), "RMS value")
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
	hfake.epoch        = 0;
	hfake.nbits        = 8;
	hfake.nchans       = 4096;
	hfake.nifs         = 1;
	hfake.npol         = 1;
	strcpy ("FAKEFAKE", hfake.name)
	strcpy ("/tmp/fakefakefakefake.fil", hfake.sigproc_file)
	strcpy ("1900-01-01-00:00:00", hfake.utc_start_str)
	// 
	FilterbankFake  ffake (hfake.tsamp/1E6, hfake.nchans, hfake.fch1, hfake.foff, rms);
	PsrDADA dada (dadakey, 0,0,0, "/home/vlite-master/surya/faketriggers/dadadada.log");
	dada.WriteLock (true);
	dada.SetHeader (hfake);
	dada.WriteHeader ();
	// Create Write loop
	float sn, dm, wd;
	std::ifstream pf (pfile);
	while (pf >> sn >> dm >> wd) {
		// only 0.1s guards
		auto fb = ffake.Signal (sn, dm, wd, 0.1, 0.1);
		dada.WriteData (fb.data(), fb.size());
	}
	//
	dada.WriteLock (false);
	return 0;
}
