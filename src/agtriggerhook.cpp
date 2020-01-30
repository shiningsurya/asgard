#include "asgard.hpp"
#include "TriggerHook.hpp"

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
		key_t dadakey;
		std::string s_dadakey, d_dadakey("60"), odir, d_odir("/mnt/ssd/dumps/");
		unsigned int nbits, nbufs;
		uint64_t bufsz, nchans, nsamps;
		// po
		po::variables_map vm;
		po::options_description opt("Options");
		// adding options
		opt.add_options()
("help,h", "Prints help")
("key,k",  po::value<std::string>(&s_dadakey)->default_value(d_dadakey), "Input DADA key[def=0x60]")
("nbufs,N",   po::value<unsigned int>(&nbufs)->default_value(12), "nbufs[def=12]")
("nbits,b",   po::value<unsigned int>(&nbits)->default_value(2), "nbits[def=2]")
("nchans,c",  po::value<uint64_t>(&nchans)->default_value(4096), "nchans[def=4096]")
("nsamps,n",  po::value<uint64_t>(&nsamps)->default_value(10240), "nsamps[def=10240]")
("bufsz,s",  po::value<uint64_t>(&bufsz)->default_value(10485760), "buffer size[def=10485760]")
("odir,o", po::value<std::string>(&odir)->default_value(d_odir), "dump directory[def=/mnt/ssd/dumps]")
("trigcoadd,t", po::bool_switch()->default_value(false), "True if should be listening to coadd trigger channel. Otherwise listen to single_trigger channel.");
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
		// big three logic
		// XXX For input bufsz not agreeing to computed bufsz
		// We ALWAYS ONLY adjust nsamps.
		// NCHANS is held sacred at 4k.
		// INPUT NBITS is held sacred. 
		uint64_t rbufsz = nsamps * nchans * nbits / 8;
		if(rbufsz != bufsz) {
				nsamps = 8 * bufsz / nchans / nbits;
		}
		// key 0d to 0x
		dadakey = dx(s_dadakey);
		// trigger channel
		bool ctrig = vm.count("trigcoadd");
		TriggerHook th(
		  dadakey, nbufs,
		  nsamps, nchans, nbits,
		  ctrig,
	   odir	
		);
  th.FollowDADA();
  return 0;
}
