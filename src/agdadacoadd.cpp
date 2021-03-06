#include "asgard.hpp"
#include <sstream>
#include "DADACoadd.hpp"
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
		key_t key_in, key_out;
		std::string _key_in, _key_out;
		std::string _dkin("56"), _dkout("60");
		bool filout, heimdall_op;
		unsigned int nbits;
		uint64_t bufsz, nchans, nsamps;
  // excision
  float tfac, ffac;
  excision::Method method;
  excision::Filter filter;
  struct excision::excisionParams xp;
		// po
		po::variables_map vm;
		po::options_description opt("Options");
		// adding options
		opt.add_options()
("help,h", "Prints help")
("key-in,k",  po::value<std::string>(&_key_in)->default_value(_dkin), "Input DADA key[def=0x56]")
("key-out,K", po::value<std::string>(&_key_out)->default_value(_dkout), "Output DADA key[def=0x60]")
("filout,f",  po::value<bool>(&filout)->default_value(true), "Write filterbank in root")
("nbits,b",   po::value<unsigned int>(&nbits)->default_value(2), "nbits[def=2]")
("nchans,c",  po::value<uint64_t>(&nchans)->default_value(4096), "nchans[def=4096]")
("nsamps,n",  po::value<uint64_t>(&nsamps)->default_value(10240), "nsamps[def=10240]")
("bufsz,s",  po::value<uint64_t>(&bufsz)->default_value(10485760), "buffer size[def=10485760]")
("excision-timef,t", po::value<float>(&tfac)->default_value(5.0f),  "Time factor")
("excision-freqf,l", po::value<float>(&ffac)->default_value(3.0f),  "Frequency factor")
("excision-method,m", po::value<excision::Method>(&method)->default_value(excision::Method::MAD), "Excision method. def=MAD")
("excision-filter,i", po::value<excision::Filter>(&filter)->default_value(excision::Filter::Noise), "Excision filtering. def=WhiteNoise");
		// parsing
		try {
				po::store(po::command_line_parser(ac,av).options(opt).run(), vm);
				po::notify(vm);
				if(ac == 1 || vm.count("help")) {
      std::cout << "Asgard::agdadacoadd MPI powered realtime coaddition" << std::endl;
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
		// big three logic
		// XXX For input bufsz not agreeing to computed bufsz
		// We ALWAYS ONLY adjust nsamps.
		// NCHANS is held sacred at 4k.
		// INPUT NBITS is held sacred. 
		uint64_t rbufsz = nsamps * nchans * nbits / 8;
		if(rbufsz != bufsz) {
				nsamps = 8 * bufsz / nchans / nbits;
		}
		// excision logic
		xp = {method, filter, tfac, ffac};
		// key 0d to 0x
		key_in  = dx(_key_in);
		key_out = dx(_key_out);
		DADACoadd  dc(
				key_in, key_out, filout, // keys and filout
				nsamps, nchans, nbits,   // big three
				xp,                      // excisionParams
				0);                      // root
		dc.Work();
		return 0;
}
