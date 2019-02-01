#include "iostream"
#include "Multicast.hpp"

int main(int ac, char * av[]) { 
		try {
				if(ac != 5) {
						std::cerr << "Usage: testmc <listen-ip> <listen-port> <mc-ip> <mc-port>\n";
						return 1;
				}
				Multicast mc(std::string(av[1]), std::atoi(av[2]), std::string(av[3]), std::atoi(av[4]));
				mc.Receive();
		}
		catch(std::exception& e) {
				std::cerr << e.what() << std::endl;
		}
		return 0;
}
