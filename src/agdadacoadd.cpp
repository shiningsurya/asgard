#include "asgard.hpp"
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

int main(int ac, char * av[]) {
		key_t key_in, key_out;
		bool filout;
		uint8_t nbits;
		uint64_t bufsz, nchans, nsamps;
		// po
		po::variables_map vm;
		po::options_description opt("Options");
		// adding options
		opt.add_options()
("help,h", "Prints help")
("key-in,k",  po::value<key_t>(&key_in)->default_value(0x56), "Input DADA key[def=0x56]")
("key-out,K", po::value<key_t>(&key_out)->default_value(0x60), "Output DADA key[def=0x60]")
("filout,f",  po::value<bool>(&filout)->default_value(true), "Write filterbank in root")
("nbits,b",   po::value<uint8_t>(&nbits)->default_value(2), "nbits[def=2]")
("nchans,c",  po::value<uint64_t>(&nchans)->default_value(4096), "nchans[def=4096]")
("nsamps,n",  po::value<uint64_t>(&nsamps)->default_value(10240), "nsamps[def=10240]")
("bufsz,s",  po::value<uint64_t>(&bufsz)->default_value(10485760), "buffer size[def=10485760]");
		// parsing
		try {
				po::store(po::command_line_parser(ac,av).options(opt).run(), vm);
				po::notify(vm);
				if(ac == 1 || vm.count("help")) {
						std::cout << opt << std::endl;
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
				nsamps = 8 * rbufsz / nchans / nbits;
		}
		DADACoadd  dc(
				key_in, key_out, filout, // keys and filout
				nsamps, nchans, nbits,          // big three
				0);                      // root
		dc.Work();
		return 0;
}
