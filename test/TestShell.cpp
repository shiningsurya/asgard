#include <asgard.hpp>
#include <Shell.hpp>

int main() {
 Shell c("/tmp/hahaha");
 //
 c.AsyncRun();
 c.Wait();
 return 0;
}
