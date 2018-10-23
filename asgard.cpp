#include "asgard.hpp"
#include<boost/program_options/cmdline.hpp>
#include<boost/program_options/config.hpp>
#include<boost/program_options/environment_iterator.hpp>
#include<boost/program_options/option.hpp>
#include<boost/program_options/options_description.hpp>
#include<boost/program_options/parsers.hpp>
#include<boost/program_options/positional_options.hpp>
#include<boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

int main(int argc, char * argv[]){
		std::string wdir, group, 
		po::variables_map vm;
		po::options_description desc("Options");
		desc.add_options()
		("help,h","Prints help")
		("bug-in-code", "Prints contact info")
		("working-directory,w","Sets the working directory")
		("group,g","Sets the group to use")



		return 0;
}
