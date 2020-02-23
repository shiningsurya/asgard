#include "asgard.hpp"
#include "TriggerJSON.hpp"
#include "TriggerPlot.hpp"

int main (int ac, char * av[]) {
	if (ac != 2) {
		std::cout << "usage: " << av[0] << " <path-to-dbson>" << std::endl;
	}
	//std::string tpath("/home/shining/work/vlite/searches/soi/20200209_223805_muos_ea99_dm26.88_sn25.58_wd9.38.dbson");
	std::string tpath (av[1]);
	DBSON x (tpath);

	///////
	TriggerPlot tp ("/tmp/asgard", "ps/vcps");
	auto th = x.GetTrigHead ();
	tp.Plot (th, x.bt.data(), x.dd.data());
	return 0;
}
