#include<iostream>
#include<boost/mpi/communicator.hpp>
#include<boost/mpi/environment.hpp>
#include<boost/mpi/collectives.hpp>

namespace mpi = boost::mpi;
using mpi::communicator;
using mpi::environment;
using mpi::reduce;
using std::cout;
using std::endl;

/************************
 *
 * Test1:
 * Have four processes. 
 *
 *
 *
 *
 * *********************/
constexpr int TROOT = 3;
constexpr int TROOT2 = 2;
int main() {
		bool amroot = false;
		unsigned int data = 0, rdata = 0;
		unsigned int comm_root = 0, tcomm_root = 0;
		uint64_t obs = 2, olen = 4;
		communicator comm, tcomm;
		environment env;
		bool vote = false;
		// launching
		if(comm.rank() == comm_root)
		{
				cout << "--------------------------------------------" << endl;
				cout << "Launched with numproc=" << comm.size() << endl;
				cout << "Faulty rank=" << TROOT << endl;
				cout << "--------------------------------------------" << endl;
				amroot = true;
		}
		//
		while(obs--) { // for every observation
				cout << "At barrier : " << comm.rank() << endl;
				if(comm.rank() == comm_root)
						cout << "New OBS="<< obs << endl;
				olen = 4;
				tcomm = std::move(comm.split(true));
				if(comm.rank() == comm_root) {
						tcomm_root = tcomm.rank();
				}
				while(olen--) {// for length of obs
						if(tcomm.rank() == tcomm_root)
								cout << "obslen="<< olen<< endl;
						// DADA READ
						data = tcomm.rank();
						vote = true;
#if 0
						if(obs == 0 && tcomm.rank() == tcomm.size()-1  && olen == 2) {
								vote = false;
						}
						if(obs == 0 && tcomm.rank() == tcomm.size()-1 && olen == 3) {
								vote = false;
						}
#endif
						if(obs == 0 && tcomm.rank() == tcomm_root && olen == 2) {
								vote = false;
								cout << "Dropping root" << endl;
						}
						reduce(tcomm, data, rdata, std::plus<unsigned int>(), tcomm_root);
						if(tcomm.rank() == tcomm_root) {
								cout << "Obs#=" << obs << " Obslen=" << olen << " reduce(6)=" << rdata << endl;
						}
						tcomm = std::move(tcomm.split(vote));
						if(tcomm.rank() == tcomm_root) {
								if(amroot)
										cout << tcomm_root << " --> "<< tcomm.rank() << endl;
								tcomm_root = tcomm.rank();
						}
						if(!vote)  break;
				} // for length of obs
		} // for every observation
		comm.barrier();
		return 0;
}
