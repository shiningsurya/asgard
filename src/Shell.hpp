#pragma once
#include <asgard.hpp>
#include <sstream>

class Shell {
 private:
 	bool AsyncRunning;
 	unsigned int TimesCalled;
 	char  command[256];
 	FILE * command_fp;
 public:
	Shell(const std::stringstream& ss) {
	 // initialize counter
	 TimesCalled = 0;
	 AsyncRunning = false;
	 strcpy(command, ss.str().c_str());
	 command_fp = nullptr;
	 std::cerr << "Shell::ctor cmd=" << command << std::endl;
	}
	Shell(const char * cmd) {
	 // initialize counter
	 TimesCalled = 0;
	 AsyncRunning = false;
	 strcpy(command, cmd);
	 command_fp = nullptr;
	 std::cerr << "Shell::ctor cmd=" << command << std::endl;
	}
	bool SyncRun() {
	 // This is blocking run
	 command_fp = popen(command, "r");
	 std::cerr << "Shell::SyncRun run=" << TimesCalled++ << std::endl;
	 return pclose(command_fp);
	}
	bool AsyncRun() {
	 // This is non blocking run
	 command_fp = popen(command, "r");
	 if(command_fp == NULL) return false;
	 std::cerr << "Shell::AsyncRun run=" << TimesCalled++ << std::endl;
	 AsyncRunning = true;
	 return true;
	}
	bool Wait() {
	 bool ret=true;
	 // This waits for non blocking run to finish
	 if(AsyncRunning) {
	 	ret = pclose(command_fp);
	 	command_fp = nullptr;
	 }
	 return ret;
	}
};
