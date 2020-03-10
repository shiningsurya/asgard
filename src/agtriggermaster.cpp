#include "asgard.hpp"
#include "TriggerMaster.hpp"

// signal handling
#include <csignal>
void d_signal_handle (int sig) {
  std::cout << "received signal=" << sig << std::endl;
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
  key_t hkey, dkey;
  std::string s_hkey,s_dkey;
  std::string d_hkey("70"), d_dkey("72");
  std::string odir, pdir, d_odir("/mnt/ssd/triggers/");
  uint64_t bufsz;
  // po
  po::variables_map vm;
  po::options_description opt("Options");
  // adding options
  opt.add_options()
    ("help,h", "Prints help")
    ("hkey,k",  po::value<std::string>(&s_hkey)->default_value(d_hkey), "Header DADA key[def=0x72]")
    ("dkey,l",  po::value<std::string>(&s_dkey)->default_value(d_dkey), "Data DADA key[def=0x72]")
    ("plotdir,p", po::value<std::string>(&pdir)->default_value(d_odir), "Plot directory[def=/mnt/ssd/triggers]")
    ("dumpdir,o", po::value<std::string>(&odir)->default_value(d_odir), "Dump directory[def=/mnt/ssd/triggers]")
    ("maxbufsz,b",  po::value<uint64_t>(&bufsz)->default_value(10485760*4), "buffer size[def=10485760*4]");
  // parsing
  try {
    po::store(po::command_line_parser(ac,av).options(opt).run(), vm);
    po::notify(vm);
    if(vm.count("help")) {
      std::cout << "Asgard::agtriggermaster Realtime processing of triggers." << std::endl;
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
  ////////////////////////////////////
  hkey = dx(s_hkey);
  dkey = dx(s_dkey);
  TriggerMaster tm (
    hkey, dkey,
    odir, pdir,
    bufsz
  );
  tm.FollowDADA ();
  return 0;
}
