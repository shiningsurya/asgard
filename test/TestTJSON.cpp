#include "asgard.hpp"
#include "TriggerJSON.hpp"
#include "TriggerPlot.hpp"

int main () {
	std::string tpath("/home/shining/work/vlite/searches/soi/20200209_223805_muos_ea99_dm26.88_sn25.58_wd9.38.dbson");
	DBSON x (tpath);
	std::cout << std::endl;
	std::cout << x << std::endl;
	///////
	TriggerPlot tp ("/tmp");
	auto th = x.GetTrigHead ();
	tp.Plot (th, x.bt.data(), x.dd.data());
	return 0;
}
