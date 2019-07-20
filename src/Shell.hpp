#pragma once
#include <asgard.hpp>
#include <cstdarg>

constexpr size_t MAX_CMD = 256;
class Shell {
 private:
 	bool AsyncRunning;
 	unsigned int TimesCalled;
 	char  command[MAX_CMD], command_format[MAX_CMD];
 	FILE * command_fp;
 public:
		Shell() {
		TimesCalled = 0;
	 AsyncRunning = false;
	 command_fp = nullptr;
		}
		void SetFmt(const char * cmd) {
				strcpy(command_format, cmd);
		}
	Shell(const char * cmd) {
	 // initialize counter
	 TimesCalled = 0;
	 AsyncRunning = false;
	 strcpy(command_format, cmd);
	 command_fp = nullptr;
	 std::cerr << "Shell::ctor cmdfmt=" << command_format << std::endl;
	}
	bool ReadRun(bool _async, ...) {
		va_list args; va_start(args, _async);
		std::vsnprintf(command, MAX_CMD, command_format, args);
		va_end(args);
		if(_async) 
				std::cerr << "Shell::AsyncRun cmd=" << command << " run=" << ++TimesCalled << std::endl;
		else
				std::cerr << "Shell::SyncRun cmd=" << command << " run=" << ++TimesCalled << std::endl;
	 // This is run
	 command_fp = popen(command, "r");
	 if(command_fp == nullptr) return false;
	 AsyncRunning = _async;
	 if(_async)
	 		return true;
		else
				return pclose(command_fp);
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
